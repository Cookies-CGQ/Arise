#pragma once 

#include "message_list.h"
#include "sn_object.h"

class Thread;

class ThreadObject : public MessageList, public SnObject
{
public:
    // 初始化
    virtual bool Init() = 0;
    // 注册感兴趣的协议
    virtual void RegisterMsgFunction() = 0;
    // 帧函数，用于更新数据
    virtual void Update() = 0;
    // 设置所在线程
    void SetThread(Thread* pThread);
    // 获取所在线程
    Thread* GetThread() const;
    // 是否激活
    bool IsActive() const;
    // 释放自身资源
    void Dispose() override;
protected:
    // 是否激活
    bool _active {true};
    // Actor所在线程
    Thread* _pThread {nullptr};
};