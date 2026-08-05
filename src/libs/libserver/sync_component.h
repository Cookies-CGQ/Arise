#pragma once

#include "common.h"
#include "app_type.h"
#include "entity.h"

class Packet;

struct AppInfo
{
    APP_TYPE AppType; // 服务类型
    int AppId;        // appid
    std::string Ip;   // IP
    int Port;         // port
    int Online;       // 在线人数
    SOCKET Socket;    // socket

    // proto -> AppInfo
    void Parse(Proto::AppInfoSync proto);
};

// 基类 -- 收集某一进程的同步信息
class SyncComponent :public Entity<SyncComponent>
{
public:
    // 消息处理 -- 同步消息
    void AppInfoSyncHandle(Packet* pPacket);
    // 获取指定服务类型的最小负载进程的信息
    bool GetOneApp(APP_TYPE appType, AppInfo& info);
    // 打印命令
    void CmdShow();

protected:
    // <appId, AppInfo>
    std::map<int, AppInfo> _apps;
};

