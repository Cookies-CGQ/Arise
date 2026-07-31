#pragma once
#include "entity.h"
#include "system.h"

class Packet;

// 机器人测试
// 启动一批机器人，让它们在各种状态（登录、移动、战斗等）之间流转，通过这个系统自动统计每个状态转换的耗时，帮助定位服务端状态机的性能瓶颈。消息由外部（如压测客户端）发送 MI_RobotTestBegin 和 MI_RobotTestEnd 来划定测量区间。
class RobotTest :public Entity<RobotTest>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override {};

private:
    // 收到测试开始消息时执行，记录当前时间戳
    void HandleTestBegin(Packet* pPacket);
    // 收到测试结束消息时执行，记录结束时间戳
    void HandleTestEnd(Packet* pPacket);

private:
    std::chrono::system_clock::time_point _start;
};

