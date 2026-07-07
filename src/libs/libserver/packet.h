#pragma once

#include "base_buffer.h"
#include "common.h"
#include "proto.h"

#pragma pack(push)
#pragma pack(4)

// 协议头
struct PacketHead
{
    unsigned short MsgId; // 协议号
};

#pragma pack(pop)

// 默认大小 10KB
#define DEFAULT_PACKET_BUFFER_SIZE	1024 * 10

class Packet : public Buffer
{
public:
	Packet(const Proto::MsgId msgId, SOCKET socket);
	~Packet();

	// 从packet buffer中protobuf结构反序列化并返回
	template<class ProtoClass>
	ProtoClass ParseToProto()
	{
		ProtoClass proto;
		proto.ParsePartialFromArray(GetBuffer(), GetDataLength());
		return proto;
	}
	// protobuf结构序列化到packet buffer中
	template<class ProtoClass>
	void SerializeToBuffer(const ProtoClass& proto)
	{
		auto total = proto.ByteSizeLong();
		while (GetEmptySize() < total)
		{
			ReAllocBuffer();
		}

		proto.SerializePartialToArray(GetBuffer(), total);
		FillData(total);
	}

    void BackToPool();
	void CleanBuffer();

	char* GetBuffer() const;
	unsigned short GetDataLength() const;
	int GetMsgId() const;
	void FillData(unsigned int size);
	void ReAllocBuffer();
	SOCKET GetSocket() const;

private:
    Proto::MsgId _msgId; // 协议号
	SOCKET _socket;
};
