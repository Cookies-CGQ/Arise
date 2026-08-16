#pragma once
#include "libserver/system.h"
#include "libplayer/world_base.h"
#include "libserver/entity.h"
#include "libserver/socket_object.h"

class Player;

// 一张运行时的地图，负责玩家进出、互相可见、移动、掉线存档、在线人数上报等
class World :public Entity<World>, public IWorld, public IAwakeFromPoolSystem<int>
{
public:
    void Awake(int worldId) override;
    void BackToPool() override;

protected:
    // 过滤函数
    Player* GetPlayer(NetIdentify* pIdentify);

    // 广播发送网络消息
    void BroadcastPacket(Proto::MsgId msgId, google::protobuf::Message& proto);
    void BroadcastPacket(Proto::MsgId msgId, google::protobuf::Message& proto, std::set<uint64> players);

    // 消息处理 -- 断线处理（玩家/上游进程整体断线）
    void HandleNetworkDisconnect(Packet* pPacket);
    // 消息处理 -- 玩家进图
    void HandleSyncPlayer(Packet* pPacket);
    // 消息处理 -- game 在传送流程中要拿玩家数据 → SerializeToProto → S2G_SyncPlayer 发回玩家 socket(即发回 game)
    void HandleRequestSyncPlayer(Player* pPlayer, Packet* pPacket);
    // 消息处理 -- game 确认玩家已传送到新世界 → 校验 sn 一致 → RemovePlayerBySn → 广播 S2C_RoleDisAppear{sn} 通知全图"此人消失"
    void HandleG2SRemovePlayer(Player* pPlayer, Packet* pPacket);
    // 消息处理 -- 移动处理
    void HandleMove(Player* pPlayer, Packet* pPacket);

private:
    // 定时任务 -- 向WorldGather定时上报
    void SyncWorldToGather();
    // 定时任务 -- 朴素AOI，1秒出现广播
    void SyncAppearTimer();

private:
    // 缓存1秒内增加或是删除的玩家
    std::set<uint64> _addPlayer;
};