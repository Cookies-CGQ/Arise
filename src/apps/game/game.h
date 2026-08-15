#pragma once

#include "libserver/thread_mgr.h"
#include "world_proxy_gather.h"
#include "lobby.h"
#include "console_cmd_world_proxy.h"
#include "redis_game.h"
#include "world_proxy_locator.h"
#include "space_sync_handler.h"
#include "libserver/socket_locator.h"

inline void InitializeComponentGame(ThreadMgr* pThreadMgr)
{
    // 大厅
    pThreadMgr->CreateComponent<Lobby>();
    // world代理信息汇总器
    pThreadMgr->CreateComponent<WorldProxyGather>();
    // redis连接器
    pThreadMgr->CreateComponent<RedisGame>();
    
    // 全局组件
    pThreadMgr->GetEntitySystem()->AddComponent<WorldProxyLocator>();
    pThreadMgr->GetEntitySystem()->AddComponent<SpaceSyncHandler>();
    pThreadMgr->GetEntitySystem()->AddComponent<SocketLocator>();

    // 控制台
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<ConsoleCmdWorldProxy>("proxy");
}