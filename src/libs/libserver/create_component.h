#pragma once

#include "entity.h"
#include "system.h"
#include "message_system.h"

class Packet;
class CreateComponentC :public Entity<CreateComponentC>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

private:
    // 消息处理函数 -- 动态创建一个组件
    void HandleCreateComponent(Packet* pPacket) const;
    // 消息处理函数 -- 移除一个组件
    void HandleRemoveComponent(Packet* pPacket);
};