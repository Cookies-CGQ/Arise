#pragma once

#include <json/reader.h>
#include "libserver/entity.h"
#include "libserver/system.h"
#include "login_obj_mgr.h"

// 登录业务逻辑
class Account :public Entity<Account>, public IAwakeSystem<>
{
public:
    void Awake() override;
    virtual void BackToPool() override;

private:
    // 定时任务 -- 同步消息给appmgr进程
    void SyncAppInfoToAppMgr();

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
    // DB服 -> 登录服：角色列表返回
    void HandleQueryPlayerListRs(Packet* pPacket);
    // 客户端 -> 登录服：创建角色
    void HandleCreatePlayer(Packet* pPacket);
    // DB服 -> 登录服：创建角色返回
    void HandleCreatePlayerRs(Packet* pPacket);

private:
    Proto::AccountCheckReturnCode ProcessMsg(Json::Value value) const;

private:
    LoginObjMgr _playerMgr;

    // http
    std::string _method = "";
    std::string _httpIp = "";
    int _httpPort = 0;
};