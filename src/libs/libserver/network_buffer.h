#pragma once

#include "base_buffer.h"

#define DEFAULT_SEND_BUFFER_SIZE	1024 * 128
#define DEFAULT_RECV_BUFFER_SIZE	1024 * 128

class Packet;
class ConnectObj;

using TotalSizeType = unsigned short;

class NetworkBuffer : public Buffer
{
public:
    // 缓冲区初始化大小为size
    NetworkBuffer(const unsigned int size, ConnectObj* pConnectObj);
	virtual ~NetworkBuffer();
    void BackToPool();
    // 判断缓冲区是否有数据
	bool HasData() const;
    // 缓冲区剩余可写空间（空间不一定连续）
	unsigned int GetEmptySize() override;
    // 一次可以连续写入的空间（空间一定连续）
	unsigned int GetWriteSize() const;
    // 一次可以连续读取的空间（空间一定连续）
	unsigned int GetReadSize() const;
    // 确认写入
	void FillDate(unsigned int size);
    // 确认读取
	void RemoveDate(unsigned int size);
    // 缓冲区扩容
	void ReAllocBuffer();

protected:
    unsigned int _dataSize; // 有效数据大小，用于判断当_beginIndex == _endIndex时，为空还是为满
    ConnectObj* _pConnectObj{nullptr};
};

class RecvNetworkBuffer : public NetworkBuffer 
{
public:
    // 接收缓冲区初始化大小为size
    RecvNetworkBuffer(unsigned int _size, ConnectObj* pConnectObj);
    // 获取接收缓冲区地址，返回值为一次可以连续写入的空间大小（用于connectObj::Recv）
	int GetBuffer(char*& pBuffer) const;
    // 从接收缓冲区中获取一个packet
	Packet* GetPacket();

private:
    // 从读取缓冲区中拷贝指定size大小到pVoid
	void MemcpyFromBuffer(char* pVoid, unsigned int size);
};

class SendNetworkBuffer : public NetworkBuffer
{
public:
    // 发送缓冲区初始化大小为size
    SendNetworkBuffer(unsigned int _size, ConnectObj* pConnectObj);
    // 获取发送缓冲区地址，返回值为一次可以连续读取的空间大小（用于connectObj::Send）
	int GetBuffer(char*& pBuffer) const;
    // 向发送缓冲区中添加一个packet
	void AddPacket(Packet* pPacket);

private:
    // 向发送缓冲区中拷贝指定size的pVoid
	void MemcpyToBuffer(char* pVoid, unsigned int size);
};
