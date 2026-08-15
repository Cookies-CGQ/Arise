#pragma once

#include "libserver/entity.h"

class Packet;

// game进程里的全局单例组件，负责把"世界"翻译成"本进程内对应的 WorldProxy
class WorldProxyLocator :public Entity<WorldProxyLocator>, public IAwakeSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

    // 代理上线注册
	void RegisterToLocator(int worldId, uint64 worldSn);
	// 代理下线注销
    void Remove(int worldId, uint64 worldSn);
    // 是否存在副本地图代理
	bool IsExistDungeon(uint64 worldSn);
	// 是否存在公共地图代理
    uint64 GetWorldSnById(int worldId);

private:
    // 收到世界已创建广播
	void HandleBroadcastCreateWorld(Packet* pPacket);

private:
	std::mutex _lock;

	// <world id, world sn>
	std::map<int, uint64> _publics; // 公共地图

	// <world sn>
	std::set<uint64> _worlds;  // 副本只存sn
};