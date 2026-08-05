#pragma once

#include <thread>
#include <list>
#include "sn_object.h"
#include "system_manager.h"
#include "thread_type.h"

// 线程状态
enum class ThreadState
{
    Init,
    Run,
    Stop,
    Destroy,
};

// 主线程/工作线程
class Thread : public SystemManager, public SnObject
{
public:
    Thread(ThreadType threadType);
    ~Thread();

    // 启动线程
    void Start();
    // 销毁线程
    void DestroyThread();
    void Dispose() override;

    // 线程是否停止运行
    bool IsStop() const; 
    // 线程是否已经销毁   
    bool IsDestroy()const;

private:
    ThreadType _threadType; // 线程类型
    ThreadState _state;     // 线程状态
    std::thread _thread;    // 线程
};
