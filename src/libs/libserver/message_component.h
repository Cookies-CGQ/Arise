#pragma once

#include "component.h"
#include "message_callback.h"
#include "system.h"

class MessageComponent : public Component<MessageComponent>, public IAwakeFromPoolSystem<IMessageCallBackFunction*>
{
public:
    ~MessageComponent();
    // 初始化
    void Awake(IMessageCallBackFunction* pCallback) override;
    // 归还对象池
    void BackToPool() override;

    // 消息类型是否感兴趣
    bool IsFollowMsgId(Packet* packet) const;
    // 消息处理
    void ProcessPacket(Packet* packet) const;

protected:
    IMessageCallBackFunction* _pCallBackFuns = nullptr;
};