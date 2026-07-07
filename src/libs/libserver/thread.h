#pragma once

#include <atomic>
#include <thread>
#include <list>
#include "thread_obj.h"
#include "sn_object.h"
#include "disposable.h"
#include "cache_swap.h"
#include "cache_refresh.h"
#include "packet.h"

enum ThreadState
{
    ThreadState_Init,
    ThreadState_Run,
    ThreadState_Stoped,
};

class ThreadObjectList : public IDisposable
{
public:
    // 添加Actor
    void AddObject(ThreadObject* obj);
    // 线程Update更新
    void Update();
    // 广播协议到所有Actor
    void AddPacketToList(Packet* pPacket);
    // 释放自身资源
    void Dispose() override;
protected:
    // 本线程的所有Actor对象
    std::mutex _obj_lock;
    CacheRefresh<ThreadObject> _objlist;
    // 本线程的所有待处理包 -- packet先由线程同一接管，后面再通过该线程分发到其他Actor
    std::mutex _packet_lock;
    CacheSwap<Packet> _cachePackets;
};

class Thread : public ThreadObjectList, public SnObject 
{
public:
    Thread();
    void Start();

    bool IsRun() const;
    bool IsStop() const;
    bool IsDispose();

private:
    ThreadState _state;
    std::thread _thread;
};