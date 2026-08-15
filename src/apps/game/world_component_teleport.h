#pragma once

#include "libserver/component.h"
#include "libserver/system.h"

class WorldProxy;
class Player;
class ResourceWorld;
class TeleportObject;

// 该组件挂在每个WorldProxy上，主要是管理本代理上所有进行中的传送事务；
// 维护 map<playerSn, TeleportObject*> _objects——每个在传送的玩家一个 TeleportObject；
// 为每个事务发起两条并行的异步线：目标地图就绪、玩家数据同步；
// 收到两个异步回包后推进状态、汇聚检查（Check），条件齐了就真正执行传送并清理事务。

class WorldComponentTeleport :public Component<WorldComponentTeleport>, public IAwakeFromPoolSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

    // 正在传送
    bool IsTeleporting(Player* pPlayer);

    // 入口：创建传送事务
	void CreateTeleportObject(int worldId, Player* pPlayer);
	// 回包：目标地图就绪
    void HandleBroadcastCreateWorldProxy(int worldId, uint64 worldSn);
	// 回包：玩家数据同步完成
    void BroadcastSyncPlayer(uint64 playerSn);

protected:
    // 发起条件一
    void CreateWorldFlag(WorldProxy* pWorldProxy, int targetWorldId, TeleportObject* pObj);
    // 发起条件二
    void CreateSyncFlag(WorldProxy* pWorldProxy, TeleportObject* pObj);
    // 汇聚检查
	bool Check(TeleportObject* pObj);

private:
    // <playersn, obj>
	std::map<uint64, TeleportObject*> _objects; // 每个正在传送的玩家
};