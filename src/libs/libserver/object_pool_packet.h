#pragma once

#include <mutex>
#include "object_pool.h"
#include "packet.h"
#include "singleton.h"

/*
    普通对象池属于某个SystemManager，且每个线程只有一个SystemManager，也就是说一个类型的普通线程池只服务于一个线程
    而Packet会跨线程流转，比如可能从网络线程分发到主线程和工作线程中，没有唯一的SystemManager所属者，因此Packet使用全局单例对象池，并且创建时传入nullptr
*/

class DynamicPacketPool: public DynamicObjectPool<Packet>, public Singleton<DynamicPacketPool>
{
public:
    // 从packet对象池获取并初始化一个packet返回
    Packet* MallocPacket(Proto::MsgId msgId, NetIdentify* pIdentify);
    // 检测Packet的引用情况并回收到对象池
    virtual void Update() override;
    // 回收packet对象
    virtual void FreeObject(IComponent* pObj) override;
    virtual void Show() override;    

private:
    std::mutex _packet_lock; // 因为会多线程使用，所以需要解决并发问题
};