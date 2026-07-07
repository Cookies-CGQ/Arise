#include <iterator>
#include "thread.h"
#include "global.h"

void ThreadObjectList::AddObject(ThreadObject *obj)
{
    std::lock_guard<std::mutex> guard(_obj_lock);
    // actor初始化
    if (!obj->Init())
    {
        std::cout << "AddObject failed, ThreadObject init failed." << std::endl;
    }
    else
    {
        // 注册协议回调函数
        obj->RegisterMsgFunction();
        // 加入到actor对象列表
        _objlist.GetAddCache()->emplace_back(obj);

        // 设置actor归属线程
        const auto pThread = dynamic_cast<Thread *>(this);
        if (pThread != nullptr)
            obj->SetThread(pThread);
    }
}

void ThreadObjectList::Update()
{
    // 1、 遍历actor对象列表，执行每个actor对象的ProcessPacket消息处理和Update数据更新
    // 2、 如果actor对象不再活跃，则从actor对象列表中移除并释放actor对象
    // 更新actor对象列表
    _obj_lock.lock();
    if (_objlist.CanSwap())
    {
        auto pDelList = _objlist.Swap();
        for (auto e : pDelList)
        {
            e->Dispose();
            delete e;
        }
    }
    _obj_lock.unlock();
    // 更新packet
    _packet_lock.lock();
    if (_cachePackets.CanSwap())
    {
        _cachePackets.Swap();
    }
    _packet_lock.unlock();

    // update
    auto pList = _objlist.GetReaderCache();
    auto pMsgList = _cachePackets.GetReaderCache();
    for (auto iter = pList->begin(); iter != pList->end(); ++iter)
    {
        auto pObj = (*iter);
        // 消息处理
        for (auto itMsg = pMsgList->begin(); itMsg != pMsgList->end(); ++itMsg)
        {
            auto pPacket = (*itMsg);
            if (pObj->IsFollowMsgId(pPacket))
            {
                pObj->ProcessPacket(pPacket);
            }
        }
        // 数据更新
        pObj->Update();

        if (!pObj->IsActive())
        {
            _objlist.GetRemoveCache()->emplace_back(pObj);
        }
    }
    pMsgList->clear();
}

void ThreadObjectList::AddPacketToList(Packet *pPacket)
{
    std::lock_guard<std::mutex> guard(_packet_lock);
    _cachePackets.GetWriterCache()->emplace_back(pPacket);
}

void ThreadObjectList::Dispose()
{
    std::lock_guard<std::mutex> guardObj(_obj_lock);
    _objlist.Dispose();

    std::lock_guard<std::mutex> guardPacket(_packet_lock);
    _cachePackets.Dispose();
}

Thread::Thread()
{
    _state = ThreadState_Init;
}

void Thread::Start()
{
    _thread = std::thread([this]() {
        _state = ThreadState_Run;
        while (!Global::GetInstance()->IsStop)
        {
            Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        const auto theadId = _thread.get_id();
        std::cout << "close thread [1/2]. thread sn:" << this->GetSN() << " thread id:" << theadId << std::endl;
        _state = ThreadState_Stoped; 
    });
}

bool Thread::IsRun() const
{
    return _state == ThreadState_Run;
}

bool Thread::IsStop() const
{
    return _state == ThreadState_Stoped;
}

bool Thread::IsDispose()
{
    if (_thread.joinable())
    {
        const auto theadId = _thread.get_id();
        _thread.join();
        std::cout << "close thread [2/2]. thread sn:" << this->GetSN() << " thread id:" << theadId << std::endl;
        return true;
    }

    return false;
}
