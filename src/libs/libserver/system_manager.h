#pragma once

#include <random>
#include <list>
#include "disposable.h"
#include "system.h"
#include "common.h"
#include "thread_type.h"
#include "check_time_component.h"
#include "update_system.h"

class EntitySystem;
class MessageSystem;
class DynamicObjectPoolCollector;

class SystemManager : virtual public IDisposable, public CheckTimeComponent
{
public:
    SystemManager();
    // 添加必要组件
    void InitComponent(ThreadType threadType);
    // 帧更新
    virtual void Update();
    void Dispose() override;

    // 获取消息处理系统
    MessageSystem* GetMessageSystem() const;
    // 获取实体管理系统
    EntitySystem* GetEntitySystem() const;
    // 获取update系统
    UpdateSystem* GetUpdateSystem() const;

    // 获取对象池集合
    DynamicObjectPoolCollector* GetPoolCollector() const;
    // 获取随机数引擎
    std::default_random_engine* GetRandomEngine() const;

    // 添加系统
    void AddSystem(const std::string& name);

protected:
    MessageSystem* _pMessageSystem; // 消息处理系统
    EntitySystem* _pEntitySystem;   // 实体管理
    UpdateSystem* _pUpdateSystem;   // update系统
    std::list<System*> _systems;;   // 其他系统集合
    std::default_random_engine* _pRandomEngine;  // 伪随机数引擎
    DynamicObjectPoolCollector* _pPoolCollector; // 对象池管理
};