#pragma once

#include "libserver/component.h"
#include "libserver/system.h"

// 玩家组件：负责管理登陆服玩家对象的账号相关状态
class PlayerComponentAccount :public Component<PlayerComponentAccount>, public IAwakeFromPoolSystem<std::string>
{
public:
    void Awake(std::string password) override;
    void BackToPool() override;

    // 获取密码
    std::string GetPassword() const;

    // 设置当前选中的角色
    void SetSelectPlayerSn(uint64 sn);
    // 获取选中的角色
    uint64 GetSelectPlayerSn() const;

    // 记录最后登录的游戏
    void SetLastGameId(int gameId);
    // 获取最后游戏ID
    int GetLastGameId() const;

private:
    std::string _password;         // 玩家账号密码
    uint64 _selectPlayerSn = 0;    // 当前选中的角色序列号
    int _lastGameId = 0;           // 最后登录的游戏ID
};