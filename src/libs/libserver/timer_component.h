#pragma once

#include <list>
#include <vector>
#include "util_time.h"
#include "system.h"
#include "entity.h"

struct Timer
{
    // 下次调用时间
    timeutil::Time NextTime;

    // 调用函数
    TimerHandleFunction Handler;

    // 首次执行时延迟秒
    int DelaySecond;

    // 间隔时间(秒）
    int DurationSecond;

    // 总调用次数（0为无限）
    int CallCountTotal;

    // 当前调用次数 
    int CallCountCur;

    // 方便删除数据时找到Timer
    uint64 SN;
};

// 定时器实体，用于处理定时器，一个线程只需要一个定时器实体
class TimerComponent :public Entity<TimerComponent>, public IAwakeSystem<>
{
public:
    // 初始化
    void Awake() override;
    // 归还对象池
    void BackToPool() override;

    // 添加定时器--total：总调用次数（0为无限）；durations：间隔时间（秒）；immediateDo：是否马上执行；immediateDoDelaySecond：首次执行与当前时间的间隔时间；handler：执行函数
    uint64 Add(int total, int durations, bool immediateDo, int immediateDoDelaySecond, TimerHandleFunction handler);
    // 删除定时器
    void Remove(std::list<uint64>& timers);

    // 是否有已经触发的定时器
    bool CheckTime();
    // 定时器基于时间堆实现，弹出堆顶
    Timer PopTimeHeap();

    // 更新
    void Update();

protected:
    void Add(Timer& data);

private:
    std::vector<Timer> _heap; // 维护时间堆
};
