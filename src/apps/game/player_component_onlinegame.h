#pragma once
#include "libserver/system.h"
#include "libserver/component.h"
#include "libplayer/player_component.h"

// 挂载在 Player 实体上的组件，用于管理维护玩家在 Game 服务的在线状态，通过 Redis 实现跨进程可见的在线心跳机制，生命周期与Player对象绑定。
class PlayerComponentOnlineInGame: public Component<PlayerComponentOnlineInGame>,
    public PlayerComponent,
    virtual public IAwakeFromPoolSystem<std::string, int>, // 对象池初始化：登录时占位
    virtual public IAwakeFromPoolSystem<std::string>       // 对象池初始化：传送时接管
{
public:
    // 初始化：用于登录时占位
    void Awake(std::string account, int version) override;
    // 初始化：用于传送时接管
    void Awake(std::string account) override;

    void BackToPool() override;

    // 心跳机制 -- 每个3分钟发送消息维持redis状态
    void SetOnlineFlag() const;

    void ParserFromProto(const Proto::Player& proto) override;
    void SerializeToProto(Proto::Player* pProto) override;

private:
    std::string _account = "";   // 账号
    int _onlineVersion = 0;      // 在线版本号
    bool _isReadFromDB = true;  // 是否从DB读取
};