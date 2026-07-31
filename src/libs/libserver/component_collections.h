#pragma once

#include <list>
#include <map>
#include <string>
#include "component.h"
#include "disposable.h"

// 组件集合
class ComponentCollections: public IDisposable
{
public:
    ComponentCollections(std::string componentName);
    ~ComponentCollections();

    // 添加组件到集合
    void Add(IComponent* pObj);
    // 从集合中删除组件
    void Remove(uint64 sn);

    // 指定sn获取相应组件
    IComponent* Get(uint64 sn = 0);
    // 获取组件集合
    std::map<uint64, IComponent*>& GetAll();

    // update
    void Swap();
    void Dispose() override;

    // 获取该组件集合的类型名
    std::string GetClassType() const;

private:
    std::string _componentName = "";    // 组件类型名
    // cache fresh
    // <sn : IComponent*>
    std::map<uint64, IComponent*> _objs;      // 读
    std::map<uint64, IComponent*> _addObjs;   // 添
    std::list<uint64> _removeObjs;            // 删
};