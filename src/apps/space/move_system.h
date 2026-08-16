#pragma once

#include "libserver/system.h"
#include "libserver/util_time.h"
#include "libserver/component_collections.h"

// 移动系统，负责角色的移动推进
class MoveSystem : public ISystem<MoveSystem>
{
public:
    MoveSystem();
    void Update(EntitySystem* pEntities) override;

private:
    timeutil::Time _lastTime;                       // 上次推进的时间
    ComponentCollections* _pCollections = nullptr;  // MoveComponent 集合
};