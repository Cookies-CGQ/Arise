#pragma once

#include "libserver/entity.h"
#include "libserver/socket_object.h"
#include "libplayer/world_base.h"

class Player;

// 世界代理
class WorldProxy :public IWorld, public Entity<WorldProxy>, public IAwakeFromPoolSystem<int, uint64>
{
public:
    void Awake(int worldId, uint64 lastWorldSn) override;
    void BackToPool() override;

    // 主动发一个proto的消息给真实世界
    void SendPacketToWorld(const Proto::MsgId msgId, ::google::protobuf::Message& proto, Player* pPlayer) const;
    // 主动发一个无body的消息给真实世界
    void SendPacketToWorld(const Proto::MsgId msgId, Player* pPlayer) const;
    // 把收到的Packet深拷贝一份转发给真实世界
    void CopyPacketToWorld(Player* pPlayer, Packet* pPacket) const;

private:
    // 过滤器函数
    Player* GetPlayer(NetIdentify* pIdentify);

    // 消息处理 -- 断线处理
    void HandleNetworkDisconnect(Packet* pPacket);
    // 消息处理 -- 玩家传送进来
    void HandleTeleport(Packet* pPacket);
    // 消息处理 -- 旧世界确认传送完成
    void HandleTeleportAfter(Player* pPlayer, Packet* pPacket);
    // 消息处理 -- 收到“新代理上线”广播
    void HandleBroadcastCreateWorldProxy(Packet* pPacket);
    // 消息处理 -- 客户端请求进入某个世界
    void HandleC2GEnterWorld(Player* pPlayer, Packet* pPacket);
    // 消息处理 -- 真实世界回推玩家数据
    void HandleS2GSyncPlayer(Player* pPlayer, Packet* pPacket);

    // 协议默认处理函数（中转核心）
    void HandleDefaultFunction(Packet* pPacket);

private:
    int _spaceAppId = 0; // 对应的真实世界所在的space服务id
};