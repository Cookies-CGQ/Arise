#pragma once

#include "common.h"
#include "thread.h"
#include "cache_refresh.h"

class Packet;

// 同类线程集合 -- 位于主线程中（存在多个不同类型的线程集合）
class ThreadCollector :public IDisposable
{
public:
    // 初始化并创建启动线程
    ThreadCollector(ThreadType threadType, int iintNum);

    // 创建num个线程并启动
    void CreateThread(int num);
    // 销毁线程集合
    void DestroyThread();

    // update
    void Update();

    // 是否全部线程停止
    bool IsStopAll();
    // 是否全部线程销毁
    bool IsDestroyAll();
    // 资源释放
    void Dispose() override;

    // 分发packet到其他所有线程
    virtual void HandlerMessage(Packet* pPacket);
    // 分发创建组件消息到其中一个线程（负载均衡）
    virtual void HandlerCreateMessage(Packet* pPacket);

protected:
    ThreadType _threadType;          // 线程集合的类型
    CacheRefresh<Thread> _threads;   // 线程集合
    uint64 _nextThreadSn = 0;        // 用于线程负载均衡    
};

