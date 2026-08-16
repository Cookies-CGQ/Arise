#include "move_system.h"
#include "move_component.h"
#include "libserver/entity_system.h"
#include "libplayer/player.h"
#include "libplayer/player_component_last_map.h"

MoveSystem::MoveSystem()
{
    _lastTime = Global::GetInstance()->TimeTick;
}

void MoveSystem::Update(EntitySystem* pEntities)
{
    // 每 0.5 秒刷一次
    const auto curTime = Global::GetInstance()->TimeTick;
    const auto timeElapsed = curTime - _lastTime;
    if (timeElapsed < 500)
        return;

    if (_pCollections == nullptr)
    {
        _pCollections = pEntities->GetComponentCollections<MoveComponent>();
        if (_pCollections == nullptr)
            return;
    }

    _lastTime = curTime;

    // 遍历所有移动组件
    const auto plists = _pCollections->GetAll();
    for (auto iter = plists->begin(); iter != plists->end(); ++iter)
    {
        auto pMoveComponent = dynamic_cast<MoveComponent*>(iter->second);
        auto pPlayer = pMoveComponent->GetParent<Player>();

        // 速度固定为2
        if (pMoveComponent->Update(timeElapsed, pPlayer->GetComponent<PlayerComponentLastMap>(), 2))
        {
            // 如果移动已经完成，删除移动组件
            pPlayer->RemoveComponent<MoveComponent>();
        }
    }
}