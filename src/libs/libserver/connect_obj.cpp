#include "connect_obj.h"
#include "network.h"
#include "network_buffer.h"
#include "thread_mgr.h"
#include "system_manager.h"
#include "message_system.h"
#include "message_system_help.h"
#include "component_help.h"
#include "object_pool_packet.h"
#include "network_locator.h"
#include "socket_locator.h"

ConnectObj::ConnectObj()
{
    _state = ConnectStateType::None;
    _recvBuffer = new RecvNetworkBuffer(DEFAULT_RECV_BUFFER_SIZE, this);
    _sendBuffer = new SendNetworkBuffer(DEFAULT_SEND_BUFFER_SIZE, this);
}

ConnectObj::~ConnectObj()
{
    if (_recvBuffer != nullptr)
    {
        delete _recvBuffer;
        _recvBuffer = nullptr;
    }

    if (_sendBuffer != nullptr)
    {
        delete _sendBuffer;
        _sendBuffer = nullptr;
    }
    _state = ConnectStateType::None;
}

void ConnectObj::Awake(SOCKET socket, NetworkType networkType, TagType tagType, TagValue tagValue, ConnectStateType state)
{
    _socketKey = SocketKey(socket, networkType);

    _tagKey.Clear();
    if (tagType != TagType::None)
        _tagKey.AddTag(tagType, tagValue);

    _state = state;
}

void ConnectObj::BackToPool()
{
#ifdef LOG_TRACE_COMPONENT_OPEN
    const auto traceMsg = std::string("close.  network type:").append(GetNetworkTypeName(_socketKey.NetType));
    ComponentHelp::GetTraceComponent()->Trace(TraceType::Connector, _socketKey.Socket, traceMsg);
#endif

    // 底层网络断开，通知上次逻辑
    MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_NetworkDisconnect, this);    

    if (_socketKey.Socket != INVALID_SOCKET)
        _sock_close(_socketKey.Socket);

    _state = ConnectStateType::None;
    _socketKey.Clear();
    _tagKey.Clear();

    _recvBuffer->BackToPool();
    _sendBuffer->BackToPool();
}

bool ConnectObj::HasRecvData() const
{
    return _recvBuffer->HasData();
}

bool ConnectObj::Recv()
{
    // 对于网络监听，如果可读，表示连接建立成功了，状态转移：正在连接->已连接
    if(_state == ConnectStateType::Connecting)
    {
        ChangeStateToConnected();
        return true;
    }

    bool isRs = false;
    char* pBuffer = nullptr;
    while (true)
    {
        // 总空间数据不足一个头的大小，扩容
        while (_recvBuffer->GetEmptySize() < (sizeof(PacketHeadS2S) + sizeof(TotalSizeType) * 2))
        {
            _recvBuffer->ReAllocBuffer();
        }

        const int emptySize = _recvBuffer->GetBuffer(pBuffer);
        const int dataSize = ::recv(_socketKey.Socket, pBuffer, emptySize, 0);
        // 读取到数据
        if (dataSize > 0)
        {
            _recvBuffer->FillDate(dataSize);
            isRs = true;
        }
        // 对端关闭
        else if (dataSize == 0)
        {
            break;
        }
        else
        {
            const auto socketError = _sock_err();
            isRs = (socketError != 0);
            // 不是真正的错误，只是暂时不可用
            if (!NetworkHelp::IsError(socketError))
            {
                isRs = true;
            }

            if (!isRs)
                LOG_WARN("recv size:" << dataSize << " error:" << socketError);

            break;
        }
    }

    // 数据打包成packet进行分发
    if (isRs)
    {
        const auto pNetwork = this->GetParent<Network>();
        const auto iNetworkType = pNetwork->GetNetworkType();
        while (true)
        {
            const auto pPacket = _recvBuffer->GetPacket();
            if (pPacket == nullptr)
                break;

            const auto pTagAccount = this->_tagKey.GetTagValue(TagType::Account);
            if (pTagAccount != nullptr)
            {
                auto pSocketLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<SocketLocator>();
                if (pSocketLocator != nullptr)
                {
                    const auto targetEntitySn = pSocketLocator->GetTargetEntitySn(pPacket->GetSocketKey()->Socket);
                    if (targetEntitySn > 0)
                    {
                        pPacket->GetTagKey()->AddTag(TagType::Entity, targetEntitySn);
                    }
                }
            }

            const auto msgId = pPacket->GetMsgId();
            const bool isTcp = NetworkHelp::IsTcp(iNetworkType);
            // HTTP
            if (!isTcp)
            {
                if (msgId == Proto::MsgId::MI_HttpRequestBad)
                {
                    // 404
                    MessageSystemHelp::SendHttpResponse404(pPacket);
                    DynamicPacketPool::GetInstance()->FreeObject(pPacket);
                    continue;
                }
            }
            else
            {
                if (msgId == Proto::MsgId::MI_Ping)
                {
                    //RecvPing();
                    continue;
                }
            }

            if (!isTcp)
            {
                if ((msgId <= Proto::MsgId::MI_HttpBegin || msgId >= Proto::MsgId::MI_HttpEnd) && msgId != Proto::MsgId::MI_HttpOuterResponse)
                {
                    LOG_WARN("http connect recv. tcp proto");
                    DynamicPacketPool::GetInstance()->FreeObject(pPacket);
                    continue;
                }
            }
            // 分发
            ThreadMgr::GetInstance()->DispatchPacket(pPacket);
        }
    }

    return isRs;
}

bool ConnectObj::HasSendData() const
{
    if(_state == ConnectStateType::Connecting)
        return true;

    return _sendBuffer->HasData();
}

void ConnectObj::SendPacket(Packet* pPacket) const
{
    _sendBuffer->AddPacket(pPacket);
    DynamicPacketPool::GetInstance()->FreeObject(pPacket);
}

bool ConnectObj::Send() const
{
    // 对于网络连接，如果可写，表示连接建立成功了，状态转移：正在连接->已连接
    if (_state == ConnectStateType::Connecting)
    {
        ChangeStateToConnected();
        return true;
    }

    while (true) 
    {
        char* pBuffer = nullptr;
        const int needSendSize = _sendBuffer->GetBuffer(pBuffer);

        // 没有数据可发送
        if (needSendSize <= 0)
        {
            return true;
        }

        const int size = ::send(_socketKey.Socket, pBuffer, needSendSize, 0);
        if (size > 0)
        {
            _sendBuffer->RemoveDate(size);

            // 下一帧再发送
            if (size < needSendSize)
            {
                return true;
            }
        }
        else if (size <= 0)
        {
            const auto socketError = _sock_err();
            std::cout << "needSendSize:" << needSendSize << " error:" << socketError << std::endl;
            return false;
        }
    }
}

void ConnectObj::Close()
{
    // ConnectoObj 一定和Network在同一个线程中的，这里，只需要本线程发送数据即可
    const auto pPacketDis = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_NetworkRequestDisconnect, this);
    GetSystemManager()->GetMessageSystem()->AddPacketToList(pPacketDis);
    pPacketDis->OpenRef();
}

ConnectStateType ConnectObj::GetState() const
{
    return _state;
}

void ConnectObj::ChangeStateToConnected()
{
    _state = ConnectStateType::Connected;

    // 如果是服务与服务之间的连接，那就添加到网络路由表中
    const auto pTagValue = _tagKey.GetTagValue(TagType::App);
    if (pTagValue != nullptr)
    {
        auto pLocator = ThreadMgr::GetInstance()->GetEntitySystem()->GetComponent<NetworkLocator>();
        pLocator->AddNetworkIdentify(&_socketKey, pTagValue->KeyInt64);
    }
    else
    {
        // 通知逻辑层，有连接连接成功了
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_NetworkConnected, this);
    }
}