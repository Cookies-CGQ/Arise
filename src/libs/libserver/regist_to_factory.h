#pragma once

#include <typeinfo>
#include "component_factory.h"
#include "object_pool.h"

// T：类型，Targs：参数；用于注册生成组件的生成函数
template<typename T, typename...Targs>
class RegistToFactory
{
public:
    RegistToFactory()
    {
        ComponentFactory<Targs...>::GetInstance()->Regist(typeid(T).name(), CreateComponent);
    }

    static T* CreateComponent(SystemManager* pSysMgr, uint64 sn, Targs... args)
    {
        // 从对象池获取对象
        auto pCollector = pSysMgr->GetPoolCollector();
        auto pPool = dynamic_cast<DynamicObjectPool<T>*>(pCollector->GetPool<T>());
        return pPool->MallocObject(pSysMgr, nullptr, sn, std::forward<Targs>(args)...);
    }
};

template<typename T, typename...Targs>
class RegistObject
{
public:
    RegistObject()
    {
        ComponentFactory<Targs...>::GetInstance()->Regist(typeid(T).name(), CreateComponent);
    }

    static T* CreateComponent(SystemManager* pSysMgr, uint64 sn, Targs... args)
    {
        // 通过 new 获取对象
        return new T(std::forward<Targs>(args)...);
    }
};