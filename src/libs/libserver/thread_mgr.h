#pragma once 

#include <mutex>
#include <map>
#include "common.h"
#include "thread.h"
#include "singleton.h"

class Packet;
class ThreadObject;
class Network;

// ThreadMgr是单例对象
class ThreadMgr : public Singleton<ThreadMgr>, public ThreadObjectList
{
public:
    ThreadMgr();
    // 启动所有的线程
    void StartAllThread();
    // 是否所有线程都停止了
    bool IsStopAll();
    // 是否所有线程资源都清理了
    bool IsDisposeAll();
    // 清理自身资源
    void Dispose() override;    
    // 创建线程
    void NewThread(int cnt = 1);
    // 添加对象到线程
    bool AddObjToThread(ThreadObject* obj);
    // 添加网络对象到线程
    void AddNetworkToThread(APP_TYPE appType, Network* pNetwork);
    // 广播协议到所有Actor
    void DispatchPacket(Packet* pPacket);
    // 定位到NetworkListen发送pPacket
    void SendPacket(Packet* pPacket);

private:
    // 根据appType查找网络对象
    Network* GetNetwork(APP_TYPE appType);

private:
    uint64 _lastThreadSn {0}; // 上次分配Actor的线程，用于负载均衡
    std::mutex _thread_lock;
    std::map<uint64, Thread*> _threads;

    // NetworkLocator用于快速查找网络对象
    std::mutex _locator_lock;
    std::map<APP_TYPE, Network*> _networkLocator;
};