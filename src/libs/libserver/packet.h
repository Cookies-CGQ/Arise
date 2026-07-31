#pragma once

#include "base_buffer.h"
#include "common.h"
#include "entity.h"
#include "system.h"

// 通信方式：TCP + 自定义4字节帧头 + Protobuf包体
// 通信协议：TotalSize(2字节，表示整帧长度，包含长度字段、消息号、包体) + MsgId(2字节，消息类型编号) + Payload(N字节，Protobuf二进制数据)

#pragma pack(push)
#pragma pack(4)

struct PacketHead
{
    unsigned short MsgId; // 协议类型
};

struct PacketInnerHead: public PacketHead
{
    unsigned int ThreadType;    // 线程类型
    unsigned short ChooseType;
};

#pragma pack(pop)

// 默认大小 10KB
#define DEFAULT_PACKET_BUFFER_SIZE	1024 * 10

class Packet: public Entity<Packet>, public Buffer, public IAwakeFromPoolSystem<Proto::MsgId, SOCKET>
{
public:
    Packet();
    ~Packet();
    // 初始化对象
    void Awake(Proto::MsgId, SOCKET socket) override;

    // 反序列化
    template<class ProtoClass>
    ProtoClass ParseToProto()
    {
        ProtoClass proto;
        proto.ParsePartialFromArray(GetBuffer(), GetDataLength());
        return proto;
    }

    // 序列化
    template<class ProtoClass>
    void SerializeToBuffer(ProtoClass& protoClase)
    {
        auto total = (unsigned int)protoClase.ByteSizeLong();
        // 如果空间不够就申请空间
        while(GetEmptySize() < total)
        {
            ReAllocBuffer();
        }
        // 序列化
        protoClase.SerializePartialToArray(GetBuffer(), total);
        FillData(total);
    }
    
    // 归还对象池
    void BackToPool() override;
    // 获取缓存
    char* GetBuffer() const;
    // 有效数据长度
    unsigned short GetDataLength() const;
    // 获取消息类型
    int GetMsgId() const;
    // 写入数据
    void FillData(unsigned int size);
    // 申请空间
    void ReAllocBuffer();
    // 获取socket
    SOCKET GetSocket() const;
    // 设置socket
    void SetSocket(SOCKET socket);

    // 引用计数
    // +1
    void AddRef();
    // -1
    void RemoveRef();
    //
    void OpenRef();
    //
    bool CanBack2Pool();

private:
    Proto::MsgId _msgId; // 这个包的消息类型
    SOCKET _socket;      // 包来自哪个socket

private:
    std::atomic<int> _ref{0}; // 由于同一个packet包可能存在多个线程中处理，所以packet采用引用计数
    bool _isRefOpen = false;   
};