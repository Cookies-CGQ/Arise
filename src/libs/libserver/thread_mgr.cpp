#include <iostream>
#include "thread_mgr.h"
#include "common.h"
#include "network.h"
#include "network_listen.h"

ThreadMgr::ThreadMgr()
{

}

void ThreadMgr::StartAllThread()
{
    auto iter = _threads.begin();
    while(iter != _threads.end())
    {
        iter->second->Start();
        ++iter;
    }
}

void ThreadMgr::NewThread(int cnt)
{
    std::lock_guard<std::mutex> guard(_thread_lock);
    for(int i = 0; i < cnt; ++i)
    {
        auto pThread = new Thread();
        _threads.insert(std::make_pair(pThread->GetSN(), pThread));
    }
}

bool ThreadMgr::AddObjToThread(ThreadObject* obj)
{
    std::lock_guard<std::mutex> guard(_thread_lock);

    auto iter = _threads.begin();
    // 一个线程都没有
    if(iter == _threads.end())
    {
        std::cout << "AddThreadObj Failed. no thead." << std::endl;
        return false;
    }
    if(_lastThreadSn > 0)
    {
        iter = _threads.find(_lastThreadSn);
        if(iter == _threads.end())
            iter = _threads.begin(); // 重新开始
    }

    // 加入到下一个活跃线程
    do
    {
        ++iter;
        if(iter == _threads.end())
            iter = _threads.begin();
    }while(!(iter->second->IsRun()));

    auto pThread = iter->second;
    pThread->AddObject(obj);
    _lastThreadSn = pThread->GetSN();
    
    return true;
}

void ThreadMgr::AddNetworkToThread(APP_TYPE appType, Network* pNetwork)
{
    if (!AddObjToThread(pNetwork))
        return;

    std::lock_guard<std::mutex> guard(_locator_lock);
    _networkLocator[appType] = pNetwork;
}

Network* ThreadMgr::GetNetwork(APP_TYPE appType)
{
    std::lock_guard<std::mutex> guard(_locator_lock);
    auto iter = _networkLocator.find(appType);
    if(iter == _networkLocator.end())
        return nullptr;
    
    return iter->second;
}

bool ThreadMgr::IsStopAll()
{
    std::lock_guard<std::mutex> guard(_thread_lock);
    for (auto iter = _threads.begin(); iter != _threads.end(); ++iter) 
    {
        if (!iter->second->IsStop())
        {
            return false;
        }
    }
    return true;
}

bool ThreadMgr::IsDisposeAll()
{
    std::lock_guard<std::mutex> guard(_thread_lock);
    for (auto iter = _threads.begin(); iter != _threads.end(); ++iter)
    {
        if (!iter->second->IsDispose())
        {
            return false;
        }
    }
    return true;
}

void ThreadMgr::Dispose()
{
	ThreadObjectList::Dispose();

    std::lock_guard<std::mutex> guard(_thread_lock);
    auto iter = _threads.begin();
    while (iter != _threads.end())
    {
        Thread* pThread = iter->second;
        pThread->Dispose();
        delete pThread;
        ++iter;
    }
    _threads.clear();
}

// void ThreadMgr::AddPacket(Packet* pPacket)
// {
//     std::lock_guard<std::mutex> guard(_thread_lock);
//     for (auto iter = _threads.begin(); iter != _threads.end(); ++iter)
//     {
//         Thread* pThread = iter->second;
//         pThread->AddPacket(pPacket);
//     }
// }

void ThreadMgr::DispatchPacket(Packet* pPacket)
{
    // 这里是thread_mgr的主线程
    AddPacketToList(pPacket);

    // 子线程（工作线程）
    std::lock_guard<std::mutex> guard(_thread_lock);
    for(auto iter = _threads.begin(); iter != _threads.end(); ++iter)
    {
        iter->second->AddPacketToList(pPacket);
    }
}

void ThreadMgr::SendPacket(Packet* pPacket)
{
    NetworkListen* pLocator = static_cast<NetworkListen*>(GetNetwork(APP_Listen));
    pLocator->SendPacket(pPacket);
}