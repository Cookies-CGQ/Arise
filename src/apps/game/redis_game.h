#pragma once
#include "libserver/redis_connector.h"

class Packet;

// game服务的redis交互层
class RedisGame : public RedisConnector
{
private:
	// 消息处理注册
	void RegisterMsgFunction() override;

	// 消息处理 -- 定时续期在线心跳，并完成token真正的消费
	void HandlePlayerSyncOnlineToRedis(Packet* pPacket);
	// 消息处理 -- 玩家离线时删除在线标志
	void HandlePlayerDeleteOnlineToRedis(Packet* pPacket);
	// 消息处理 -- 取出token并删除
	void HandleGameTokenToRedis(Packet* pPacket);
};
