#pragma once

#include "libplayer/world_base.h"
#include "libserver/system.h"
#include "libserver/entity.h"

class Packet;
class Player;

class Lobby : public Entity<Lobby>, public IWorld, public IAwakeSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

private:
    // 消息处理 -- 玩家断线，从PlayerCollector移除
    void HandleNetworkDisconnect(Packet* pPacket);
    // 消息处理 -- 玩家凭token登录
    void HandleLoginByToken(Packet* pPacket);
    // 消息处理 -- Redis 返回 token验证结果
    void HandleGameTokenToRedisRs(Packet* pPacket);

private:
    // World ID : 同一 world 的玩家标识
	std::map<int, std::set<uint64>> _waitingForWorld;
};
