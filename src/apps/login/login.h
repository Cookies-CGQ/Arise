#pragma once

#include "libserver/thread_mgr.h"
#include "account.h"
#include "redis_login.h"
#include "http_verify_pool.h"

inline void InitializeComponentLogin(ThreadMgr* pThreadMgr)
{
    pThreadMgr->CreateComponent<Account>();
    pThreadMgr->CreateComponent<RedisLogin>();
    // 第三方账号验证连接池（运行在 ConnectThread，预建 8 条长连接）
    pThreadMgr->CreateComponent<HttpVerifyPool>(ConnectThread, false);
}