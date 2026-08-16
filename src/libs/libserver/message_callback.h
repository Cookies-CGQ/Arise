#pragma once

#include "packet.h"

// 采用策略模式，根据消息处理不同切换不同的策略，调用方只依赖接口，不关心具体实现
class IMessageCallBack : public Component<IMessageCallBack>
{
public:
    virtual ~IMessageCallBack() = default;
    virtual bool ProcessPacket(Packet* packet) = 0;
};

using MsgCallbackFun = std::function<void(Packet*)>;

// 策略一：不含过滤
class MessageCallBack :public IMessageCallBack, public IAwakeFromPoolSystem<MsgCallbackFun>
{
public:
    void Awake(MsgCallbackFun fun) override;
    void BackToPool() override;
    virtual bool ProcessPacket(Packet* pPacket) override;

private:
    MsgCallbackFun _handleFunction;
};

// 策略二：含过滤
template<class T>
class MessageCallBackFilter :public IMessageCallBack, public IAwakeFromPoolSystem<>
{
public:
    void Awake() override {}
    
    void BackToPool() override
    {
        HandleFunction = nullptr;
        GetFilterObj = nullptr;
    }

    virtual bool ProcessPacket(Packet* pPacket) override
    {
        auto pObj = GetFilterObj(pPacket);
        // 被过滤
        if (pObj == nullptr)
            return false;

#ifdef LOG_TRACE_COMPONENT_OPEN
        const google::protobuf::EnumDescriptor* descriptor = Proto::MsgId_descriptor();
        const auto name = descriptor->FindValueByNumber(pPacket->GetMsgId())->name();

        const auto traceMsg = std::string("process. ")
            .append(" sn:").append(std::to_string(pPacket->GetSN()))
            .append(" msgId:").append(name);
        ComponentHelp::GetTraceComponent()->Trace(TraceType::Packet, pPacket->GetSocketKey()->Socket, traceMsg);
#endif

        HandleFunction(pObj, pPacket);
        return true;
    }

    std::function<void(T*, Packet*)> HandleFunction = nullptr;       // 消息处理函数
    std::function<T*(NetIdentify*)> GetFilterObj = nullptr;    // 过滤器
};