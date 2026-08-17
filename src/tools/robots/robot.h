#pragma once

#include "libserver/state_template.h"
#include "libserver/robot_state_type.h"
#include "libplayer/player.h"
#include "robot_state.h"

class Robot : public Player, public StateTemplateMgr<RobotStateType, RobotState, Robot>, virtual public IAwakeFromPoolSystem<std::string>
{
public:
    void Awake(std::string account) override;
    void BackToPool() override;
    void Update();
    void NetworkDisconnect();
    void EnterWorld(int worldId);

    // 登录失败退避：延迟到指定毫秒时间点后才允许重连，避免重连风暴
    void SetLoginRetryDelay(uint64 delayMs);
    uint64 GetLoginRetryTime() const { return _loginRetryTime; }

protected:
    void RegisterState() override;

private:
    int _worldId{ 0 };
    uint64 _loginRetryTime{ 0 }; // 允许下次重连的时间点（TimeTick 毫秒）
};