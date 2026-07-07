#pragma once
#include "disposable.h"
#include "thread_mgr.h"
#include "common.h"

#if ENGINE_PLATFORM != PLATFORM_WIN32
#include <signal.h>
#else
#include <csignal>
#endif

// 启动一个进程服务
template<class APPClass>
inline int MainTemplate()
{
    APPClass* pApp = new APPClass();  // 创建服务
    pApp->InitApp();                  // 初始化服务
    pApp->Run();                      // 本线程持续更新全局时间，是工作线程的时间的来源
    delete pApp;                   
    return 0;
}

// 服务类（每个服务一个进程）
class ServerApp : public IDisposable
{
public:
    // 初始化服务，指定服务类型和工作线程个数
    ServerApp(APP_TYPE appType, int cnt = 10);
    ~ServerApp();
    // 初始化服务
    virtual void InitApp() = 0;
    // 释放自身资源
    void Dispose() override;
    // 启动所有线程
    void StartAllThread() const;
    // 本线程持续更新时间戳
    void Run() const; 
    // 更新全局时间
    void UpdateTime() const;
    // 创建一个网络监听Actor
    bool AddListenerToThread(std::string ip, int port) const;
    // 信号处理
    static void Signalhandler(int signalValue);

protected:
    ThreadMgr* _pThreadMgr;  // 线程管理
    APP_TYPE _appType;       // 服务类型
};