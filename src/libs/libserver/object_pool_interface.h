#pragma once

class ObjectBlock;

class IDynamicObjectPool
{
public:
    virtual ~IDynamicObjectPool() = default;
    // 帧函数 -- 更新对象池中的对象
    virtual void Update() = 0;
    // 销毁对象池
    virtual void DestroyInstance() = 0;
    // 回收对象
    virtual void FreeObject(ObjectBlock* pObj) = 0;
};

