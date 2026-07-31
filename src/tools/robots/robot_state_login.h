#pragma once

#include "robot_state.h"

// 状态类 -- 正在连接
class RobotStateLoginConnecting : public RobotState
{
public:
    DynamicStateCreate(RobotStateLoginConnecting, RobotState_Login_Connecting);

    void OnEnterState() override;
    RobotStateType OnUpdate() override;
};

// 状态类 -- 已连接，发送验证
class RobotStateLoginConnected : public RobotState
{
public:
    DynamicStateCreate(RobotStateLoginConnected, RobotState_Login_Connected);

    void OnEnterState() override;
};

// 状态类 -- 已验证通过
class RobotStateLoginLogined : public RobotState
{
public:
    DynamicStateCreate(RobotStateLoginLogined, RobotState_Login_Logined);
};

// 状态类 -- 已选择角色
class RobotStateLoginSelectPlayer : public RobotState
{
public:
    DynamicStateCreate(RobotStateLoginSelectPlayer, RobotState_Login_SelectPlayer);
};