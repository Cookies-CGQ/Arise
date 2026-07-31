#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <mutex>
#include "component.h"

// 组件工厂(模板参数是组件构造函数的参数)
template<typename ... Targs>
class ComponentFactory
{
public:
    typedef std::function<IComponent*(SystemManager*, Targs...)> FactoryFunction;

    static ComponentFactory<Targs...>* GetInstance()
    {
        if(_pInstance == nullptr)
        {
            _pInstance = new ComponentFactory<Targs...>();
        }
        return _pInstance;
    }

    // 注册
    bool Regist(const std::string& className, FactoryFunction pFunc)
    {
        std::lock_guard<std::mutex> guard(_lock);
        if(_map.find(className) != _map.end())
            return false; // 已经注册了
        
        _map.insert(std::make_pair(className, pFunc));
        return true;
    }

    // 是否已经注册
    bool IsRegisted(const std::string& className)
    {
        std::lock_guard<std::mutex> guard(_lock);
        return _map.find(className) != _map.end();
    }

    // 创建组件
    IComponent* Create(SystemManager* pSysMgr, const std::string className, Targs... args)
    {
        _lock.lock();
        auto iter = _map.find(className);
        if(iter == _map.end())
        {
			std::cout << "ComponentFactory Create failed. can't find component. className:" << className.c_str() << std::endl;
            return nullptr;
        }
        auto fun = iter->second;
        _lock.unlock();

        return fun(pSysMgr, std::forward<Targs>(args)...);
    }

private:
    std::mutex _lock;
    static ComponentFactory<Targs...>* _pInstance;
    std::map<std::string, FactoryFunction> _map;     // 类名: 生成函数
};

template<typename... Targs>
ComponentFactory<Targs...>* ComponentFactory<Targs...>::_pInstance = nullptr;