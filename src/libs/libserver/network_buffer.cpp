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
    // 判断有效数据是否大于一个总长度值
    if (_dataSize < sizeof(TotalSizeType))
    {
        return nullptr;
    }

    // 读取总长度值
    unsigned short totalSize;
    MemcpyFromBuffer(reinterpret_cast<char*>(&totalSize), sizeof(TotalSizeType));

    // 有效数据不足一条协议长度
    if (_dataSize < totalSize)
    {
        return nullptr;
    }

    RemoveDate(sizeof(TotalSizeType));

    // 读取 PacketHead
    PacketHead head;
    MemcpyFromBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
    RemoveDate(sizeof(PacketHead));

    const google::protobuf::EnumDescriptor* descriptor = Proto::MsgId_descriptor();
    if (descriptor->FindValueByNumber(head.MsgId) == nullptr)
    {
        _pConnectObj->Close();
        std::cout << "recv invalid msg." << std::endl;
        return nullptr;
    }

    // 创建packet
    Packet* pPacket = MessageSystemHelp::CreatePacket((Proto::MsgId)head.MsgId, _pConnectObj);
    const unsigned int dataLength = totalSize - sizeof(PacketHead) - sizeof(TotalSizeType);
    while (pPacket->GetTotalSize() < dataLength)
    {
        pPacket->ReAllocBuffer();
    }

    MemcpyFromBuffer(pPacket->GetBuffer(), dataLength);
    pPacket->FillData(dataLength);
    RemoveDate(dataLength);

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
    TotalSizeType totalSize = dataLength + sizeof(PacketHead) + sizeof(TotalSizeType);

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

        PacketHead head;
        head.MsgId = pPacket->GetMsgId();
        MemcpyToBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
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