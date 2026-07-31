#pragma once

#include "mysql_connector.h"
#include "libserver/thread_mgr.h"

// 初始化数据库服务相关组件
inline void InitializeComponentDBMgr(ThreadMgr* pThreadMgr)
{
    pThreadMgr->CreateComponent<MysqlConnector>(MysqlThread, true);
}