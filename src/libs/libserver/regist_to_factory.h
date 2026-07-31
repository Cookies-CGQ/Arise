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

    // 生成函数
    static T* CreateComponent(SystemManager* pSysMgr, Targs... args)
    {
        // 从对象池获取组件对象
        auto pCollector = pSysMgr->GetPoolCollector();
        auto pPool = (DynamicObjectPool<T>*)pCollector->GetPool<T>();
        return pPool->MallocObject(pSysMgr, std::forward<Targs>(args)...);
    }
};
