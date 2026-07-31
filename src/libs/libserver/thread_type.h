#pragma once

enum ThreadType
{
    MainThread = 1 << 0, // 主线程
    ListenThread = 1 << 1, // 网络监听线程
    ConnectThread = 1 << 2, // 主动连接线程
    LogicThread = 1 << 3, // 逻辑线程
    MysqlThread = 1 << 4, // 数据库线程
    AllThreadType = MainThread | LogicThread | ListenThread | ConnectThread | MysqlThread,
};

// 通过线程类型获取线程类型名
inline const char* GetThreadTypeName(const ThreadType threadType)
{
    if (threadType == MainThread)
        return "MainThread";

    if (threadType == LogicThread)
        return "LogicThread";

    if (threadType == ListenThread)
        return "ListenThread";

    if (threadType == ConnectThread)
        return "ConnectThread";

    if (threadType == MysqlThread)
        return "MysqlThread";
    
    if(threadType == AllThreadType)
        return "AllThreadType";

    return "unknown";
}
