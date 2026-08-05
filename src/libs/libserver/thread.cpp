#include <iterator>
#include "thread.h"
#include "global.h"
#include "entity_system.h"
#include "log4.h"
#include "component_help.h"

Thread::Thread(ThreadType threadType)
{
    _state = ThreadState::Init;
    _threadType = threadType;
}

Thread::~Thread()
{

}

void Thread::Start()
{
    if(_state == ThreadState::Run)
        return;
    
    _thread = std::thread([this](){
            // 给每个线程添加必须的组件
            InitComponent(_threadType);
            // 改变状态
            _state = ThreadState::Run;

            auto pGlobal = Global::GetInstance();
            while (!pGlobal->IsStop)
            {
                // 驱动
                Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            Dispose();
            log4cplus::threadCleanup();
            _state = ThreadState::Stop;
        }
    );
}

bool Thread::IsStop() const
{
    return _state == ThreadState::Stop;
}

bool Thread::IsDestroy() const
{
    return _state == ThreadState::Destroy;
}

void Thread::DestroyThread()
{
    if (_state == ThreadState::Destroy)
        return;

    if (_thread.joinable())
    {
        _thread.join();
        _state = ThreadState::Destroy;
    }
}

void Thread::Dispose()
{
    if (_state == ThreadState::Destroy)
        return;

    SystemManager::Dispose();
}

