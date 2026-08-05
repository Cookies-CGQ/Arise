#include "common.h"
#include "server_app.h"
#include "res_path.h"
#include "app_type.h"
#include "yaml.h"
#include "object_pool_packet.h"
#include "component_help.h"
#include "global.h"

ServerApp::ServerApp(APP_TYPE appType, int argc, char* argv[])
{
    _appType = appType;
    _argc = argc;
    _argv = argv;
}

void ServerApp::Dispose()
{
    DynamicPacketPool::GetInstance()->Dispose();
    DynamicPacketPool::DestroyInstance();
    ThreadMgr::DestroyInstance();
}

void ServerApp::Initialize()
{
    std::cout << "\ncommand arguments:" << std::endl;
    for (auto count = 0; count < _argc; count++)
        std::cout << "  argv[" << count << "]   " << _argv[count] << std::endl;
    
    // 参数分析，找到-sid=，获取服务ID
    // 例如启动时：./login -sid=101
    for(int argIdx = 1; argIdx < _argc; ++argIdx)
    {
        std::string cmd = _argv[argIdx];
        std::string findcmd = "-sid=";
        std::string::size_type fi1 = cmd.find(findcmd);
        if(fi1 != std::string::npos)
        {
            cmd.erase(fi1, findcmd.size());
            _appId = std::stoi(cmd); // 获取服务ID
            break;
        }
    }

    // 信号捕捉
    signal(SIGINT, Signalhandler);

    Global::Instance(_appType, _appId);

    // Packet对象池
    DynamicPacketPool::Instance();

    // 初始化线程管理类
    _pThreadMgr = ThreadMgr::Instance();
    _pThreadMgr->InitializeGlobalComponent(_appType, _appId);
    _pThreadMgr->InitializeThread(); // InitializeThread只创建工作线程
}

void ServerApp::Signalhandler(const int signalValue)
{
    auto pGlobal = Global::GetInstance();
    switch (signalValue)
    {
#if ENGINE_PLATFORM != PLATFORM_WIN32
    case SIGSTOP:
    case SIGQUIT:
#endif

    case SIGTERM:
    case SIGINT:
        pGlobal->IsStop = true; // 停止线程运行
        break;
    }

    std::cout << "\nrecv signal. value:" << signalValue << " Global IsStop::" << pGlobal->IsStop << std::endl;
}

void ServerApp::Run()
{
    log4cplus::initialize();

    auto pGlobal = Global::GetInstance();
    while (!pGlobal->IsStop)
    {
        pGlobal->UpdateTime(); // 更新全局时间
        _pThreadMgr->Update(); // 主线程update
        DynamicPacketPool::GetInstance()->Update(); // packet对象池更新
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 休眠1ms
    }

    // 等待所有线程停止
    std::cout << "stoping all threads..." << std::endl;
    bool isStop = false;
    do
    {
        isStop = _pThreadMgr->IsStopAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (!isStop);

    // 等待所有线程销毁
    std::cout << "destroy all threads..." << std::endl;
    _pThreadMgr->DestroyThread();
    bool isDestroy = false;
    do
    {
        isDestroy = _pThreadMgr->IsDestroyAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (!isDestroy);

    // 资源清理
    _pThreadMgr->Dispose();

    log4cplus::deinitialize();
}