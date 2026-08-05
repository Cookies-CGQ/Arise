#pragma once
#include "common.h"

// 性能计时组件，用于在代码中插入检查点来测量各阶段的耗时
class CheckTimeComponent
{
public:
    // 标记计时起点
    void CheckBegin();
    // 检查点
    void CheckPoint(std::string key);

protected:
    uint64 _beginTick; // 起始时间戳 -- 毫秒级
    std::map<std::string, uint64> _aveTime;  // 每个key对应的平滑平均耗时，采用指数移动平均（EMA），平滑因子 α = 0.5
    std::map<std::string, uint64> _maxTicks; // 每个key对应的历史最大耗时
};