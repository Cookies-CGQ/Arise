#pragma once

#include <thread>
#include "system.h"
#include "thread_type.h"
#include "entity.h"

#define ThreadEfficiencyTime 5

class Packet;

// 线程效率监控组件，用于收集和上报每个线程执行 Update 的平均耗时
class EfficiencyThreadComponent :public Entity<EfficiencyThreadComponent>, public IAwakeSystem<ThreadType, std::thread::id>
{
public:
    void Awake(ThreadType threadType, std::thread::id threadId) override;
    void BackToPool() override;

    // 被外部调用，传入本次Update的耗时
    void UpdateTime(uint64 disTime);
    // 定时任务 -- 同步上报信息给其他监控模块消费
    void Sync() const;

private:
    // test update 执行平均时间
    uint64 _efficiencyUpdateTime = 0;                // 执行平均时间 -- 指数移动平均
    std::string _threadId = "";                      // 线程ID
    ThreadType _threadType = ThreadType::MainThread; // 线程类型
};