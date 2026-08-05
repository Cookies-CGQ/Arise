#pragma once

#include "libserver/message_system.h"
#include "libserver/component.h"

// 玩家在线管理组件：用于在login服务中维护玩家在Redis中的在线标志。它通过定时心跳和对象池回收清理两个机制来管理在线状态。
class PlayerComponentOnlineInLogin :public Component<PlayerComponentOnlineInLogin>, public IAwakeFromPoolSystem<std::string>
{
public:
    void Awake(std::string account) override;
    // 对象池资源回收，同时删除该玩家在redis上的在线标志
    void BackToPool() override;
    // 定时器 -- 定时心跳维持在线标志
    void SetOnlineFlag() const;

private:
    std::string _account = "";
};