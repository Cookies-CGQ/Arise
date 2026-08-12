#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <mutex>
#include "component.h"

// 组件工厂(模板参数是组件构造函数的参数)，如果组件的构造函数的参数相同，那组件工厂的实例就是同一个
template<typename ... Targs>
class ComponentFactory
{
public:
	typedef std::function<SnObject*(SystemManager*, uint64 sn, Targs...)> FactoryFunction;

    static ComponentFactory<Targs...>* GetInstance()
    {
        // 标准保证函数内 static 局部变量的初始化是线程安全的
        static ComponentFactory<Targs...> _instance;
        return &_instance;
    }

    // 注册 -- 类名：生成函数
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
    SnObject* Create(SystemManager* pSysMgr, const std::string className, uint64 sn, Targs... args)
	{
		_lock.lock();
		auto iter = _map.find(className);
		if (iter == _map.end())
		{
			std::cout << "ComponentFactory Create failed. can't find component. className:" << className.c_str() << std::endl;
			return nullptr;
		}
		auto fun = iter->second;
		_lock.unlock();

		return fun(pSysMgr, sn, std::forward<Targs>(args)...);
	}

private:
    std::mutex _lock;
    std::map<std::string, FactoryFunction> _map;     // 类名: 生成函数
};