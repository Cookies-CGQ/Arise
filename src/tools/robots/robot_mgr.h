#pragma once

#include "libserver/util_time.h"
#include "libserver/robot_state_type.h"
#include "libserver/network_connector.h"

class RobotMgr : public NetworkConnector, public IAwakeFromPoolSystem<>
{
public:
    void Awake() override;
    // 状态统计
    void ShowInfo();

    // 是否为单例组件
    static bool IsSingle() { return true; }

private:
    // 消息处理函数 -- 接收状态同步
    void HandleRobotState(Packet* pPacket);
    // 判断能否进入下一阶段
    void NofityServer(RobotStateType maxType);

private:
    bool _isChange = false;  // 标记是否更新
    // <account, RobotStateType>
    std::map<std::string, RobotStateType> _robots;
};