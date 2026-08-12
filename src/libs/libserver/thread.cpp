#include <iterator>
#include "thread.h"
#include "global.h"
#include "entity_system.h"
#include "efficiency_thread_component.h"

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
    _thread = std::thread([this](){
        InitComponent(_threadType);
        _state = ThreadState::Run;
        const auto pGlobal = Global::GetInstance();

#if LOG_EFFICIENCY_COMPONENT_OPEN
        auto pObj = this->GetEntitySystem()->AddComponent<EfficiencyThreadComponent>(_threadType, _thread.get_id());
        timeutil::Time start = 0;
#endif
        while (!pGlobal->IsStop)
        {

#if LOG_EFFICIENCY_COMPONENT_OPEN
            start = pGlobal->TimeTick;
#endif

            Update();

#if LOG_EFFICIENCY_COMPONENT_OPEN
            pObj->UpdateTime(pGlobal->TimeTick - start);
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        Dispose();
        log4cplus::threadCleanup();
        _state = ThreadState::Stop;
    });
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

