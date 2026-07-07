#pragma once 
#include <iostream>
#include <list>
#include <mutex>
#include "disposable.h"

template <class T>
class CacheSwap : public IDisposable
{
public:
    CacheSwap()
    {
        _readerCache = &_caches1;
        _writerCache = &_caches2;
    }

    // 获取写入缓存指针
    std::list<T*>* GetWriterCache();
    // 获取读取缓存指针
    std::list<T*>* GetReaderCache();
    // 指针交互
    void Swap();
    // 是否指针可以交互 -- 写入缓存是否有数据
    bool CanSwap();
    // 释放自身资源
    void Dispose() override;

private:
    std::list<T*> _caches1; // 缓存1
    std::list<T*> _caches2; // 缓存2
    std::list<T*>* _readerCache; // 读取缓存
    std::list<T*>* _writerCache; // 写入缓存
};

template <class T>
inline std::list<T*>* CacheSwap<T>::GetWriterCache()
{
    return _writerCache;
}

template <class T>
inline std::list<T*>* CacheSwap<T>::GetReaderCache()
{
    return _readerCache;
}

template <class T>
inline void CacheSwap<T>::Swap()
{
    auto tmp = _readerCache;
    _readerCache = _writerCache;
    _writerCache = tmp;
}

template <class T>
inline bool CacheSwap<T>::CanSwap()
{
    return _writerCache->size() > 0;
}

template <class T>
inline void CacheSwap<T>::Dispose()
{
    _caches1.clear();
    _caches2.clear();
    _readerCache = _writerCache = nullptr;
}