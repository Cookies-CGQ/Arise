#pragma once

#include "system.h"
#include "entity.h"
#include "thread_type.h"

class Packet;

// 每个线程都会创建一个ConsoleThreadComponent用于输出线程相关信息
class ConsoleThreadComponent :public Entity<ConsoleThreadComponent>, public IAwakeSystem<ThreadType>
{
public:
    // 初始化
    void Awake(ThreadType iType);
    void BackToPool();

private:
    // 消息处理函数 -- 接收消息统一处理
    void HandleCmdThread(Packet* pPacket);
    // 消息处理函数 -- 打印线程实体信息
    void HandleCmdThreadEntites(Packet* pPacket);
    // 消息处理函数 -- 打印线程对象池信息
    void HandleCmdThreadPool(Packet* pPacket);
    // 消息处理函数 -- 打印线程连接信息
    void HandleCmdThreadConnect(Packet* pPacket);

private:
    ThreadType _threadType; // 线程类型

    static std::mutex _show_lock; // 进程锁--用于输出打印
};
