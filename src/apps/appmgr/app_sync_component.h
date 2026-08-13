#pragma once

#include <jsoncpp/json/writer.h>
#include "libserver/sync_component.h"
#include "libserver/system.h"

// 收集多个服务的同步消息，所有业务进程启动后都会主动连到AppMgr通过TCP上报自己的状态
// 主要能力：客户端登录入口查询；向Login推送Game列表；
class AppSyncComponent :public Entity<AppSyncComponent>, public SyncComponent, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

protected:
    // 消息处理函数 -- 连接断开
    void HandleNetworkDisconnect(Packet* pPacket) override;
    // 消息处理函数 -- 处理获取login请求
    void HandleHttpRequestLogin(Packet* pPacket);
    // 消息处理函数 -- 获取game服务同步的信息
    void HandleAppInfoSync(Packet* pPacket);

private:
    // 定时器 -- 定时同步game服务的消息到login服务中
    void SyncGameInfoToLogin();

private:
    Json::StreamWriter* _jsonWriter = nullptr; // JSON序列化器
};
