#pragma once

#include <map>
#include <mutex>
#include <functional>
#include <list>
#include "common.h"
#include "packet.h"

class IMessageCallBackFunction
{
public:
    virtual ~IMessageCallBackFunction() = default;
    // 是否对该消息类型感兴趣
    virtual bool IsFollowMsgId(Packet* packet) = 0;
    // 如果感兴趣，执行该消息的处理方法
    virtual void ProcessPacket(Packet* Packet) = 0;
};

class MessageCallBackFunction: public IMessageCallBackFunction
{
public:
    using HandleFunction = std::function<void(Packet*)>;
    // 注册感兴趣的消息类型和对应处理函数
    void RegisterFunction(int msgId, HandleFunction function);
    // 是否对该消息类型感兴趣
    bool IsFollowMsgId(Packet* packet) override;
    // 如果感兴趣，执行该消息的处理方法
    void ProcessPacket(Packet* packet) override;

    std::map<int, HandleFunction>& GetCallBackHandler()
    {
        return _callbackHandle;
    }

protected:
    std::map<int, HandleFunction> _callbackHandle; // MsgId: HandleFunction
};

template<class T>
class MessageCallBackFunctionFilterObj: public MessageCallBackFunction
{
public:
    using HandleFunctionWithObj = std::function<void(T*, Packet*)>;
    using HandleGetObject = std::function<T*(NetworkIdentify*)>;

    // 注册感兴趣的消息类型和对应处理函数（指定对象版）
    void RegisterFunctionWithObj(int msgId, HandleFunctionWithObj function);
    // 是否对该消息类型感兴趣
    bool IsFollowMsgId(Packet* packet) override;
    // 如果感兴趣，执行该消息的处理方法
    void ProcessPacket(Packet* packet) override;
    
    HandleGetObject GetPacketObject = nullptr;

private:
    std::map<int, HandleFunctionWithObj> _callbackHandleWithObj;
};

template <class T>
void MessageCallBackFunctionFilterObj<T>::RegisterFunctionWithObj(const int msgId, HandleFunctionWithObj function)
{
    _callbackHandleWithObj[msgId] = function;
}

template<class T>
bool MessageCallBackFunctionFilterObj<T>::IsFollowMsgId(Packet* packet)
{
    if(MessageCallBackFunction::IsFollowMsgId(packet))
        return true;
    
    if(_callbackHandleWithObj.find(packet->GetMsgId()) != _callbackHandleWithObj.end())
    {
        if(GetPacketObject != nullptr)
        {
            T* pObj = GetPacketObject(packet);
            if(pObj != nullptr)
                return true;
        }
    }
    return false;
}

template<class T>
void MessageCallBackFunctionFilterObj<T>::ProcessPacket(Packet* packet)
{
    const auto handleIter = _callbackHandle.find(packet->GetMsgId());
    if(handleIter != _callbackHandle.end())
    {
        handleIter->second(packet);
        return;
    }
    
    auto iter = _callbackHandleWithObj.find(packet->GetMsgId());
    if(iter != _callbackHandleWithObj.end())
    {
        if(GetPacketObject != nullptr)
        {
            T* pObj = GetPacketObject(packet);
            if(pObj != nullptr)
            {
                iter->second(pObj, packet);
            }
        }
    }
}