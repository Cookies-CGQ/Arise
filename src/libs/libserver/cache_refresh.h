#pragma once
#include <algorithm>
#include <list>
#include <map>
#include <queue>
#include "log4_help.h"
#include "disposable.h"

template <class T>
class CacheRefresh: public IDisposable
{
public:
    // 获取读缓冲区
    std::map<uint64, T*>* GetReaderCache();

    // 添加
    void AddObj(T* pObj);
    // 删除
    void RemoveObj(uint64 sn);
    // 缓冲区数据个数
    int Count();

    // 返回删除的Obj，后续是否有内存回收处理
    void Swap(std::queue<T*>* pRecycleList);
    // 是否状态需要更新
    bool CanSwap();
    // 回收到对象池
    void BackToPool();
    // 释放资源
    void Dispose() override;

protected:
    std::map<uint64, T*> _objs; // 读缓冲区
    std::map<uint64, T*> _adds; // 添加缓冲区
    std::list<uint64> _removes;  // 删除缓冲区
};

template <class T>
inline std::map<uint64, T *> *CacheRefresh<T>::GetReaderCache()
{
    return &_objs;
}

template <class T>
inline void CacheRefresh<T>::AddObj(T *pObj)
{
    _adds.emplace(std::make_pair(pObj->GetSN(), pObj));
}

template <class T>
inline void CacheRefresh<T>::RemoveObj(uint64 sn)
{
    _removes.emplace_back(sn);
}

template <class T>
inline int CacheRefresh<T>::Count()
{
    return _objs.size() + _adds.size();
}

template <class T>
void CacheRefresh<T>::Swap(std::queue<T*>* pRecycleList)
{
    if (!_adds.empty())
    {
        _objs.insert(_adds.begin(), _adds.end());
        _adds.clear();
    }

    if (!_removes.empty())
    {
        for (auto one : _removes)
        {
            auto iter = _objs.find(one);
            if (iter == _objs.end())
            {
                LOG_WARN("CacheRefresh Swap error. not find obj to remove. type:" << typeid(T).name() << " sn:" << one);
            }
            else
            {
                if (pRecycleList != nullptr)
                {
                    iter->second->ResetSN(true);
                    pRecycleList->emplace(iter->second);
                } 
                else
                {
                    delete iter->second;
                }

                _objs.erase(iter);
            }
        }
        _removes.clear();
    }
}

template <class T>
inline bool CacheRefresh<T>::CanSwap()
{
    return !_adds.empty() || !_removes.empty();
}

template <class T>
inline void CacheRefresh<T>::BackToPool()
{
    for (auto iter = _adds.begin(); iter != _adds.end(); ++iter)
    {
        iter->second->ComponentBackToPool();
    }
    _adds.clear();

    for (auto iter = _objs.begin(); iter != _objs.end(); ++iter)
    {
        iter->second->ComponentBackToPool();
    }
    _objs.clear();

    _removes.clear();
}

template <class T>
void CacheRefresh<T>::Dispose()
{
    for (auto iter = _adds.begin(); iter != _adds.end(); ++iter)
    {
        auto pObj = iter->second;
        pObj->Dispose();
        delete pObj;
    }
    _adds.clear();

    for (auto iter = _objs.begin(); iter != _objs.end(); ++iter)
    {
        auto pObj = iter->second;
        pObj->Dispose();
        delete pObj;
    }
    _objs.clear();
    _removes.clear();
}
