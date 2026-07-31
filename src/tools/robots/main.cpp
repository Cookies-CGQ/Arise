#include "libserver/common.h"
#include "libserver/server_app.h"
#include "robot_mgr.h"
#include "robot_console_login.h"
#include "global_robots.h"

int main(int argc, char *argv[])
{
    GlobalRobots::Instance();

    // 初始化
    const APP_TYPE curAppType = APP_TYPE::APP_ROBOT;
    ServerApp app(curAppType, argc, argv);
    app.Initialize();

    auto pThreadMgr = ThreadMgr::GetInstance();

    // 注册控制台命令
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<RobotConsoleLogin>("login");

    // RobotMgr增加到主线程中
    pThreadMgr->GetEntitySystem()->AddComponent<RobotMgr>();

    // 进入主线程
    app.Run();
    app.Dispose();
}
