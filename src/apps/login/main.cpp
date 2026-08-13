#include "libserver/common.h"
#include "libserver/server_app.h"
#include "libserver/network_listen.h"
#include "libserver/network_connector.h"
#include "libserver/global.h"
#include "login.h"
#include "libresource/resource_manager.h"

int main(int argc, char* argv[])
{
    // login服务
    const APP_TYPE curAppType = APP_TYPE::APP_LOGIN;
    ServerApp app(curAppType, argc, argv);
    app.Initialize(); // 创建并运行logic线程

    auto pGlobal = Global::GetInstance();

    auto pThreadMgr = ThreadMgr::GetInstance();
    InitializeComponentLogin(pThreadMgr);

    pThreadMgr->GetEntitySystem()->AddComponent<ResourceManager>();

    // tcp listen -- 用于客户端连接
    pThreadMgr->CreateComponent<NetworkListen>(ListenThread, false, (int)pGlobal->GetCurAppType(), (int)pGlobal->GetCurAppId());

    // tcp connector -- 用于连接appmgr服务和db服务
    pThreadMgr->CreateComponent<NetworkConnector>(ConnectThread, false, (int)NetworkType::TcpConnector, (int)(APP_APPMGR|APP_DB_MGR));
    // http connector -- 用于连接第三方平台
    pThreadMgr->CreateComponent<NetworkConnector>(ConnectThread, false, (int)NetworkType::HttpConnector, (int)0);

    app.Run();
    app.Dispose();

    return 0;
}