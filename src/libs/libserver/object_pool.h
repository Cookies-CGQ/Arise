#pragma once

#include <iostream>
#include <sstream>
#include <queue>
#include <iomanip>
#include "sn_object.h"
#include "object_pool_interface.h"
#include "cache_refresh.h"
#include "log4_help.h"
#include "system_manager.h"

template<typename T>
class DynamicObjectPool :public IDynamicObjectPool
{
public:
    // 释放资源
    void Dispose() override;

    // 取出一个对象初始化并返回
    template<typename ... Targs>
    T* MallocObject(SystemManager* pSys, Targs... args);
    
    // 更新对象池内部状态
    virtual void Update() override;
    // 回收对象
    virtual void FreeObject(IComponent* pObj) override;
    // 显示对象池状态信息
    virtual void Show() override;

protected:
    std::queue<T*> _free;       // 空闲队列
    CacheRefresh<T> _objInUse;  // 正在使用

#if _DEBUG
    int _totalCall = 0;
#endif
};

template <typename T>
void DynamicObjectPool<T>::Dispose()
{
    if (_objInUse.Count() > 0)
    {
        std::cout << "delete pool. " << typeid(T).name() << " count:" << _objInUse.Count() << std::endl;
    }

    while (!_free.empty())
    {
        auto obj = _free.front();
        delete obj;
        _free.pop();
    }
}

template<typename T>
template<typename ... Targs>
T* DynamicObjectPool<T>::MallocObject(SystemManager* pSys, Targs... args)
{
    // 如果没有空闲对象，则申请空间
    if(_free.empty())
    {
        // 如果在线程中是单例，那么该线程中该类型的对象池只创建一个即可
        if(T::IsSingle())
        {
            T* pObj = new T();
            pObj->ResetSN(true);
            pObj->SetPool(this);
            _free.push(pObj);
        }
        else
        {
            for(int index = 0; index < 50; index++)
            {
                T* pObj = new T();
                pObj->ResetSN(true);
                pObj->SetPool(this);
                _free.push(pObj);
            }
        }
    }
#if _DEBUG
    ++_totalCall;
#endif
    // 取出一个对象
    auto pObj = _free.front();
    _free.pop();

    if(pObj->GetSN() != 0)
    {
        LOG_ERROR("failed to create type:" << typeid(T).name() << " sn != 0. sn:" << pObj->GetSN());
    }

    pObj->ResetSN();
    pObj->SetPool(this);
    pObj->SetSystemManager(pSys);
    pObj->Awake(std::forward<Targs>(args)...); // 初始化对象

#if LOG_SYSOBJ_OPEN
    LOG_SYSOBJ("*[pool] awake obj. obj sn:" << pObj->GetSN() << " type:" << pObj->GetTypeName() << " thead id:" << std::this_thread::get_id());
#endif

    _objInUse.AddObj(pObj);
    return pObj;
}

template<typename T>
void DynamicObjectPool<T>::Update()
{
    if (_objInUse.CanSwap())
    {
        // 回收的放回_free
        _objInUse.Swap(&_free);
    }
}

template<typename T>
inline void DynamicObjectPool<T>::FreeObject(IComponent* pObj)
{
    if(pObj->GetSN() == 0)
    {
        LOG_ERROR("free obj sn == 0. type:" << typeid(T).name());
        return;
    }

#if LOG_SYSOBJ_OPEN
    LOG_SYSOBJ("*[pool] free obj. obj sn:" << pObj->GetSN() << " type:" << pObj->GetTypeName() << " thead id:" << std::this_thread::get_id());
#endif

    _objInUse.RemoveObj(pObj->GetSN());
}

template <typename T>
void DynamicObjectPool<T>::Show()
{
    std::stringstream log;
    log << " total:" << std::setw(5) << std::setfill(' ') << _free.size() + _objInUse.Count()

#if _DEBUG
        << "    call:" << std::setw(5) << std::setfill(' ') << _totalCall
#endif

        << "    free:" << std::setw(5) << std::setfill(' ') << _free.size()
        << "    use:" << std::setw(5) << std::setfill(' ') << _objInUse.Count()
        << "    " << typeid(T).name();

    LOG_DEBUG(log.str().c_str());
}