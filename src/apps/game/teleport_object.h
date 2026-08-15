#pragma once

#include "libserver/component.h"
#include "libserver/system.h"

// 跳转的前置异步条件：
// 1,跳转需要等待目标世界的 WorldProxy 已经存在（公共地图可能已存在、可能要申请创建；副本则要现场创建）
// 2,跳转需要等待旧世界（真实 World）把玩家的完整数据推回来了。
// TeleportObject 就是记录"这一次传送"进度的对象

// 条件三态
enum class TeleportFlagType
{
    None = 0,        // 未开始
    Waiting = 1,     // 异步请求已发出，等待回包
    Completed = 2,   // 条件已满足
};

// 可等待的异步结果，把异步条件抽象成通用模板
template<typename T>
struct TeleportFlag
{
public:
    friend class TeleportObject;

    void SetValue(T value)
    {
        this->Value = value;
        this->Flag = TeleportFlagType::Completed;
    }

    T GetValue()
    {
        return this->Value;
    }

    // 条件是否完成
    bool IsCompleted()
    {
        return this->Flag == TeleportFlagType::Completed;
    }

public:
    TeleportFlagType Flag; // 状态

private:
    // 条件满足后的产出，两个 Flag 的产出类型不同——目标地图产出 uint64（世界代理的 SN），玩家同步产出 bool，所以用模板
    T Value; 
};

class TeleportObject :public Component<TeleportObject>, public IAwakeFromPoolSystem<int, uint64>
{
public:
    void Awake(int worldId, uint64 playerSn) override;
    void BackToPool() override;

    int GetTargetWorldId() const;
    uint64 GetPlayerSN() const;

public:
    TeleportFlag<uint64> FlagWorld;     // 异步条件一：目标世界代理SN
    TeleportFlag<bool> FlagPlayerSync;  // 异步条件二：玩家数据是否同步完成

private:
    int _targetWorldId = 0; // 传送目的他 worldId
    uint64 _playerSn = 0;   // 哪个玩家在传送
};