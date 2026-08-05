#pragma once

#include <jsoncpp/json/reader.h>
#include "libserver/entity.h"
#include "libserver/system.h"
#include "libserver/sync_component.h"
#include "libserver/socket_object.h"

class Player;

// 登录业务逻辑
class Account :public Entity<Account>, public IAwakeSystem<>, public SyncComponent
{
public:
    void Awake() override;
    virtual void BackToPool() override;

private:
    // 定时任务 -- 同步消息给appmgr进程
    void SyncAppInfoToAppMgr();
    
    // AppMgr向login同步game服务信息
    void HandleAppInfoListSync(Packet* pPacket);

    // networkConnector组件 -> accout组件：向第三方平台的http连接建立完成，发送验证需求
    void HandleNetworkConnected(Packet* pPacket);
    // 处理断线
    void HandleNetworkDisconnect(Packet* pPacket);
    // 多个账号同时登录，主动关闭socket连接
    void SocketDisconnect(std::string account, NetworkIdentify* pIdentify);
    // 第三方 -> 登录服：账号验证结构返回
    void HandleHttpOuterResponse(Packet* pPacket);
    // 客户端 -> 登录服：账号验证
    void HandleAccountCheck(Packet* pPacket);
    // AppMgr向login同步game服务信息
    void HandleAccountQueryOnlineToRedisRs(Packet* pPacket);
    // DB服 -> 登录服：角色列表返回
    void HandleQueryPlayerListRs(Packet* pPacket);
    // 客户端 -> 登录服：创建角色
    void HandleCreatePlayer(Packet* pPacket);
    // DB服 -> 登录服：创建角色返回
    void HandleCreatePlayerRs(Packet* pPacket);
    // 客户端选择角色
    void HandleSelectPlayer(Packet* pPacket);
    // 请求一个token
    void RequestToken(Player* pPlayer) const;
    // Redis 返回 Token，客户端可以连接Game服务了，完成整个登录流程
    void HandleTokenToRedisRs(Packet* pPacket);

private:
    Proto::AccountCheckReturnCode ProcessMsg(Json::Value value) const;

private:
    // http，第三方验证平台
    std::string _method = ""; // HTTP请求方法
    std::string _httpIp = ""; // 第三方账号验证服务的IP
    int _httpPort = 0;        // 第三方账号验证服务的端口
};