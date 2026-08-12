#pragma once

#include <map>
#include "system.h"
#include "entity.h"
#include "util_time.h"

class Packet;

struct ThreadEfficiencyInfo
{
    ThreadType ThreadTypeKey;    // 线程类型
    uint64 UpdateTime;           // udpate平均时间
    timeutil::Time LastRecvTime; // 最后一次收到该线程上报的时间戳
    uint64 UpdateTimeMax;        // 最大更新耗时
};

// 用于收集和展示服务器各个线程的运行效率数据
class ConsoleEfficiencyComponent :public Entity<ConsoleEfficiencyComponent>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

private:
    // 消息处理 -- 收到控制台消息之后，打印所有被监控线程的效率报表
    void HandleCmdEfficiency(Packet* pPacket);
    // 消息处理 -- 数据收集，收集来自各个线程上报的Proto::Efficiency协议包
    void HandleEfficiency(Packet* pPacket);

private:
    // thread_id, ThreadEfficiencyInfo，进行各个线程的信息缓存
    std::map<std::string, ThreadEfficiencyInfo> _threads;
};