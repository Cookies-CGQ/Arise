#pragma once

#include "libserver/component.h"
#include "player_component.h"
#include "libserver/vector3.h"
#include "libserver/system.h"

// 世界 + 位置
struct LastWorld
{
	int WorldId = 0;             // 配置表里的地图ID
	uint64 WorldSn = 0;          // 该地图运行时实例的SN
	Vector3 Position{ 0, 0,0 };  // 玩家在这个世界的坐标
    // 普通构造
	LastWorld(const int worldId, const uint64 worldSn, const Vector3 pos);
    // 使用proto结构构造
	LastWorld(Proto::LastWorld proto);
    // 内存数据结构写回到proto中
	void SerializeToProto(Proto::LastWorld* pProto) const;
};

// Player组件，负责记录玩家最后所在的场景
class PlayerComponentLastMap :public Component<PlayerComponentLastMap>, public IAwakeFromPoolSystem<>, public PlayerComponent
{
public:
	void Awake() override;
    void BackToPool() override;

    // 从proto读取数据
	void ParserFromProto(const Proto::Player& proto) override;
	// 写回数据到proto
    void SerializeToProto(Proto::Player* pProto) override;

    // 获取公共地图
	LastWorld* GetLastPublicMap() const;
	// 获取副本地图
    LastWorld* GetLastDungeon() const;
	// 获取当前地图 -- 公共地图 / 副本地图
    LastWorld* GetCur() const;

    // 进入地图时更新记录
	void EnterWorld(int worldId, uint64 worldSn);
	// 设置当前地图
    void SetCurWorld(int worldId);

protected:
    // 进入副本地图
	void EnterDungeon(int worldId, uint64 worldSn, Vector3 position);	

private:
	LastWorld* _pPublic = nullptr;    // 公共地图
	LastWorld* _pDungeon = nullptr;   // 副本地图
	int _curWorldId = 0;              // 玩家当前所在世界的配置ID
};