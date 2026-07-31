#pragma once

#include <list>
#include <mutex>

template<class T>
class CacheSwap
{
public:
    CacheSwap()
    {
        _writerCache = &_caches1;
        _readerCache = &_caches2;
    }

    // 获取写缓冲区指针
    std::list<T*>* GetWriterCache();
    // 获取读缓冲区指针
    std::list<T*>* GetReaderCache();
    // 交换
    void Swap();
    // 是否能交换
    bool CanSwap();
    // 内存回收处理
    void BackToPool();

private:
    std::list<T*> _caches1;       // 缓冲区1
    std::list<T*> _caches2;       // 缓冲区2
    std::list<T*>* _readerCache;  // 读缓冲区指针
    std::list<T*>* _writerCache;  // 写缓冲区指针
};

template<class T>
inline std::list<T*>* CacheSwap<T>::GetWriterCache()
{
    return _writerCache;
}

template<class T>
inline std::list<T*>* CacheSwap<T>::GetReaderCache()
{
    return _readerCache;
}

template<class T>
inline void CacheSwap<T>::Swap()
{
    auto tmp = _readerCache;
    _readerCache = _writerCache;
    _writerCache = tmp;
}

template<class T>
inline bool CacheSwap<T>::CanSwap()
{
    return _writerCache->size() > 0;
}

template<class T>
inline void CacheSwap<T>::BackToPool()
{
    for(auto iter = _caches1.begin(); iter != _caches1.end(); ++iter)
    {
        (*iter)->BackToPool();
    }
    _caches1.clear();

    for(auto iter = _caches2.begin(); iter != _caches2.end(); ++iter)
    {
        (*iter)->BackToPool();
    }
    _caches2.clear();
}