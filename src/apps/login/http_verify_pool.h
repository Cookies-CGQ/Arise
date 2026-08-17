#pragma once

#include <map>
#include <queue>
#include <utility>
#include "libserver/entity.h"
#include "libserver/system.h"
#include "libserver/socket_object.h"

class Packet;

// 第三方账号验证连接池（方案C）
// 预建 N 条到 PHP 的长连接（keep-alive），验证请求复用连接，
// 彻底消除"每账号一条短命连接"的 fd 翻转风暴与串包竞态。
// 响应不再依赖连接标签路由，按响应体中的 account 字段重新路由给 Account。
class HttpVerifyPool : public Entity<HttpVerifyPool>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

private:
    // 发起验证请求（来自 Account，跨线程消息）
    void HandleVerifyRequest(Packet* pPacket);
    // 池连接建立成功（无标签的 HttpConnector 连接）
    void HandleNetworkConnected(Packet* pPacket);
    // 验证响应：归还连接并按 body 中的 account 转发给 Account
    void HandleHttpOuterResponse(Packet* pPacket);
    // 池连接断开：移除记录
    void HandleNetworkDisconnect(Packet* pPacket);
    // 定时检查：补足连接、清理僵死连接
    void CheckConnections();
    // 用指定池连接发送验证请求
    void SendVerify(SOCKET socket, const std::string& account, const std::string& password);
    // 处理排队中的验证请求
    void ProcessQueue();

private:
    std::string _ip = "";    // 第三方验证服务 IP
    int _port = 0;           // 第三方验证服务端口
    std::string _method = ""; // 验证路径（如 /member_login_t.php）

    // 池连接：fd -> 完整 SocketKey（含连接代次）
    std::map<SOCKET, SocketKey> _poolKeys;
    // 池连接空闲标记：fd -> 是否空闲
    std::map<SOCKET, bool> _free;
    // 使用中的连接开始时间（用于僵死连接清理）
    std::map<SOCKET, uint64> _inUseSince;
    // 等待队列：<账号, 密码>
    std::queue<std::pair<std::string, std::string>> _waitQueue;
};
