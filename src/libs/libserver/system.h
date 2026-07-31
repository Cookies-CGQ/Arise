#pragma once

#include "disposable.h"

class EntitySystem;

class ISystem: public IDisposable
{
protected:
    ISystem() = default;

public:
    virtual ~ISystem() = default;
    void Dispose() override
    {

    }
    virtual void Update(EntitySystem* pEntities)
    {

    }
};

// 线程单例
template <typename... TArgs>
class IAwakeSystem : virtual public ISystem
{
public:
    IAwakeSystem() = default;
    virtual ~IAwakeSystem() = default;
    // 用于在对象池中初始化取出来的对象
    virtual void Awake(TArgs ... args) = 0;
    // 对象池为空时只创建一个对象
    static bool IsSingle()
    {
        return true;
    }
};

// 非线程单例
template <typename... TArgs>
class IAwakeFromPoolSystem : virtual public ISystem
{
public:
    IAwakeFromPoolSystem() = default;
    virtual ~IAwakeFromPoolSystem() = default;
    virtual void Awake(TArgs... args) = 0;
    static bool IsSingle()
    {
        return false;
    }
};