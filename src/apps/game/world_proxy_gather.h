#pragma once

#include "libserver/system.h"
#include "libserver/entity.h"

class Packet;

struct WorldProxyInfo
{
	uint64 WorldProxySn;  // 代理的序列号
	int WorldId;          // world ID
	int Online;           // 在线人数
};

// world代理信息汇总
class WorldProxyGather :public Entity<WorldProxyGather>, public IAwakeSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

private:
	// 定时器 -- 将所有world的在线人数汇总，通过MI_AppInfoSync上报给APP_APPMGR
	void SyncGameInfo();
	// 消息处理 -- 更新_maps中对应world的在线人数
	void HandleWorldProxySyncToGather(Packet* pPacket);
	// 消息处理 -- 打印所有worldproxy信息到日志
	void HandleCmdWorldProxy(Packet* pPacket);

private:
	std::map<uint64, WorldProxyInfo> _maps;
};
