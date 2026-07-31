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
    // 消息处理函数
    void HandleCmdThread(Packet* pPacket);
    void HandleCmdThreadEntites(Packet* pPacket);
    void HandleCmdThreadPool(Packet* pPacket);
    void HandleCmdThreadConnect(Packet* pPacket);

private:
    ThreadType _threadType; // 线程类型

    static std::mutex _show_lock; // 进程锁--用于输出打印
};
