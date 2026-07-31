#pragma once

#include <functional>
#include <list>
#include "sn_object.h"
#include "common.h"

class IEntity;
class IDynamicObjectPool;
class SystemManager;

using TimerHandleFunction = std::function<void(void)>;

class IComponent: virtual public SnObject
{
public:
    friend class EntitySystem;

    virtual ~IComponent() = default;

    // 设置该组件关联的对象池
    void SetPool(IDynamicObjectPool* pPool);
    // 设置该组件关联的实体
    void SetParent(IEntity* pObj);
    // 设置该组件关联的系统管理
    void SetSystemManager(SystemManager* pObj);

    // 获取该组件关联的实体
    template<class T>
    T* GetParent();
    // 获取该组件关联的实体
    IEntity* GetParent() const;

    // 获取该组件关联的系统管理
    SystemManager* GetSystemManager() const;

    // 归还对象池
    virtual void BackToPool() = 0;
    virtual void ComponentBackToPool();

    // 获取组件类型名
    virtual const char* GetTypeName() = 0;
    // 获取组件类型哈希值
    virtual uint64 GetTypeHashCode() = 0;

protected:
    // 添加定时器
    void AddTimer(const int total, const int durations, const bool immediateDo, const int immediateDoDelaySecond, TimerHandleFunction handler);
    std::list<uint64> _timers; // 定时器列表

    IEntity* _parent = nullptr;                 // 关联的实体
    SystemManager* _pSystemManager = nullptr;   // 关联的系统管理
    IDynamicObjectPool* _pPool = nullptr;       // 关联的对象池
};

template<class T>
T* IComponent::GetParent()
{
    return dynamic_cast<T*>(_parent);
}

template<class T>
class Component :public IComponent
{
public:
    // 获取组件类型名
    const char* GetTypeName() override;
    // 获取组件类型哈希值
    uint64 GetTypeHashCode() override;
};

template <class T>
const char* Component<T>::GetTypeName()
{
    return typeid(T).name();
}

template <class T>
uint64 Component<T>::GetTypeHashCode()
{
    return typeid(T).hash_code();
}
