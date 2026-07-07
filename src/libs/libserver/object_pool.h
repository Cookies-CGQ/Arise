#pragma once
#include <iostream>
#include <sstream>
#include <queue>
#include <list>
#include "sn_object.h"
#include "object_block.h"
#include "packet.h"
#include "object_pool_interface.h"
#include "thread_obj.h"
#include "cache_refresh.h"
#include "object_pool_mgr.h"

// 对象池 -- 每种类型的对象池全局只会实例化一份
template <typename T>
class DynamicObjectPool :public IDynamicObjectPool
{
public:
    // 获取对象池实例
    static DynamicObjectPool<T>* GetInstance()
    {
        std::lock_guard<std::mutex> guard(_instanceLock);
        if(_pInstance == nullptr)
        {
            // 如果该类型的对象池不存在，则创建并加入到对象池管理器中
            _pInstance = new DynamicObjectPool<T>();
            DynamicObjectPoolMgr::GetInstance()->AddPool(_pInstance);
        }
        // 返回对象池
        return _pInstance;
    }

    // 销毁对象池
    void DestroyInstance() override
    {
        std::lock_guard<std::mutex> guard(_instanceLock);
        if(_pInstance == nullptr)
            return;
        delete _pInstance;
        _pInstance = nullptr;
    }

    DynamicObjectPool();
    ~DynamicObjectPool();

    // 构造初始化对象
    template<typename ...Targs>
    T* MallocObject(Targs... args);
    // 帧函数 -- 更新对象池中的对象
    void Update() override;
    // 回收对象
    void FreeObject(ObjectBlock* pObj) override;
    // 测试
    void Show();

private:
    // 创建一个对象
    void CreateOne();    

private:
    std::queue<T*> _free; // 暂未使用的对象队列
    std::mutex _freeLock;
    
    CacheRefresh<T> _objInUse; // 对象池中对象被使用时的刷新器
    std::mutex _inUseLock;

    static DynamicObjectPool<T>* _pInstance; // 对象池实例
    static std::mutex _instanceLock;
};

template <typename T>
DynamicObjectPool<T>* DynamicObjectPool<T>::_pInstance = nullptr;

template <typename T>
std::mutex DynamicObjectPool<T>::_instanceLock;

template <typename T>
DynamicObjectPool<T>::DynamicObjectPool()
{

}

template <typename T>
void DynamicObjectPool<T>::CreateOne()
{
    T* pObj = new T(this);
    _free.push(pObj);
}

template <typename T>
DynamicObjectPool<T>::~DynamicObjectPool()
{
    Update();
    // 销毁未使用的
    while (_free.size() > 0)
    {
        auto iter = _free.front();
        delete iter;
        _free.pop();
    }
    // 销毁刷新器中的对象
    _objInUse.Dispose();
}

template <typename T>
template <typename ... Targs>
T* DynamicObjectPool<T>::MallocObject(Targs... args)
{
    _freeLock.lock();
    if (_free.size() == 0)
    {
        CreateOne();
    }

    auto pObj = _free.front();
    _free.pop();
    _freeLock.unlock();

    // 重置SN
    pObj->ResetSN();
    // 对象初始化
    pObj->TakeoutFromPool(std::forward<Targs>(args)...);

    _inUseLock.lock();
    _objInUse.GetAddCache()->push_back(pObj);
    _inUseLock.unlock();

    return pObj;
}

template <typename T>
void DynamicObjectPool<T>::Update()
{
    std::list<T*> freeObjs;
    _inUseLock.lock();
    if (_objInUse.CanSwap())
    {
        freeObjs = _objInUse.Swap();
    }
    _inUseLock.unlock();

    std::lock_guard<std::mutex> guard(_freeLock);
    for (auto one : freeObjs)
    {
        _free.push(one);
    }
}

template<typename T>
inline void DynamicObjectPool<T>::FreeObject(ObjectBlock* pObj)
{
    std::lock_guard<std::mutex> guard(_inUseLock);
    _objInUse.GetRemoveCache()->emplace_back(dynamic_cast<T*>(pObj));
}

template <typename T>
void DynamicObjectPool<T>::Show()
{
    std::lock_guard<std::mutex> guard(_freeLock);
    std::lock_guard<std::mutex> guardInUse(_inUseLock);
    auto count = _objInUse.GetReaderCache()->size() + _objInUse.GetAddCache()->size() + _objInUse.GetRemoveCache()->size();

    std::stringstream log;
    log << "*************************** " << "\n";
    log << "pool total count:\t" << _free.size() + count << "\n";
    log << "free count:\t\t" << _free.size() << "\n";
    log << "in use count:\t" << count << "\n";

    std::cout << log.str() << std::endl;
}
