#include "console_app_component.h"
#include "login_sync_component.h"
#include "libserver/message_component.h"

void ConsoleAppComponent::Awake()
{
    auto pMsgCallBack = new MessageCallBackFunction();
    AddComponent<MessageComponent>(pMsgCallBack);
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_CmdApp, BindFunP1(this, &ConsoleAppComponent::HandleCmdApp));
}

void ConsoleAppComponent::BackToPool()
{
}

void ConsoleAppComponent::HandleCmdApp(Packet* pPacket)
{
    auto cmdProto = pPacket->ParseToProto<Proto::CmdApp>();
    auto cmdType = cmdProto.cmd_type();
    if (cmdType == Proto::CmdApp_CmdType_Info)
    {
        HandleCmdAppInfo();
    }
}

void ConsoleAppComponent::HandleCmdAppInfo()
{
    // SyncComponent只存在于一个logic线程，而每个logic线程都有ConsoleAppComponent组件，所以需要判空
    auto pSyncEntity = GetSystemManager()->GetEntitySystem()->GetComponent<SyncComponent>();
    // SyncComponent与ConsoleAppComponent组件不在同一个线程，直接return，对这条广播的MI_CmdApp消息不处理
    if (pSyncEntity == nullptr)
        return;

    // SyncComponent与ConsoleAppComponent组件在同一个线程，输出SyncComponent相关的信息
    pSyncEntity->CmdShow();
}