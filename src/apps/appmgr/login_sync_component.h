#pragma once

#include <json/writer.h>
#include "libserver/sync_component.h"
#include "libserver/system.h"

// 收集多个Login服务的同步消息
class LoginSyncComponent :public SyncComponent, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

protected:
    // 消息处理函数 -- 处理获取login请求
    void HandleHttpRequestLogin(Packet* pPacket);

private:
    Json::StreamWriter* _jsonWriter = nullptr; // JSON序列化器
};
