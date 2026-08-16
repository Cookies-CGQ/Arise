#pragma once

#include "libserver/entity.h"
#include "libserver/system.h"

class Packet;

// 负责汇聚本进程所有world的负载信息，然后上报给所有的game和appmgr
class WorldGather :public Entity<WorldGather>, public IAwakeFromPoolSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

private:
    // 定时任务 -- 定时上报
	void SyncSpaceInfo();
    // 消息处理 -- 接收world上报
	void HandleWorldSyncToGather(Packet* pPacket);
    // 消息处理 -- 控制台展示
	void HandleCmdWorld(Packet* pPacket);

private:
	// <map sn, map player count>
	std::map<uint64, int> _maps;
};