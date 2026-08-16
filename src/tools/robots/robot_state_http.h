#pragma once

#include <string>
#include "robot_state.h"

class RobotStateHttpConnecting : public RobotState
{
public:
    DynamicStateCreate(RobotStateHttpConnecting, RobotStateType::Http_Connecting);

    void OnEnterState() override;
    RobotStateType OnUpdate() override;
};

class RobotStateHttpConnected : public RobotState
{
public:
    DynamicStateCreate(RobotStateHttpConnected, RobotStateType::Http_Connected);
    void OnEnterState() override;
    RobotStateType OnUpdate() override;
};