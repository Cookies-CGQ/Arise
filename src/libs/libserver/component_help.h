#pragma once

#include "entity_system.h"
#include "yaml.h"
#include "res_path.h"
#include "trace_component.h"

// 全局便捷方法
class ComponentHelp
{
public:
    static EntitySystem* GetGlobalEntitySystem();
    static Yaml* GetYaml();
    static ResPath* GetResPath();
    static TraceComponent* GetTraceComponent();

#if ENGINE_PLATFORM != PLATFORM_WIN32    
    // 如果bResult为false，则打印当前堆栈
    static void CatchError(bool bResult); 
#endif
};