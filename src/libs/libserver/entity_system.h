#pragma once

#include "component.h"
#include "disposable.h"
#include "component_factory.h"
#include "object_pool.h"
#include "component_collections.h"
#include "system_manager.h"
#include "log4_help.h"
#include "object_pool_collector.h"

class Packet;

// 实体/组件管理 -- 每个线程一个
class EntitySystem: public IDisposable
{
public:
    friend class ConsoleThreadComponent;

    EntitySystem(SystemManager* pMgr);
    virtual ~EntitySystem();

    // 添加组件，这里添加的组件是无上级实体的
    template <class T, typename... TArgs>
    T* AddComponent(TArgs... args);

    // 添加组件，这里添加的组件是指定父类实体的
    template <class T, typename... TArgs>
    T* AddComponentWithParent(IEntity* pParent, uint64 sn, TArgs... args);

    // 添加组件，通过类名动态创建组件
    template <typename... TArgs>
    IComponent* AddComponentByName(std::string className, uint64 sn, TArgs... args);

    // 对于单例性质的组件，通过调用这个函数可以直接获取到这个组件
    template<class T>
    T* GetComponent();

    // 删除组件
    void RemoveComponent(IComponent* pObj);
    
    // 获取组件集合
    template<class T>
    ComponentCollections* GetComponentCollections();

    // 更新
    void Update();
    
    // 释放资源
    void Dispose() override;

private:
    // 添加组件
    template<class T>
    void AddComponent(T* pComponent);

    // TypeHashCode: ComponentCollections*
    std::map<uint64, ComponentCollections*> _objSystems; // 管理所有的实体和组件

private:
    SystemManager* _systemManager; // 关联的系统管理
};

template<class T>
inline void EntitySystem::AddComponent(T* pComponent)
{
    const auto typeHashCode = pComponent->GetTypeHashCode();
#if LOG_SYSOBJ_OPEN
    LOG_SYSOBJ("*[sys] add obj. obj sn:" << pComponent->GetSN() << " type:" << pComponent->GetTypeName() << " thead id:" << std::this_thread::get_id());
#endif

    // 如果不存在这个类型组件的集合就先创建
    auto iter = _objSystems.find(typeHashCode);
    if(iter == _objSystems.end())
    {
        _objSystems[typeHashCode] = new ComponentCollections(pComponent->GetTypeName());
    }

    auto pEntities = _objSystems[typeHashCode];
    pEntities->Add(dynamic_cast<IComponent*>(pComponent));
    pComponent->SetSystemManager(_systemManager);
}

template <class T, typename ... TArgs>
T* EntitySystem::AddComponent(TArgs... args)
{
    return AddComponentWithParent<T>(nullptr, 0, std::forward<TArgs>(args)...);
}

template <class T, typename ... TArgs>
T* EntitySystem::AddComponentWithParent(IEntity* pParent, uint64 sn, TArgs... args)
{
    auto pCollector = _systemManager->GetPoolCollector();
    auto pPool = dynamic_cast<DynamicObjectPool<T>*>(pCollector->GetPool<T>());
    T* pComponent = pPool->MallocObject(_systemManager, pParent, sn, std::forward<TArgs>(args)...);
    if (pComponent == nullptr)
        return nullptr;

    AddComponent(pComponent);
    return pComponent;
}

template<typename ...TArgs>
inline IComponent* EntitySystem::AddComponentByName(std::string className, uint64 sn, TArgs ...args)
{
    auto pObj = ComponentFactory<TArgs...>::GetInstance()->Create(_systemManager, className, sn, std::forward<TArgs>(args)...);
    if (pObj == nullptr)
        return nullptr;

    IComponent* pComponent = static_cast<IComponent*>(pObj);
    AddComponent(pComponent);
    return pComponent;
}

template<class T>
T* EntitySystem::GetComponent()
{
    const auto typeHashCode = typeid(T).hash_code();
    auto iter = _objSystems.find(typeHashCode);
    if(iter == _objSystems.end())
        return nullptr;

    return dynamic_cast<T*>(iter->second->Get());
}

template<class T>
inline ComponentCollections* EntitySystem::GetComponentCollections()
{
    const auto typeHashCode = typeid(T).hash_code();
    auto iter = _objSystems.find(typeHashCode);
    if(iter == _objSystems.end())
    {
        return nullptr;
    }
    
    return iter->second;
}