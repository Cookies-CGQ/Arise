#include <cstdlib>
#include <cstring>
#include "packet.h"
#include "network_buffer.h"
#include "connect_obj.h"
#include "message_system_help.h"
#include "network.h"
#include "mongoose/mongoose.h"

NetworkBuffer::NetworkBuffer(const unsigned int size, ConnectObj* pConnectObj)
{
    _pConnectObj = pConnectObj;
    _bufferSize = size;
    _beginIndex = _endIndex = 0;
    _dataSize = 0;
    _buffer = new char[_bufferSize];
}

NetworkBuffer::~NetworkBuffer()
{
    if(_buffer != nullptr)
    {
        delete[] _buffer;
        _buffer = nullptr;
    }
}

void NetworkBuffer::BackToPool()
{
    _beginIndex = _endIndex = 0;
    _dataSize = 0;
}

bool NetworkBuffer::HasData() const
{
    if(_dataSize <= 0)
        return false;
    // 如果缓冲区连最起码的长度字段的大小都没有那视为没有数据
    if(_dataSize < sizeof(TotalSizeType))
        return false;

    return true;
}

unsigned int NetworkBuffer::GetEmptySize()
{
    return _bufferSize - _dataSize;
}

unsigned int NetworkBuffer::GetWriteSize() const
{
    if(_beginIndex <= _endIndex)
        return _bufferSize - _endIndex;
    else
        return _beginIndex - _endIndex;
}

unsigned int NetworkBuffer::GetReadSize() const
{
    if(_dataSize <= 0)
        return 0;

    if(_beginIndex < _endIndex)
        return _endIndex - _beginIndex;
    else
        return _bufferSize - _beginIndex;
}

void NetworkBuffer::FillDate(unsigned int size)
{
    _dataSize += size;
    if((_bufferSize - _endIndex) <= size)
    {
        size -= (_bufferSize - _endIndex);
        _endIndex = 0;
    }
    // 只负责写入，改变index，至于空间够不够是外部的事
    _endIndex += size;
}

void NetworkBuffer::RemoveDate(unsigned int size)
{
    _dataSize -= size;
    if((_beginIndex + size) >= _bufferSize)
    {
        size -= (_bufferSize - _beginIndex);
        _beginIndex = 0;
    }
    _beginIndex += size;
}

void NetworkBuffer::ReAllocBuffer()
{
    Buffer::ReAllocBuffer(_dataSize);
}

RecvNetworkBuffer::RecvNetworkBuffer(const unsigned int size, ConnectObj* pConnectObj)
    :NetworkBuffer(size, pConnectObj)
{

}

int RecvNetworkBuffer::GetBuffer(char*& pBuffer) const
{
    pBuffer = _buffer + _endIndex;
    return GetWriteSize();
}

Packet* RecvNetworkBuffer::GetPacket()
{
    auto pNetwork = _pConnectObj->GetParent<Network>();
    if (!NetworkHelp::IsTcp(pNetwork->GetNetworkType()))
        return GetHttpPacket();

    return GetTcpPacket();
}

Packet* RecvNetworkBuffer::GetTcpPacket()
{
    // 有效数据长度小于总长度字段大小
    if (_dataSize < sizeof(TotalSizeType))
    {
        return nullptr;
    }

    // 读取总长度
    unsigned short totalSize;
    MemcpyFromBuffer(reinterpret_cast<char*>(&totalSize), sizeof(TotalSizeType));

    // 有效数据长度小于总长度
    if (_dataSize < totalSize)
    {
        return nullptr;
    }

    RemoveDate(sizeof(TotalSizeType));

    // 读取头部长度字段
    unsigned short headSize;
    MemcpyFromBuffer(reinterpret_cast<char*>(&headSize), sizeof(TotalSizeType));
    RemoveDate(sizeof(TotalSizeType));

    // 读取头部，PacketHead / PacketHeadS2S
    Proto::MsgId msgId;
    bool isS2S = false;
    uint64 entitySn = 0;
    uint64 playerSn = 0;
    // PacketHead / PacketHeadS2S字段长度不同
    if (headSize == sizeof(PacketHead))
    {
        // 读取头部
        PacketHead head;
        MemcpyFromBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
        RemoveDate(sizeof(PacketHead));
        msgId = static_cast<Proto::MsgId>(head.MsgId);
    }
    else
    {
        // 读取头部
        PacketHeadS2S head;
        MemcpyFromBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHeadS2S));
        RemoveDate(sizeof(PacketHeadS2S));
        msgId = static_cast<Proto::MsgId>(head.MsgId);
        entitySn = head.EntitySn;
        playerSn = head.PlayerSn;
        isS2S = true;
    }

    const google::protobuf::EnumDescriptor* descriptor = Proto::MsgId_descriptor();
    if (descriptor->FindValueByNumber(msgId) == nullptr)
    {
        _pConnectObj->Close();
        std::cout << "recv invalid msg." << std::endl;
        return nullptr;
    }

    // 创建packet
    Packet* pPacket = MessageSystemHelp::CreatePacket(msgId, _pConnectObj);
    unsigned int dataLength = totalSize - sizeof(PacketHead) - sizeof(TotalSizeType) * 2;
    if (isS2S)
        dataLength = totalSize - sizeof(PacketHeadS2S) - sizeof(TotalSizeType) * 2;

    while (pPacket->GetTotalSize() < dataLength)
    {
        pPacket->ReAllocBuffer();
    }

    MemcpyFromBuffer(pPacket->GetBuffer(), dataLength);
    pPacket->FillData(dataLength);
    RemoveDate(dataLength);

    if (isS2S)
    {
        auto pTagKey = pPacket->GetTagKey();
        pTagKey->AddTag(TagType::Entity, entitySn);
        pTagKey->AddTag(TagType::Player, playerSn);
    }

    return pPacket;
}

Packet* RecvNetworkBuffer::GetHttpPacket()
{
    if (_endIndex < _beginIndex)
    {
        _pConnectObj->Close();
        LOG_ERROR("http recv invalid.");
        return nullptr;
    }

    const unsigned int recvBufLength = _endIndex - _beginIndex;
    const auto pNetwork = _pConnectObj->GetParent<Network>();
    const auto iType = pNetwork->GetNetworkType();
    const bool isConnector = iType == NetworkType::HttpConnector;

    http_message hm;
    const unsigned int headerLen = mg_parse_http(_buffer + _beginIndex, _endIndex - _beginIndex, &hm, !isConnector);
    if (headerLen <= 0)
        return nullptr;

    unsigned int bodyLen = 0;
    const auto mgBody = mg_get_http_header(&hm, "Content-Length");
    if (mgBody != nullptr)
    {
        bodyLen = atoi(mgBody->p);

        if (bodyLen > 0 && (recvBufLength < (bodyLen + headerLen)))
            return nullptr;
    }

    bool isChunked = false;
    const auto mgTransferEncoding = mg_get_http_header(&hm, "Transfer-Encoding");
    if (mgTransferEncoding != nullptr && mg_vcasecmp(mgTransferEncoding, "chunked") == 0)
    {
        isChunked = true;

        if (recvBufLength == headerLen)
            return nullptr;

        bodyLen = mg_http_get_request_len(_buffer + _beginIndex + headerLen, recvBufLength - headerLen);
        if (bodyLen <= 0)
            return nullptr;

        bodyLen = _endIndex - _beginIndex - headerLen;
    }
    else if (mgBody == nullptr && isConnector)
    {
        // 响应既没有 Content-Length 也没有 chunked（Connection: close 场景），
        // 缓冲区中 header 之后的所有数据都是 body；body 未到齐则等待下一轮读取
        // 注意：只对响应生效，GET 等无 body 的请求不能走这个分支
        if (recvBufLength <= headerLen)
            return nullptr;

        bodyLen = recvBufLength - headerLen;
    }

#ifdef LOG_HTTP_OPEN
    std::stringstream allBuffer;
    allBuffer.write(_buffer + _beginIndex, (bodyLen + headerLen));
    LOG_HTTP("\r\n" << allBuffer.str().c_str());
#endif

    Packet* pPacket = MessageSystemHelp::ParseHttp(_pConnectObj,_buffer + _beginIndex + headerLen, bodyLen, isChunked, &hm);
    RemoveDate(bodyLen + headerLen);

    return pPacket;
}

void RecvNetworkBuffer::MemcpyFromBuffer(char* pVoid, const unsigned int size)
{
    const auto readSize = GetReadSize();
    // 如果循环分两次拷贝
    if (readSize < size)
    {
        ::memcpy(pVoid, _buffer + _beginIndex, readSize);
        ::memcpy(pVoid + readSize, _buffer, size - readSize);
    }
    else
    {
        ::memcpy(pVoid, _buffer + _beginIndex, size);
    }
}

SendNetworkBuffer::SendNetworkBuffer(const unsigned int size, ConnectObj* pConnectObj)
    : NetworkBuffer(size, pConnectObj)
{

}

int SendNetworkBuffer::GetBuffer(char*& pBuffer) const
{
    if(_dataSize <= 0)
    {
        pBuffer = nullptr;
        return 0;
    }
    
    if (_beginIndex < _endIndex)
    {
        pBuffer = _buffer + _beginIndex;
        return _endIndex - _beginIndex;
    }
    else
    {
        pBuffer = _buffer + _beginIndex;
        return _bufferSize - _beginIndex;
    }
}

void SendNetworkBuffer::AddPacket(Packet* pPacket)
{
    const auto dataLength = pPacket->GetDataLength();
    const auto pTagValue = pPacket->GetTagKey()->GetTagValue(TagType::Entity);

    TotalSizeType totalSize = dataLength + sizeof(PacketHead) + sizeof(TotalSizeType) * 2;
    if (pTagValue != nullptr)
    {
        totalSize = dataLength + sizeof(PacketHeadS2S) + sizeof(TotalSizeType) * 2;
    }

    // 扩容
    while (GetEmptySize() < totalSize) 
    {
        ReAllocBuffer();
    }

    // 判断是http处理还是tcp处理
    const auto msgId = pPacket->GetMsgId();
    if (!NetworkHelp::IsHttpMsg(msgId))
    {
        // 如果是tcp packet，还需要写入 总长度 + PacketHead，最后才是数据内容
        MemcpyToBuffer(reinterpret_cast<char*>(&totalSize), sizeof(TotalSizeType));

        if (pTagValue == nullptr)
        {
            PacketHead head{};
            head.MsgId = pPacket->GetMsgId();
            TotalSizeType headSize = sizeof(PacketHead);
            MemcpyToBuffer(reinterpret_cast<char*>(&headSize), sizeof(TotalSizeType));
            MemcpyToBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
        }
        else
        {
            PacketHeadS2S head{};
            head.MsgId = pPacket->GetMsgId();
            head.EntitySn = pTagValue->KeyInt64;
            const auto pTagPlayer = pPacket->GetTagKey()->GetTagValue(TagType::Player);
            if (pTagPlayer == nullptr)
                head.PlayerSn = 0;
            else
                head.PlayerSn = pTagPlayer->KeyInt64;

            TotalSizeType headSize = sizeof(PacketHeadS2S);
            MemcpyToBuffer(reinterpret_cast<char*>(&headSize), sizeof(TotalSizeType));
            MemcpyToBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHeadS2S));
        }
    }

    // 写入发送缓冲区
    MemcpyToBuffer(pPacket->GetBuffer(), pPacket->GetDataLength());
}

void SendNetworkBuffer::MemcpyToBuffer(char* pVoid, const unsigned int size)
{
    const auto writeSize = GetWriteSize();
    if (writeSize < size)
    {
        ::memcpy(_buffer + _endIndex, pVoid, writeSize);
        ::memcpy(_buffer, pVoid + writeSize, size - writeSize);
    }
    else
    {
        ::memcpy(_buffer + _endIndex, pVoid, size);
    }

    // 确定写入--偏移指针
    FillDate(size);
}