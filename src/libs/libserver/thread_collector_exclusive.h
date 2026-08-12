#pragma once

#include "thread_collector.h"

class ThreadCollectorExclusive :public ThreadCollector
{
public:
    ThreadCollectorExclusive(ThreadType threadType, int initNum);

    // 消息分发
    virtual void HandlerMessage(Packet* pPacket) override;
};