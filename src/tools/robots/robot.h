#pragma once

#include "libserver/network_connector.h"
#include "libserver/state_template.h"
#include "libserver/robot_state_type.h"
#include "robot_state.h"

class Robot : public NetworkConnector, public StateTemplateMgr<RobotStateType, RobotState, Robot>, public IAwakeFromPoolSystem<std::string>
{
public:
	void Awake(std::string account);
	void Update() override;

    // 获取账号
	std::string GetAccount() const;
    // 发送账号验证请求
	void SendMsgAccountCheck();

    static bool IsSingle() { return false; }

protected:
    // 状态机 -- 状态注册
	void RegisterState() override;

private:
    // 消息处理函数 -- 账号验证响应
	void HandleAccountCheckRs(Robot* pRobot, Packet* pPacket);
    // 消息处理函数 -- 处理服务器返回的玩家列表响应
    void HandlePlayerList(Robot* pRobot, Packet* pPacket);

private:
	std::string _account; // 账号
};