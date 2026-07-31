#pragma once

#if ENGINE_PLATFORM != PLATFORM_WIN32
#include <signal.h>
#else
#include <csignal>
#endif

#include "disposable.h"
#include "common.h"
#include "thread_mgr.h"
#include "app_type.h"

// 启动一个服务，放置main中
class ServerApp :public Singleton<ServerApp>, public IDisposable
{
public:
    ServerApp(APP_TYPE appType, int argc, char* argv[]);

    // 初始化
    void Initialize();
    // 释放资源
    void Dispose() override;

    // 负责主线程驱动、全局时间更新、其他组件状态更新
    void Run();

    // signal
    static void Signalhandler(int signalValue);

protected:
    ThreadMgr* _pThreadMgr = nullptr;               // 线程管理
    APP_TYPE _appType = APP_TYPE::APP_None;         // 服务类型
    int _appId = 0;                                 // 服务ID -- 每个同类型服务可能有多个，所以用ID进行区分

    int _argc;
    char** _argv;
};