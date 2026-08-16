#pragma once

#include "libserver/component.h"
#include "libserver/system.h"
#include "libplayer/player_component.h"

// 挂在 Player 实体上的一个数据切片组件，只负责从玩家数据中取出一小块--性别--供地图广播使用
class PlayerComponentDetail :public Component<PlayerComponentDetail>, public IAwakeFromPoolSystem<>, public PlayerComponent
{
public:
    void Awake() override;
    void BackToPool() override;

    // 实现PlayerComponent接口
    void ParserFromProto(const Proto::Player& proto) override;
    void SerializeToProto(Proto::Player* pProto) override;

    // 对外的业务访问器 -- 获取玩家角色性别
    Proto::Gender GetGender() const;

private:
    Proto::Gender _gender;
};