#pragma once

#include <algorithm>
#include <map>
#include <list>
#include <memory>
#include <queue>
#include "common.h"
#include "entity_system.h"

class IEntity: public IComponent
{
public:
    virtual ~IEntity() = default;
    void ComponentBackToPool() override;

    // 给实体添加组件
    template<class T, typename ... TArgs>
    T* AddComponent(TArgs ... args);

    // 给实体添加组件
    template<class T, typename ... TArgs>
    T* AddComponentWithSn(uint64 sn, TArgs ... args);

    // 获取指定类型的组件
    template<class T>
    T* GetComponent();

    // 删除指定类型的组件
    template<class T>
    void RemoveComponent();

    // 删除组件
    void RemoveComponent(IComponent* pObj);

protected:
    std::map<uint64, IComponent*> _components; // 该实体拥有的组件，type hash code: IComponet
};

template <class T, typename... TArgs>
inline T* IEntity::AddComponent(TArgs... args)
{
    return AddComponentWithSn<T>(0, std::forward<TArgs>(args)...);
}

template <class T, typename ... TArgs>
T* IEntity::AddComponentWithSn(uint64 sn, TArgs... args)
{
    const auto typeHashCode = typeid(T).hash_code();
    if (_components.find(typeHashCode) != _components.end())
    {
        LOG_ERROR("Add same component. type:" << typeid(T).name());
        return nullptr;
    }

    T* pComponent = _pSystemManager->GetEntitySystem()->AddComponentWithParent<T>(this, sn, std::forward<TArgs>(args)...);
    _components.insert(std::make_pair(typeHashCode, pComponent));
    return pComponent;
}


template<class T>
T* IEntity::GetComponent()
{
    const auto typeHashCode = typeid(T).hash_code();
    const auto iter = _components.find(typeHashCode);
    if (iter == _components.end())
        return nullptr;

    return dynamic_cast<T*>(iter->second);
}

template<class T>
void IEntity::RemoveComponent()
{
    // 先删除本地组件数据
    const auto typeHashCode = typeid(T).hash_code();
    const auto iter = _components.find(typeHashCode);
    if(iter == _components.end())
    {
        LOG_ERROR("Entity RemoveComponent error. not find. className:" << typeid(T).name());
        return;
    }

    auto pComponent = iter->second;
    RemoveComponent(pComponent);
}

template<class T>
class Entity : virtual public IEntity
{
public:
    const char* GetTypeName() override;
    uint64 GetTypeHashCode() override;
};

template<class T>
const char* Entity<T>::GetTypeName()
{
    return typeid(T).name();
}

template<class T>
uint64 Entity<T>::GetTypeHashCode()
{
    return typeid(T).hash_code();
}