#pragma once

#include <mutex>
#include <vector>
#include "common.h"
#include "thread.h"
#include "cache_swap.h"
#include "singleton.h"
#include "entity_system.h"
#include "component_factory.h"
#include "regist_to_factory.h"
#include "message_system_help.h"
#include "thread_collector.h"
#include "thread_type.h"

// 线程管理 -- 主线程
class ThreadMgr :public Singleton<ThreadMgr>, public SystemManager
{
public:
    ThreadMgr();
    // 读取该服务的配置文件并创建线程
    void InitializeThread();
    // 创建线程
    void CreateThread(ThreadType iType, int num);
	
    // 初始化全局组件
    void InitializeGlobalComponent(APP_TYPE ppType, int appId);
    // update
    void Update() override;
    // 分发创建组件消息（单播）
    void UpdateCreatePacket();
    // 分发普通消息（广播）
    void UpdateDispatchPacket();

    // 工作线程是否全部停止
    bool IsStopAll();
    // 销毁所有工作线程
    void DestroyThread();
    // 是否销毁所有工作线程
    bool IsDestroyAll();
    // 释放资源
    void Dispose() override;

    // 创建组件
    template<class T, typename ...TArgs>
    void CreateComponent(TArgs... args);
    template<class T, typename ...TArgs>
    void CreateComponent(ThreadType iType, TArgs... args);

    // 添加一条消息（待分发）
    void DispatchPacket(Packet* pPacket);

private:
    // 给Proto::CreateComponent添加参数
    template <typename...Args>
    void AnalyseParam(Proto::CreateComponent& proto, int value, Args...args);
    template <typename...Args>
    void AnalyseParam(Proto::CreateComponent& proto, std::string value, Args...args);
    void AnalyseParam(Proto::CreateComponent& proto)
    {

    }

private:
    std::map<ThreadType, ThreadCollector*> _threads; // 线程类型：同一线程类型的线程集合

    // 创建组件消息
    std::mutex _create_lock;
    CacheSwap<Packet> _createPackets;

    // 其他消息消息
    std::mutex _packet_lock;
    CacheSwap<Packet> _packets;
};

template<class T, typename ...TArgs>
void ThreadMgr::CreateComponent(TArgs ...args)
{
    // 没有指定线程类型默认就是安排在逻辑线程
    CreateComponent<T>(LogicThread, std::forward<TArgs>(args)...);
}

template<class T, typename ...TArgs>
void ThreadMgr::CreateComponent(ThreadType iType, TArgs ...args)
{
    std::lock_guard<std::mutex> guard(_create_lock);

    // 如果还没注册就进行注册
    const std::string className = typeid(T).name();
    if (!ComponentFactory<TArgs...>::GetInstance()->IsRegisted(className))
    {
        RegistToFactory<T, TArgs...>();
    }

    // 创建组件packet
    Proto::CreateComponent proto;
    proto.set_thread_type((int)iType);
    proto.set_class_name(className.c_str());
    AnalyseParam(proto, std::forward<TArgs>(args)...);

    auto pCreatePacket = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_CreateComponent, 0);
    pCreatePacket->SerializeToBuffer(proto);
    _createPackets.GetWriterCache()->emplace_back(pCreatePacket);
}

template<typename ... Args>
void ThreadMgr::AnalyseParam(Proto::CreateComponent& proto, int value, Args... args)
{
    auto pProtoParam = proto.mutable_params()->Add();
    pProtoParam->set_type(Proto::CreateComponentParam::Int);
    pProtoParam->set_int_param(value);
    AnalyseParam(proto, std::forward<Args>(args)...);
}

template<typename ... Args>
void ThreadMgr::AnalyseParam(Proto::CreateComponent& proto, std::string value, Args... args)
{
    auto pProtoParam = proto.mutable_params()->Add();
    pProtoParam->set_type(Proto::CreateComponentParam::String);
    pProtoParam->set_string_param(value.c_str());
    AnalyseParam(proto, std::forward<Args>(args)...);
}
