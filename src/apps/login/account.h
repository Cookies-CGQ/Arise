#pragma once 

#include "login_obj_mgr.h"
#include "libserver/thread_mgr.h"

class Account : public ThreadObject
{
public:
    bool Init() override;
    void RegisterMsgFunction() override;
    void Update() override;

private:
    // 接收到的玩家账号登录请求
    void HandleAccountCheck(Packet* pPacket);
    // 返回玩家账号登录请求的处理结果
    void HandleAccountCheckToHttpRs(Packet* pPacket);

private:
    // 维护所有正在登录的玩家账号对象
    LoginObjMgr _playerMgr;
};