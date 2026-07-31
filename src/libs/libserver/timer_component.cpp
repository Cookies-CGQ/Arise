#include <utility>
#include "timer_component.h"
#include "global.h"
#include "log4_help.h"
#include "update_component.h"
#include "util_time.h"

// 用于Timer比较 
struct CompareTimer
{
    constexpr bool operator()(const Timer& _Left, const Timer& _Right) const
    {
        return (_Left.NextTime > _Right.NextTime);
    }
};

void TimerComponent::Add(Timer& data)
{
    _heap.emplace_back(data);
    // 如果只有一个定时器在堆中，先建堆
    if(_heap.size() == 1)
    {
        make_heap(_heap.begin(), _heap.end(), CompareTimer());
    }
    // 如果不只一个，则插入堆中并维持堆的结构
    else
    {
        push_heap(_heap.begin(), _heap.end(), CompareTimer());
    }
}

void TimerComponent::Awake()
{
    // 需要帧更新，所以添加UpdateComponent组件
    auto pUpdateComponent = AddComponent<UpdateComponent>();
    pUpdateComponent->UpdataFunction = BindFunP0(this, &TimerComponent::Update); // 绑定帧函数
}

void TimerComponent::BackToPool()
{
    _heap.clear();
}

uint64 TimerComponent::Add(const int total, const int durations, const bool immediateDo, const int immediateDoDelaySecond, TimerHandleFunction handler)
{
    Timer data;
    data.SN = Global::GetInstance()->GenerateSN();
    data.CallCountCur = 0;
    data.CallCountTotal = total;
    data.DurationSecond = durations;
    data.Handler = std::move(handler);
    data.NextTime = timeutil::AddSeconds(Global::GetInstance()->TimeTick, durations);

    if (immediateDo)
    {
        data.NextTime = timeutil::AddSeconds(Global::GetInstance()->TimeTick, immediateDoDelaySecond);
    }

    Add(data);
    return data.SN;
}

void TimerComponent::Remove(std::list<uint64>& timers)
{
    for(auto sn: timers)
    {
        auto iter = std::find_if(_heap.begin(), _heap.end(), [sn](const Timer& one){
            return one.SN == sn;
        });

        if(iter == _heap.end())
            continue;

        _heap.erase(iter);
    }

    // 重新建堆
    make_heap(_heap.begin(), _heap.end(), CompareTimer());
}

bool TimerComponent::CheckTime()
{
    if(_heap.empty())
        return false;

    // 查看堆顶是否触发
    const auto data = _heap.front();
    
    return data.NextTime <= Global::GetInstance()->TimeTick;
}

Timer TimerComponent::PopTimeHeap()
{
    // 将堆顶放置于末尾，并维持堆的结构
    pop_heap(_heap.begin(), _heap.end(), CompareTimer());
    
    // 读取堆顶并删除
    Timer data = _heap.back();
    _heap.pop_back();

    return data;
}

void TimerComponent::Update()
{
    while(CheckTime())
    {
        // 取出已经触发的定时器并执行
        Timer data = PopTimeHeap();
        data.Handler();

        // 处理该定时器是否需要从新加入
        // 调用次数+1
        if(data.CallCountTotal != 0)
        {
            data.CallCountCur++;
        }
        // 是否到达指定调用次数
        if(data.CallCountTotal != 0 && data.CallCountCur >= data.CallCountTotal)
        {
            // 到达指定次数了，不需要再进入时间堆中
        }
        else
        {
            // 重新加入堆中
            data.NextTime = timeutil::AddSeconds(Global::GetInstance()->TimeTick, data.DurationSecond);
            Add(data);
        }
    }
}