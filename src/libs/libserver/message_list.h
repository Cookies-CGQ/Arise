#pragma once
#include <mutex>
#include <map>
#include <list>
#include <functional>

#include "common.h"
#include "packet.h"

// 消息队列基类--消息队列只进行维护感兴趣的协议和协议筛选，不存储消息，存储消息的方式已经转换到线程中
class MessageCallBackFunctionInfo
{
public:
    virtual ~MessageCallBackFunctionInfo() = default;
    // 判断协议是否感兴趣
    virtual bool IsFollowMsgId(Packet* packet) = 0;
    // 处理协议
    virtual void ProcessPacket(Packet* packet) = 0;
};

// 普通版消息回调器，用于接收广播协议
class MessageCallBackFunction :public MessageCallBackFunctionInfo
{
public:
    using HandleFunction = std::function<void(Packet*)>;
    // 注册感兴趣的广播协议
    void RegisterFunction(int msgId, HandleFunction function);
    // 是否对这个广播协议感兴趣
    bool IsFollowMsgId(Packet* packet) override;
    // 处理协议
    void ProcessPacket(Packet* packet) override;

    std::map<int, HandleFunction>& GetCallBackHandler() { return _callbackHandle; }

protected:
    // 感兴趣的广播协议
    std::map<int, HandleFunction> _callbackHandle;
};

// 普通版plus，带对象过滤的版本消息回调器，用于接收指定协议和广播协议
template<class T>
class MessageCallBackFunctionFilterObj :public MessageCallBackFunction
{
public:
    using HandleFunctionWithObj = std::function<void(T*, Packet*)>;
    using HandleGetObject = std::function<T*(SOCKET)>;
    // 注册感兴趣的指定协议
    void RegisterFunctionWithObj(int msgId, HandleFunctionWithObj function);
    // 是否对这个指定协议感兴趣
    bool IsFollowMsgId(Packet* packet) override;
    // 处理协议
    void ProcessPacket(Packet* packet) override;

    // 用于判断
    HandleGetObject GetPacketObject{ nullptr };

private:
    // 感兴趣的指定协议
    std::map<int, HandleFunctionWithObj> _callbackHandleWithObj;
};

template <class T>
void MessageCallBackFunctionFilterObj<T>::RegisterFunctionWithObj(const int msgId, HandleFunctionWithObj function)
{
    _callbackHandleWithObj[msgId] = function;
}

template <class T>
bool MessageCallBackFunctionFilterObj<T>::IsFollowMsgId(Packet* packet)
{
    // 检查广播协议回调表
    if (MessageCallBackFunction::IsFollowMsgId(packet))
        return true;

    // 检查指定协议回调表
    if (_callbackHandleWithObj.find(packet->GetMsgId()) != _callbackHandleWithObj.end())
    {
        // 进行过滤
        if (GetPacketObject != nullptr)
        {
            T* pObj = GetPacketObject(packet->GetSocket());
            if (pObj != nullptr)
                return true;
        }
    }

    return false;
}

template <class T>
void MessageCallBackFunctionFilterObj<T>::ProcessPacket(Packet* packet)
{
    // 是否是广播协议
    const auto handleIter = _callbackHandle.find(packet->GetMsgId());
    if (handleIter != _callbackHandle.end())
    {
        handleIter->second(packet);
        return;
    }
    // 是否是指定协议
    auto iter = _callbackHandleWithObj.find(packet->GetMsgId());
    if (iter != _callbackHandleWithObj.end())
    {
        if (GetPacketObject != nullptr)
        {
            T* pObj = GetPacketObject(packet->GetSocket());
            if (pObj != nullptr)
            {
                iter->second(pObj, packet);
            }
        }
    }
}

class MessageList :public IDisposable
{
public:
    // 释放自身资源
    void Dispose() override;
    // 注册回调器
    void AttachCallBackHandler(MessageCallBackFunctionInfo* pCallback);
    // 判断协议是否感兴趣
    bool IsFollowMsgId(Packet* packet) const;
    // 处理协议
    void ProcessPacket(Packet* packet) const;
    // 发送协议到其他Actor
    static void DispatchPacket(Packet* pPacket);
    // 发送packet
    static void SendPacket(Packet* pPacket);

protected:
    MessageCallBackFunctionInfo* _pCallBackFuns{ nullptr };
};
