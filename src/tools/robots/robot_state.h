#pragma once
#include "libserver/state_template.h"
#include "libserver/robot_state_type.h"

class Robot;

// Robot状态基类
class RobotState : public StateTemplate<RobotStateType, Robot>
{
public:
    // 帧函数
    RobotStateType Update() override;
    // 虚函数，待子类实现逻辑
    virtual RobotStateType OnUpdate()
    {
        return GetState();
    }

    // 进入状态时调用
    void EnterState() override;
    // 虚函数，待子类实现逻辑
    virtual void OnEnterState()
    {

    }

    // 离开状态时调用
    void LeaveState() override;
    // 虚函数，待子类实现逻辑
    virtual void OnLeaveState()
    {

    }
};