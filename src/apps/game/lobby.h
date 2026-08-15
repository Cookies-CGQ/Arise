#pragma once

#include "libplayer/world_base.h"
#include "libserver/system.h"
#include "libserver/entity.h"
#include "libserver/socket_object.h"

class Packet;
class Player;

// 游戏大厅，玩家连接game服务之后，先在Lobby上完成登录校验、加载角色数据；再由Lobby决定把玩家“传送”到哪个世界，传送之后Lobby就不再管这个玩家了
class Lobby : public Entity<Lobby>, public IWorld, public IAwakeSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

private:
    // 找到Player
    Player* GetPlayer(NetIdentify* pIdentify);

    // 消息处理 -- 玩家断线，从PlayerCollector移除
    void HandleNetworkDisconnect(Packet* pPacket);
    // 消息处理 -- 玩家凭token登录
    void HandleLoginByToken(Packet* pPacket);
    // 消息处理 -- Redis 返回 token验证结果
    void HandleGameTokenToRedisRs(Packet* pPacket);
    // 消息处理 -- 查询玩家角色数据返回
    void HandleQueryPlayerRs(Packet* pPacket);
    // 消息处理 -- 查询副本地图是否存在结果返回
    void HandleQueryWorldRs(Packet* pPacket);
    // 消息处理 -- 收到“世界已创建”广播，世界可能是公共地图也可能是副本地图
    void HandleBroadcastCreateWorldProxy(Packet* pPacket);
    // 消息处理 -- player跳转成功之后，清理在lobby的资源
    void HandleTeleportAfter(Player* pPlayer, Packet* pPacket);

    // 进入公共地图
    void EnterPublicWorld(Player* pPlayer);

private:
    // 公共地图 World ID : 等待的玩家SN集合，一个worldId全服只有一个实例进程，所以用worldId可以唯一定位
	std::map<int, std::set<uint64>> _waitingForWorld;
    // 副本地图 World SN : 等待的玩家SN集合，副本资源可以被实例化多次，需要使用worldSN区分同一副本的不同实例
    std::map<uint64, std::set<uint64>> _waitingForDungeon;
};