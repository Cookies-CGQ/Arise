#pragma once

#include "component.h"
#include "disposable.h"

// 对象池接口
class IDynamicObjectPool:public IDisposable
{
public:
    // 更新对象池内部状态
    virtual void Update() = 0;
    // 回收对象
    virtual void FreeObject(IComponent* pObj) = 0;
    // 显示对象池状态信息
    virtual void Show() = 0;
};