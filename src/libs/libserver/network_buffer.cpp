#include "packet.h"
#include "network_buffer.h"
#include "connect_obj.h"
#include <cstdlib>
#include <iostream>
#include <cstring>

NetworkBuffer::NetworkBuffer(const unsigned size, ConnectObj* pConnectObj)
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
        delete[] _buffer;
}

void NetworkBuffer::BackToPool()
{
    _beginIndex = 0;
    _endIndex = 0;
    _dataSize = 0;
}

bool NetworkBuffer::HasData() const
{
    if(_dataSize <= 0)
        return false;
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
    {
        return _bufferSize - _endIndex;
    }
    else
    {
        return _beginIndex - _endIndex;
    }
}

unsigned int NetworkBuffer::GetReadSize() const
{
    if(_dataSize <= 0)
        return 0;
    if(_beginIndex < _endIndex)
    {
        return _endIndex - _beginIndex;
    }
    else
    {
        return _bufferSize - _beginIndex;
    }
}

void NetworkBuffer::FillDate(unsigned int size)
{
    _dataSize += size;
    if((_bufferSize - _endIndex) <= size)
    {
        size -= _bufferSize - _endIndex;
        _endIndex = 0;
    }
    _endIndex += size;
}

void NetworkBuffer::RemoveDate(unsigned int size)
{
    _dataSize -= size;
    if((_beginIndex + size) >= _bufferSize)
    {
        size -= _bufferSize - _beginIndex;
        _beginIndex = 0;
    }
    _beginIndex += size;
}

void NetworkBuffer::ReAllocBuffer()
{
    Buffer::ReAllocBuffer(_dataSize);
}

RecvNetworkBuffer::RecvNetworkBuffer(const unsigned int size, ConnectObj* pConnectObj) 
    : NetworkBuffer(size, pConnectObj) 
{

}

int RecvNetworkBuffer::GetBuffer(char*& pBuffer) const
{
    pBuffer = _buffer + _endIndex;
    return GetWriteSize();
}

Packet* RecvNetworkBuffer::GetPacket()
{
    // 可读数据不足一个数据包的长度字段的大小
    if(_dataSize < sizeof(TotalSizeType))
    {
        return nullptr;
    }
    
    // 读取长度字段
    unsigned short totalSize;
    MemcpyFromBuffer(reinterpret_cast<char*>(&totalSize), sizeof(TotalSizeType));
    // 可读数据不足一个协议体长度
    if(_dataSize < totalSize)
    {
        return nullptr;
    }
    // 读取缓冲区可以读取一个协议
    // 读取长度字段
    RemoveDate(sizeof(TotalSizeType));
    // 读取协议头
    PacketHead head;
    MemcpyFromBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
    RemoveDate(sizeof(PacketHead));
    // 读出协议，检查id
    const google::protobuf::EnumDescriptor* descriptor = Proto::MsgId_descriptor();
    if(descriptor->FindValueByNumber(head.MsgId) == nullptr)
    {
        // 关闭网络
        _pConnectObj->Close();
        std::cout << "recv invalid msg" << std::endl;
        return nullptr;
    }
    // 读取协议体
    const auto socket = _pConnectObj->GetSocket();
    Packet* pPacket = new Packet((Proto::MsgId)head.MsgId, socket);
    const auto dataLength = totalSize - sizeof(PacketHead) - sizeof(TotalSizeType);
	while (pPacket->GetTotalSize() < dataLength)
	{
        // Packet缓冲区扩容
		pPacket->ReAllocBuffer();
	}
    MemcpyFromBuffer(pPacket->GetBuffer(), dataLength);
    pPacket->FillData(dataLength);
    RemoveDate(dataLength);

    return pPacket;
}

void RecvNetworkBuffer::MemcpyFromBuffer(char* pVoid, const unsigned int size)
{
    const auto readSize = GetReadSize();
    if(readSize < size)
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
    // 没有数据可读
    if(_dataSize <= 0)
    {
        pBuffer = nullptr;
        return 0;
    }
    if(_beginIndex < _endIndex)
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

    // 长度不够，扩容
    while(GetEmptySize() < totalSize)
    {
        ReAllocBuffer();
    }

    // 整体长度
    MemcpyToBuffer(reinterpret_cast<char*>(&totalSize), sizeof(TotalSizeType));
    // 头部
	PacketHead head;
	head.MsgId = pPacket->GetMsgId();
	MemcpyToBuffer(reinterpret_cast<char*>(&head), sizeof(PacketHead));
    // 协议体
    MemcpyToBuffer(pPacket->GetBuffer(), pPacket->GetDataLength());
}

void SendNetworkBuffer::MemcpyToBuffer(char* pVoid, const unsigned int size)
{
    const auto writeSize = GetWriteSize();
    if(writeSize < size)
    {
        ::memcpy(_buffer + _endIndex, pVoid, writeSize);
        ::memcpy(_buffer, pVoid + writeSize, size - writeSize);
    }
    else
    {
        ::memcpy(_buffer + _endIndex, pVoid, size);
    }
    FillDate(size);
}
