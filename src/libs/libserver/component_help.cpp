#include "component_help.h"
#include "thread_mgr.h"

#if ENGINE_PLATFORM != PLATFORM_WIN32
#include <execinfo.h>
#endif

EntitySystem* ComponentHelp::GetGlobalEntitySystem()
{
    return ThreadMgr::GetInstance()->GetEntitySystem();
}

Yaml* ComponentHelp::GetYaml()
{
    return ThreadMgr::GetInstance()->GetEntitySystem()->GetComponent<Yaml>();
}

ResPath* ComponentHelp::GetResPath()
{
    return ThreadMgr::GetInstance()->GetEntitySystem()->GetComponent<ResPath>();
}

TraceComponent* ComponentHelp::GetTraceComponent()
{
    return ThreadMgr::GetInstance()->GetEntitySystem()->GetComponent<TraceComponent>();
}

#if ENGINE_PLATFORM != PLATFORM_WIN32
void ComponentHelp::CatchError(bool bResult)
{
    if (bResult)
        return;

    void* array[15]; // 指针数组，用于存储当前调用栈的返回地址，最多捕获15层栈帧 -- 超过10层的调用会被截断
    size_t size;     // 实际捕获的帧数（<= 15）
    char** strings;
    size_t i;

    size = backtrace(array, 15);
    strings = backtrace_symbols(array, size); // 将地址解析为符号名

    std::stringstream ss;
    ss << "Obtained " << size << " stack frames.\n";

    for (i = 0; i < size; i++)
        ss << strings[i] << "\n";
        
    LOG_ERROR(ss.str().c_str());
    free(strings);
}
#endif