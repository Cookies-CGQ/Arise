#pragma once

#include <map>
#include <list>
#include <functional>
#include "common.h"
#include "system.h"
#include "cache_swap.h"
#include "socket_object.h"
#include "component_help.h"
#include "packet.h"
#include "message_callback.h"

class IComponent;
class SystemManager;
class Packet;
class EntitySystem;

class MessageSystem :virtual public ISystem<MessageSystem>
{
public:
    MessageSystem(SystemManager* pMgr);
    void Dispose() override;

    // 消息处理
    void Update(EntitySystem* pEntities) override;
    // 添加packet到本线程
    void AddPacketToList(Packet* pPacket);

    // 注册消息处理函数
    void RegisterFunction(IEntity* obj, int msgId, MsgCallbackFun cbfun);
    // 注册默认消息处理函数
    void RegisterDefaultFunction(IEntity* obj, MsgCallbackFun cbfun);
    // 移除消息处理函数
    void RemoveFunction(IComponent* obj);
    
    // 注册消息处理函数（过滤版）
    template<typename T>
    void RegisterFunctionFilter(IEntity* obj, int msgId, std::function<T*(NetIdentify*)> getObj, std::function<void(T*, Packet*)> fun);

private:
    // 本线程中的所有待处理包
    std::mutex _packet_lock;
    CacheSwap<Packet> _cachePackets; 

    SystemManager* _systemMgr = nullptr;

    // 消息类型 : ObjSN : 消息处理函数，本线程的消息处理注册信息由这里管理
    std::map<int, std::map<uint64, IMessageCallBack*>*> _callbacks;
    // 默认处理函数，ObjSN : 默认消息处理函数
    std::map<uint64, IMessageCallBack*> _defaultCallbacks;
};

template <typename T>
void MessageSystem::RegisterFunctionFilter(IEntity* obj, int msgId, std::function<T*(NetIdentify*)> getObj, std::function<void(T*, Packet*)> fun)
{
    auto iter = _callbacks.find(msgId);
    if (iter == _callbacks.end())
    {
        _callbacks.insert(std::make_pair(msgId, new std::map<uint64, IMessageCallBack*>()));
    }

    auto pCallback = _systemMgr->GetEntitySystem()->AddComponent<MessageCallBackFilter<T>>();
    pCallback->GetFilterObj = std::move(getObj);
    pCallback->HandleFunction = std::move(fun);
    pCallback->SetParent(obj);
    _callbacks[msgId]->insert(std::make_pair(obj->GetSN(), pCallback));
}