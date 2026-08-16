#pragma once

#include "libserver/entity.h"
#include "libserver/system.h"

class Packet;

// 进程单例组件 -- 整个space进程里所有world实体都是从这里诞生
class WorldOperatorComponent : public Entity<WorldOperatorComponent>, public IAwakeSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

private:
    // 消息处理 -- 生成地图实体（发送方：appmgr服务创建公共地图的请求和game服务创建副本的请求）
	void HandleCreateWorld(Packet* pPacket);
};