#pragma once 
#include <algorithm>
#include <vector>
#include <list>
#include "disposable.h"

template <class T>
class CacheRefresh : public IDisposable
{
public:
    // 获取add 缓存
    std::vector<T*>* GetAddCache();
    // 获取remove 缓存
    std::vector<T*>* GetRemoveCache();
    // 获取reader 缓存
    std::vector<T*>* GetReaderCache();
    // refresh 操作 -- 处理add和remove更新到主缓存reader中;返回删除的obj，后续是否有内存回收处理
    std::list<T*> Swap();
    // 是否可以进行refresh 操作
    bool CanSwap();
    // 释放自身资源
    void Dispose() override;

protected:
    std::vector<T*> _reader;  // 主缓存
    std::vector<T*> _add;     // 新增缓存
    std::vector<T*> _remove;  // 删除缓存
};

template <class T>
inline std::vector<T*>* CacheRefresh<T>::GetAddCache()
{
    return &_add;
}

template <class T>
inline std::vector<T*>* CacheRefresh<T>::GetRemoveCache()
{
    return &_remove;
}

template <class T>
inline std::vector<T*>* CacheRefresh<T>::GetReaderCache()
{
    return &_reader;
}

template <class T>
inline std::list<T*> CacheRefresh<T>::Swap()
{
    std::list<T*> rs;
    // 新增
    for(auto e : _add)
    {
        _reader.push_back(e);
    }
    _add.clear();
    // 删除
    for(auto e : _remove)
    {
        auto iterReader = std::find_if(_reader.begin(), _reader.end(), [e](auto x){
            return x == e;
        });
        
        if(iterReader == _reader.end())
        {
            std::cout << "CacheRefresh Swap error. not find obj to remove. sn:" << e->GetSN() << std::endl;
        }
        else
        {
            rs.push_back(e);
            _reader.erase(iterReader);
        }
    }
    _remove.clear();
    
    return rs;
}

template <class T>
bool CacheRefresh<T>::CanSwap()
{
    return _add.size() > 0 || _remove.size() > 0;
}

template <class T>
void CacheRefresh<T>::Dispose()
{
    for (auto iter = _add.begin(); iter != _add.end(); ++iter)
    {
        (*iter)->Dispose();
        delete (*iter);
    }
    _add.clear();

    for (auto iter = _remove.begin(); iter != _remove.end(); ++iter)
    {
        (*iter)->Dispose();
        delete (*iter);
    }
    _remove.clear();

    for (auto iter = _reader.begin(); iter != _reader.end(); ++iter)
    {
        (*iter)->Dispose();
        delete (*iter);
    }
    _reader.clear();
}