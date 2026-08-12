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
    int Online;       // 在线人数 / 负载计数
    SOCKET Socket;    // socket

    // proto -> AppInfo，从协议消息填充到自身
    void Parse(Proto::AppInfoSync proto);
};

// 用于记录分布式服务器集群中各节点之间同步应用信息
class SyncComponent
{
public:
    // 更新 / 插入同步信息
    void AppInfoSyncHandle(Packet* pPacket);
    void CmdShow();

protected:
    // 最少负载选择
    bool GetOneApp(APP_TYPE appType, AppInfo* pInfo);
    // 反序列化到本地
    void Parse(Proto::AppInfoSync proto, SOCKET socket);
    // 消息处理 -- 调试命令
    void HandleCmdApp(Packet* pPacket);
    // 消息处理 -- 断线清理
    virtual void HandleNetworkDisconnect(Packet* pPacket);

protected:
    // <appId, AppInfo>
    std::map<int, AppInfo> _apps;
};