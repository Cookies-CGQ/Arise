#pragma once

#include "libserver/entity.h"
#include "resource_world.h"

// 单例组件，负责在启动时把策划配置表加载到内存
class ResourceManager :public Entity<ResourceManager>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

public:
    ResourceWorldMgr* Worlds;
};
