#include <ctime>
#include <sys/time.h>
#include "common.h"
#include "server_app.h"
#include "network_listen.h"
#include "object_pool_mgr.h"

ServerApp::ServerApp(APP_TYPE appType, int cnt)
{
    // 注册信号
    signal(SIGINT, Signalhandler);
    _appType = appType;                  // 服务类型
    DynamicObjectPoolMgr::Instance();    // 对象池管理单例对象初始化
    Global::Instance();                  // global单例对象初始化
    ThreadMgr::Instance();               // 线程管理单例对象初始化
    _pThreadMgr = ThreadMgr::Instance(); // 线程管理单例对象初始化

    // 更新全局时间
    UpdateTime();

    // 创建线程
    _pThreadMgr->NewThread(cnt);
    // 启动所有工作线程
    _pThreadMgr->StartAllThread();
}

ServerApp::~ServerApp()
{
    _pThreadMgr->DestroyInstance();
}

void ServerApp::Signalhandler(const int signalValue)
{
    switch (signalValue)
    {
#if ENGINE_PLATFORM != PLATFORM_WIN32
    case SIGSTOP:
    case SIGQUIT:
#endif

    case SIGTERM:
    case SIGINT:
        Global::GetInstance()->IsStop = true;
        break;
    }

    std::cout << "\nrecv signal. value:" << signalValue << " Global IsStop::" << Global::GetInstance()->IsStop << std::endl;
}


void ServerApp::Dispose()
{
    _pThreadMgr->Dispose();
}

// void ServerApp::StartAllThread() const
// {
//     _pThreadMgr->StartAllThread();
// }

void ServerApp::Run() const
{
    while (!Global::GetInstance()->IsStop)
    {
        UpdateTime();
        _pThreadMgr->Update();
        DynamicObjectPoolMgr::GetInstance()->Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 停止所有线程
    std::cout << "stoping all threads..." << std::endl;
    bool isStop;
    do
    {
        isStop = _pThreadMgr->IsStopAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (!isStop);

    // 释放所有线程资源
    std::cout << "disposing all threads..." << std::endl;

    // 1.子线程资源
    bool isDispose;
    do
    {
        isDispose = _pThreadMgr->IsDisposeAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (!isDispose);

    // 2.主线程资源
    _pThreadMgr->Dispose();

    std::cout << "disposing all pool..." << std::endl;
    DynamicObjectPoolMgr::GetInstance()->Update();
    DynamicObjectPoolMgr::GetInstance()->Dispose();
    DynamicObjectPoolMgr::DestroyInstance();

    Global::DestroyInstance();
    ThreadMgr::DestroyInstance();
}

void ServerApp::UpdateTime() const
{
#if ENGINE_PLATFORM != PLATFORM_WIN32
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    Global::GetInstance()->TimeTick = tv.tv_sec * 1000 +  tv.tv_usec * 0.001;
#else
    auto timeValue = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    Global::GetInstance()->TimeTick = timeValue.time_since_epoch().count();
#endif
}

bool ServerApp::AddListenerToThread(std::string ip, int port) const
{
    // 创建并监听，作为一个监听网络连接的actor对象
    NetworkListen* pListener = new NetworkListen();
    if (!pListener->Listen(ip, port))
    {
        delete pListener;
        return false;
    }
    // 添加到线程中，同时注册到网络定位器（SendPacket依赖此注册）
    _pThreadMgr->AddNetworkToThread(APP_Listen, pListener);
    return true;
}