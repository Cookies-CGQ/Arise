#pragma once

#include "libserver/entity.h"
#include "libserver/sync_component.h"

// space服务注册表，game服务通过这个组件接收到space服务周期性的同步消息
class SpaceSyncHandler :public SyncComponent, public Entity<SpaceSyncHandler>, public IAwakeSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

    // 选择一个负载最小的space服务信息
	bool GetSpaceApp(AppInfo* pInfo); 

protected:
    // 消息处理 -- 接收处理同步信息
    void HandleAppInfoSync(Packet* pPacket);
};