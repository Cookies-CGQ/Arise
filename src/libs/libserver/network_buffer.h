#pragma once

#include "base_buffer.h"

// 默认大小 128KB
#define DEFAULT_SEND_BUFFER_SIZE	1024 * 128
#define DEFAULT_RECV_BUFFER_SIZE	1024 * 128

class Packet;
class ConnectObj;

// 存储协议总长度的类型
using TotalSizeType = unsigned short;

class NetworkBuffer: public Buffer
{
public:
    NetworkBuffer(const unsigned int size, ConnectObj* pConnectObj);
    virtual ~NetworkBuffer();

    // 清空缓冲区
    void BackToPool();

    // 是否有数据
    bool HasData() const;
    
    // 实际空字节数（包括环的头与环的尾）
    unsigned int GetEmptySize() override;

    // 当前可一次性连续写入的空间长度(就是连续写入不考虑循环写入)
    unsigned int GetWriteSize() const;

    // 当前可一次性连续读取的长度(就是连续读取不考虑循环读取)
    unsigned int GetReadSize() const;

    // 确定写入size字节
    void FillDate(unsigned int size);
    // 确定已经读取size字节
    void RemoveDate(unsigned int size);
    //申请一次扩容
    void ReAllocBuffer();

protected:
    // 在环形中，极端情况下 _endIndex 可能与 _beginIndex 重合
    // 重合时有两种可能，一种是没有数据，另一种是满数据
    // 所以判断两种可能时，需要用有效数据来区分
    unsigned int _dataSize;                 // 有效数据
    ConnectObj* _pConnectObj = nullptr;     // 连接对象 
};

// 系统缓存区有：系统接收缓冲区和系统发送缓冲区
// RecvNetworkBuffer和SendNetworkBuffer：是应用层面的缓冲区
// RecvNetworkBuffer介于上层和系统接收缓冲区之间，被上层读取时采用Packet读取，被系统接收缓冲区写入时还是按字节写入
// SendNetworkBuffer介于上层和系统发送缓冲区之间，被上层写入时采用Packet写入，被系统发送缓冲区读取时还是按字节读取

// 接收缓冲区
class RecvNetworkBuffer : public NetworkBuffer
{
public:
    RecvNetworkBuffer(unsigned int _size, ConnectObj* pConnectObj);
    // 获取接收缓冲区的起始可写区域并返回可以连续写入的可写区域大小
    int GetBuffer(char*& pBuffer) const;
    // 从接收缓冲区中获取返回一个packet
    Packet* GetPacket();

protected:
    // 获取TCP packet
    Packet* GetTcpPacket();
    // 获取HTTP packet
    Packet* GetHttpPacket();

private:
    // 从接收缓冲区拷贝size字节到pVoid(只是拷贝，未真正读取缓冲区)
    void MemcpyFromBuffer(char* pVoid, unsigned int size);
};

// 发送缓冲区
class SendNetworkBuffer : public NetworkBuffer
{
public:
    SendNetworkBuffer(unsigned int _size, ConnectObj* pConnectObj);
    // 获取发送缓冲区的起始可读区域并返回可以连续读取的可读区域大小
    int GetBuffer(char*& pBuffer) const;
    // 向发送缓冲区写入一个packet
    void AddPacket(Packet* pPacket);

private:
    // 从pVoid拷贝size到发送缓冲区（真正写入到发送缓冲区）
    void MemcpyToBuffer(char* pVoid, unsigned int size);
};