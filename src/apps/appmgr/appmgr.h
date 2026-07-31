#pragma once

#include "libserver/console.h"
#include "libserver/thread_mgr.h"
#include "console_app_component.h"
#include "login_sync_component.h"
#include "console_cmd_app.h"

// 初始化appmgr服务需要的组件
inline void InitializeComponentAppMgr(ThreadMgr* pThreadMgr)
{
    // 添加一个login服务信息同步组件
	pThreadMgr->CreateComponent<LoginSyncComponent>();

    // 注册命令
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<ConsoleCmdApp>("app");
    
    // 给每个LogicThread线程添加ConsoleAppComponent组件
    pThreadMgr->CreateComponent<ConsoleAppComponent>(LogicThread, true);
}