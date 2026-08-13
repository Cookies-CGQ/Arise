#pragma once

#include "app_sync_component.h"
#include "create_world_component.h"
#include "console_cmd_create.h"

// 初始化appmgr服务需要的组件
inline void InitializeComponentAppMgr(ThreadMgr* pThreadMgr)
{
	pThreadMgr->CreateComponent<AppSyncComponent>();
    pThreadMgr->CreateComponent<CreateWorldComponent>();

    // 注册命令
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<ConsoleCmdCreate>("create");
}