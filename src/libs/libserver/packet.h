#pragma once

#include "base_buffer.h"
#include "common.h"
#include "entity.h"
#include "system.h"
#include "socket_object.h"

// 通信方式：TCP + 自定义4字节帧头 + Protobuf包体
// 通信协议：TotalSize（2字节，表示整帧长度，也包含自身两个字节）+ PacketHeadSize（Head长度，根据PacketHead类型而定）+ PacketHead（PacketHead / PacketHeadS2S）+ Protobuf序列化数据

#pragma pack(push)
#pragma pack(4)

struct PacketHead
{
    unsigned short MsgId; // 协议类型
};

struct PacketHeadS2S : public PacketHead
{
    uint64 EntitySn;
    uint64 PlayerSn;
};

#pragma pack(pop)

// 默认大小 10KB
#define DEFAULT_PACKET_BUFFER_SIZE	1024 * 10

class Packet : public Entity<Packet>, public Buffer, public NetIdentify, public IAwakeFromPoolSystem<Proto::MsgId, NetIdentify*>
{
public:
    Packet();
    ~Packet();
    // 初始化对象
    void Awake(Proto::MsgId msgId, NetIdentify* pIdentify) override;

    // 反序列化
    template<class ProtoClass>
    ProtoClass ParseToProto()
    {
        ProtoClass proto;
        proto.ParsePartialFromArray(GetBuffer(), GetDataLength());
        return proto;
    }

    // protobuf 序列化
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

    // HTTP请求/响应 序列化
    void SerializeToBuffer(const char* s, unsigned int len)
    {
        while (GetEmptySize() < len)
        {
            ReAllocBuffer();
        }

        ::memcpy(_buffer + _endIndex, s, len);
        FillData(len);
    }
    
    // 归还对象池
    void BackToPool() override;
    // 拷贝传入packet的数据
    void CopyFrom(Packet* pPacket);
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

    // packet跨线程引用计数
    // +1
    void AddRef();
    // -1
    void RemoveRef();
    // 如果没有启动，即使引用计数为0也不会回收，只有启动了且引用计数为0才会进行回收
    void OpenRef();
    // 该packet可以回收
    bool CanBack2Pool();

private:
    Proto::MsgId _msgId; // 这个包的消息类型

private:
    std::atomic<int> _ref = 0; // 由于同一个packet包可能存在多个线程中处理，所以packet采用引用计数
    bool _isRefOpen = false;   
};