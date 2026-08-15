#pragma once

#include "libserver/system.h"
#include "libserver/entity.h"

class Packet;

struct WorldProxyInfo
{
	uint64 WorldSn;  // 地图sn
	int WorldId;     // 地图id
	int Online;      // 代理负载
};

// world代理信息汇总，定期向AppMgr服务同步本进程所有代理信息
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
	std::map<uint64, WorldProxyInfo> _maps; // worldSN : WorldProxyInfo
};
