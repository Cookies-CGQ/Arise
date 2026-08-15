#pragma once

#include "libserver/system.h"
#include "libserver/entity.h"

// worldProxy的组件，用于向WorldProxyGather定时同步自身状态
class WorldProxyComponentGather :public Entity<WorldProxyComponentGather>, public IAwakeFromPoolSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

private:
    // 定时任务 -- 同步自身状态到WorldProxyGather
    void SyncWorldInfoToGather();

private:
    uint64 _worldSn = 0;   // 地图sn
    int _worldId = 0;      // 地图ID
};