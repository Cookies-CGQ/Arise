#include "login_app.h"
#include "account.h"
#include "libserver/robot_test.h"

void LoginApp::InitApp()
{
    // 创建监听网络Actor
    AddListenerToThread("127.0.0.1", 2233);
    // 创建测试Actor
    RobotTest* pTest = new RobotTest();
    _pThreadMgr->AddObjToThread(pTest);
    // 创建账户验证Actor
    Account* pAccount = new Account();
    _pThreadMgr->AddObjToThread(pAccount);
}