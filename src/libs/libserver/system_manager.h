#pragma once

#include <random>
#include <list>
#include "disposable.h"
#include "system.h"
#include "common.h"
#include "thread_type.h"

class EntitySystem;
class MessageSystem;
class DynamicObjectPoolCollector;

class SystemManager: virtual public IDisposable
{
public:
    SystemManager();
    // 添加必要组件
    void InitComponent(ThreadType threadType);
    // 帧更新
    virtual void Update();
    void Dispose() override;

    MessageSystem* GetMessageSystem() const;
    EntitySystem* GetEntitySystem() const;
    DynamicObjectPoolCollector* GetPoolCollector() const;
    std::default_random_engine* GetRandomEngine() const;

protected:
    MessageSystem* _pMessageSystem; // 消息处理系统
    EntitySystem* _pEntitySystem;   // 实体管理
    std::list<ISystem*> _systems;   // 其他系统集合
    std::default_random_engine* _pRandomEngine;  // 伪随机数引擎
    DynamicObjectPoolCollector* _pPoolCollector; // 对象池管理
};