#pragma once

#include "libserver/redis_connector.h"

class Packet;

// Login服务的redis连接
class RedisLogin : public RedisConnector
{
private:
    // 注册消息处理函数
    void RegisterMsgFunction() override;

    // 生成token并设置到redis中
    void HandleLoginTokenToRedis(Packet* pPacket);
    // 查询改账号是否已经在线（login在线 or game在线）
    void HandleAccountQueryOnline(Packet* pPacket);

    // 维持账号在login服务的在线状态
    void HandleAccountSyncOnlineToRedis(Packet* pPacket);
    // 删除账号在login服务的在线状态
    void HandleAccountDeleteOnlineToRedis(Packet* pPacket);
};