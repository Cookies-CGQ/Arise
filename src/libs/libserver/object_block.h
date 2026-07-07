#pragma once
#include "sn_object.h"
#include "disposable.h"

class IDynamicObjectPool;

class ObjectBlock : virtual public SnObject, virtual public IDisposable
{
public:
    ObjectBlock(IDynamicObjectPool* pPool); 
    virtual ~ObjectBlock();
    // 释放自身资源
    virtual void Dispose() override;
    // 对象回收
    virtual void BackToPool() = 0;   

protected:
    IDynamicObjectPool* _pPool {nullptr}; // 记录对象来自哪个池子，便于后续回收
};