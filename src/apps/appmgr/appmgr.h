#pragma once

#include "libserver/console.h"
#include "app_sync_component.h"
#include "console_cmd_app.h"

// 初始化appmgr服务需要的组件
inline void InitializeComponentAppMgr(ThreadMgr* pThreadMgr)
{
    // 添加一个服务信息同步组件
	pThreadMgr->CreateComponent<AppSyncComponent>();

    // 注册命令
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<ConsoleCmdApp>("app");
}