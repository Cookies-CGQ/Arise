#pragma once

#include <mutex>
#include <list>
#include "common.h"
#include "component.h"
#include "system.h"
#include "object_pool.h"

class SystemManager;

// 对象池集合
class DynamicObjectPoolCollector : public IDisposable
{
public:
    DynamicObjectPoolCollector(SystemManager* pSys);
    // 释放资源
    void Dispose() override;

    // 获取指定类型的对象池
    template<class T>
    IDynamicObjectPool* GetPool();

    // 更新
    void Update();
    void Show();

private:
    std::map<uint64, IDynamicObjectPool*> _pools;
    SystemManager* _pSystemManager = nullptr;
};

template<class T>
IDynamicObjectPool* DynamicObjectPoolCollector::GetPool()
{
    const auto typeHashCode = typeid(T).hash_code();
    auto iter = _pools.find(typeHashCode);
    if (iter != _pools.end())
    {
        return iter->second;
    }

    // 如果没有该类型的对象池就创建该类型的对象池
    auto pPool = new DynamicObjectPool<T>();
    _pools.insert(std::make_pair(typeHashCode, pPool));
    return pPool;
}
