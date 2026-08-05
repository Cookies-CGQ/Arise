#pragma once

#include "libserver/thread_mgr.h"
#include "world_proxy_gather.h"
#include "lobby.h"
#include "console_cmd_world_proxy.h"
#include "redis_game.h"

inline void InitializeComponentGame(ThreadMgr* pThreadMgr)
{
    // 大厅
    pThreadMgr->CreateComponent<Lobby>();
    // world代理信息汇总器
    pThreadMgr->CreateComponent<WorldProxyGather>();
    // redis连接器
    pThreadMgr->CreateComponent<RedisGame>();
    // 控制台
    auto pConsole = pThreadMgr->GetEntitySystem()->GetComponent<Console>();
    pConsole->Register<ConsoleCmdWorldProxy>("wproxy");
}
