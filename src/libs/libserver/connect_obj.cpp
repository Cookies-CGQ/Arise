#include <iostream>
#include "connect_obj.h"
#include "network.h"
#include "network_buffer.h"
#include "packet.h"
#include "thread_mgr.h"
#include "object_pool_interface.h"

ConnectObj::ConnectObj(IDynamicObjectPool* pPool) :ObjectBlock(pPool)
{
    _pNetWork = nullptr;
    _socket = INVALID_SOCKET;
    _recvBuffer = new RecvNetworkBuffer(DEFAULT_RECV_BUFFER_SIZE, this);
    _sendBuffer = new SendNetworkBuffer(DEFAULT_SEND_BUFFER_SIZE, this);
}

ConnectObj::~ConnectObj()
{
    if (_recvBuffer != nullptr)
        delete _recvBuffer;

    if (_sendBuffer != nullptr)
        delete _sendBuffer;
}

void ConnectObj::TakeoutFromPool(Network* pNetWork, SOCKET socket)
{
    _pNetWork = pNetWork;
    _socket = socket;
}

void ConnectObj::BackToPool()
{
    if (!Global::GetInstance()->IsStop)
    {
        // 通知其他对象，有Socket中断了
        Packet* pResultPacket = new Packet(Proto::MsgId::MI_NetworkDisconnect, _socket);
        MessageList::DispatchPacket(pResultPacket);
    }

    _pNetWork = nullptr;
    _socket = INVALID_SOCKET;
    _recvBuffer->BackToPool();
    _sendBuffer->BackToPool();

    _pPool->FreeObject(this);
}

// 接收缓冲区是否有数据
bool ConnectObj::HasRecvData() const
{
    return _recvBuffer->HasData();
}

// 获取packet
Packet* ConnectObj::GetRecvPacket() const
{
    return _recvBuffer->GetPacket();
}

// 接收数据到接收缓冲区
bool ConnectObj::Recv() const
{
    bool isRs = false;
    char* pBuffer = nullptr;
    // 非阻塞fd，使用while一次读完系统接收缓冲区
    while(true)
    {
        // 总空间数据不足一个协议头的大小，需要扩容
		if (_recvBuffer->GetEmptySize() < (sizeof(PacketHead) + sizeof(TotalSizeType)))
		{
			_recvBuffer->ReAllocBuffer();
		}

		const int emptySize = _recvBuffer->GetBuffer(pBuffer);
		const int dataSize = ::recv(_socket, pBuffer, emptySize, 0);
        if(dataSize > 0)
        {
            _recvBuffer->FillDate(dataSize);
        }
        else if(dataSize == 0)
        {
            // 对端关闭，由父级Network关闭连接
            break;
        }
        else
        {
            const auto socketError = _sock_err();
#ifndef WIN32
            if (socketError == EINTR || socketError == EWOULDBLOCK || socketError == EAGAIN)
            {
                isRs = true;
            }
#else
            if (socketError == WSAEINTR || socketError == WSAEWOULDBLOCK)
            {
                isRs = true;
            }
#endif
            break;
        }
    }

    // 将收到的信息打包成packet分发给所有Actor
    if (isRs)
    {
        while (true) 
        {
            const auto pPacket = _recvBuffer->GetPacket();
            if (pPacket == nullptr)
                break;

            if(pPacket->GetMsgId() == Proto::MsgId::MI_Ping)
            {
                // RecvPing();
            }
            else
            {
                if (_pNetWork->IsBroadcast() && _pNetWork->GetThread() != nullptr)
                {
                    ThreadMgr::GetInstance()->DispatchPacket(pPacket);
                }
                else
                {
                    _pNetWork->GetThread()->AddPacketToList(pPacket);
                }
            }
        }
    }

    return isRs;
}

// 发送缓冲区是否有数据
bool ConnectObj::HasSendData() const
{
    return _sendBuffer->HasData();
}

// 发送packet到发送缓冲区
void ConnectObj::SendPacket(Packet *pPacket) const
{
	_sendBuffer->AddPacket(pPacket);
}

// 从发送缓存区读取发送数据
bool ConnectObj::Send() const
{
    while(true)
    {
        char* pBuffer = nullptr;
        const int needSendSize = _sendBuffer->GetBuffer(pBuffer);
        if(needSendSize <= 0)
            return true;

        const int size = ::send(_socket, pBuffer, needSendSize, 0);
        // 发送成功
        if(size > 0)
        {
            _sendBuffer->RemoveDate(size);
            // 发送缓冲区数据还没发完，等下次再发
            if(size < needSendSize)
                return true;
        }
        // 发送失败
        else if(size <= 0)
        {
            const auto socketError = _sock_err();
			std::cout << "needSendSize:" << needSendSize << " error:" << socketError << std::endl;
            return false;
        }
    }
}

void ConnectObj::Close()
{
    const auto pPacketDis = new Packet(Proto::MsgId::MI_NetworkDisconnectToNet, GetSocket());
    // 发送packet到NetWork所在线程并进行断开连接处理（使用发送packet的方式是因为Close可能会在其他线程调用）
    _pNetWork->GetThread()->AddPacketToList(pPacketDis);
}