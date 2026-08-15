#include "world_proxy.h"
#include "world_proxy_locator.h"
#include "player_component_onlinegame.h"
#include "libserver/component_help.h"
#include "libserver/message_system_help.h"
#include "libserver/message_system.h"
#include "libplayer/player_collector_component.h"
#include "libplayer/player.h"
#include "libserver/log4.h"
#include "libserver/network_help.h"
#include "world_proxy_component_gather.h"
#include "world_component_teleport.h"
#include "libresource/resource_help.h"
#include "libserver/socket_locator.h"

void WorldProxy::Awake(int worldId, uint64 lastWorldSn)
{
    _worldId = worldId;
    _spaceAppId = Global::GetAppIdFromSN(_sn); // 这是_sn也是space服务的世界实例的sn，从sn可以反推space服务id

    AddComponent<PlayerCollectorComponent>();   // Player集合组件
    AddComponent<WorldProxyComponentGather>();  // 状态同步组件
    AddComponent<WorldComponentTeleport>();     // 地图跳转组件

    // 注册到locator
    auto pProxyLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<WorldProxyLocator>();
    pProxyLocator->RegisterToLocator(_worldId, GetSN());

    Proto::BroadcastCreateWorldProxy protoCreate;
    protoCreate.set_world_id(_worldId);
    protoCreate.set_world_sn(GetSN());

    // 如果这是为了某次传送而新建的副本世界proxy，定向发给旧世界proxy
    if (lastWorldSn > 0)
    {
        NetIdentify netIdentify;
        netIdentify.GetTagKey()->AddTag(TagType::Entity, lastWorldSn);
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_BroadcastCreateWorldProxy, protoCreate, &netIdentify);
    }
    // 否则全局广播
    else
    {
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_BroadcastCreateWorldProxy, protoCreate, nullptr);
    }

    // message
    auto pMsgSystem = GetSystemManager()->GetMessageSystem();
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_NetworkDisconnect, BindFunP1(this, &WorldProxy::HandleNetworkDisconnect));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_Teleport, BindFunP1(this, &WorldProxy::HandleTeleport));
    pMsgSystem->RegisterFunctionFilter<Player>(this, Proto::MsgId::MI_TeleportAfter, BindFunP1(this, &WorldProxy::GetPlayer), BindFunP2(this, &WorldProxy::HandleTeleportAfter));

    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_BroadcastCreateWorldProxy, BindFunP1(this, &WorldProxy::HandleBroadcastCreateWorldProxy));
    pMsgSystem->RegisterFunctionFilter<Player>(this, Proto::MsgId::S2G_SyncPlayer, BindFunP1(this, &WorldProxy::GetPlayer), BindFunP2(this, &WorldProxy::HandleS2GSyncPlayer));

    // 客户端发送来的协议
    pMsgSystem->RegisterFunctionFilter<Player>(this, Proto::MsgId::C2G_EnterWorld, BindFunP1(this, &WorldProxy::GetPlayer), BindFunP2(this, &WorldProxy::HandleC2GEnterWorld));

    // 默认协议处理函数
    pMsgSystem->RegisterDefaultFunction(this, BindFunP1(this, &WorldProxy::HandleDefaultFunction));
}

void WorldProxy::BackToPool()
{
}

void WorldProxy::SendPacketToWorld(const Proto::MsgId msgId, ::google::protobuf::Message& proto, Player* pPlayer) const
{
    // 包被发往 _spaceAppId 这个 space 进程，space 侧根据 Entity 标签找到对应的真实 World；
    // Player 标签让 World 知道这是哪个玩家的操作。
    // APP_ALLINONE 是单进程合并部署模式，进程内部也走消息系统，需要 ToWorld 标签辅助定位方向。
    TagKey tagKey;
    tagKey.AddTag(TagType::Player, pPlayer->GetPlayerSN()); // 玩家标签
    tagKey.AddTag(TagType::Entity, _sn); // 世界标签
    if (Global::GetInstance()->GetCurAppType() == APP_ALLINONE)
    {
        tagKey.AddTag(TagType::ToWorld, _sn); // 如果是单进程模式下：额外ToWorld标签区分“转给世界”的方向
    }
    MessageSystemHelp::SendPacket(msgId, proto, &tagKey, APP_SPACE, _spaceAppId);
}

void WorldProxy::SendPacketToWorld(const Proto::MsgId msgId, Player* pPlayer) const
{
    TagKey tagKey;
    tagKey.AddTag(TagType::Player, pPlayer->GetPlayerSN());
    tagKey.AddTag(TagType::Entity, _sn);
    if (Global::GetInstance()->GetCurAppType() == APP_ALLINONE)
    {
        tagKey.AddTag(TagType::ToWorld, _sn);
    }
    MessageSystemHelp::SendPacket(msgId, &tagKey, APP_SPACE, _spaceAppId);
}

void WorldProxy::CopyPacketToWorld(Player* pPlayer, Packet* pPacket) const
{
    // 和SendPacketToWorld类似
    auto pPacketCopy = MessageSystemHelp::CreatePacket((Proto::MsgId)pPacket->GetMsgId(), nullptr);
    pPacketCopy->CopyFrom(pPacket);
    auto pTagKey = pPacketCopy->GetTagKey();
    pTagKey->AddTag(TagType::Player, pPlayer->GetPlayerSN());
    pTagKey->AddTag(TagType::Entity, _sn);
    if (Global::GetInstance()->GetCurAppType() == APP_ALLINONE)
    {
        pTagKey->AddTag(TagType::ToWorld, _sn);
    }

    MessageSystemHelp::SendPacket(pPacketCopy, APP_SPACE, _spaceAppId);
}

Player* WorldProxy::GetPlayer(NetIdentify* pIdentify)
{
    // 两种来源：客户端原始包（Account 标签 → socket 查）和跨进程路由包（Player 标签 → SN 查）。
    // 查不到返回 nullptr，消息系统会跳过该处理函数。
    auto pTags = pIdentify->GetTagKey();
    const auto pTagAccount = pTags->GetTagValue(TagType::Account);
    if (pTagAccount != nullptr)
    {
        auto pPlayerMgr = this->GetComponent<PlayerCollectorComponent>();
        return pPlayerMgr->GetPlayerBySocket(pIdentify->GetSocketKey()->Socket);
    }

    const auto pTagPlayer = pTags->GetTagValue(TagType::Player);
    if (pTagPlayer != nullptr)
    {
        auto pPlayerMgr = this->GetComponent<PlayerCollectorComponent>();
        return pPlayerMgr->GetPlayerBySn(pTagPlayer->KeyInt64);
    }

    return nullptr;
}

void WorldProxy::HandleDefaultFunction(Packet* pPacket)
{
    // 没有显示注册的协议都落在这里，默认行为就是中转
    // 这意味着大量"玩家↔世界"的业务协议（移动、战斗、聊天……）在 WorldProxy 这里完全不需要注册，直接双向透传。
    // 只有代理自己要介入的协议（传送、断线、进世界、玩家同步）才显式注册
    auto pPlayerMgr = this->GetComponent<PlayerCollectorComponent>();
    Player* pPlayer = nullptr;
    const auto pTagKey = pPacket->GetTagKey();
    if (pTagKey == nullptr)
    {
        LOG_ERROR("world proxy recv msg. but no tag. msgId:" << Log4Help::GetMsgIdName(pPacket->GetMsgId()).c_str());
        return;
    }

    bool isToClient = false;
    const auto pTagPlayer = pTagKey->GetTagValue(TagType::Player);
    // 有Player标签 -> 说明是发给玩家的
    if (pTagPlayer != nullptr)
    {
        isToClient = true;
        pPlayer = pPlayerMgr->GetPlayerBySn(pTagPlayer->KeyInt64);
    }
    // 说明是玩家客户端发来的
    else
    {
        pPlayer = pPlayerMgr->GetPlayerBySocket(pPacket->GetSocketKey()->Socket);
    }

    // 有可能协议传来时，已经断线了
    if (pPlayer == nullptr)
        return;

    // 默认只作中转操作
    if (isToClient)
    {
        // 发送给客户端
        auto pPacketCopy = MessageSystemHelp::CreatePacket((Proto::MsgId)pPacket->GetMsgId(), pPlayer);
        pPacketCopy->CopyFrom(pPacket);
        MessageSystemHelp::SendPacket(pPacketCopy);
        //LOG_DEBUG("transfer msg to client. msgId:" << Log4Help::GetMsgIdName(pPacket->GetMsgId()).c_str());
    }
    else
    {
        // 发送给space
        CopyPacketToWorld(pPlayer, pPacket);
        //LOG_DEBUG("transfer msg to space. msgId:" << Log4Help::GetMsgIdName(pPacket->GetMsgId()).c_str())
    }
}

void WorldProxy::HandleNetworkDisconnect(Packet* pPacket)
{
    if (!NetworkHelp::IsTcp(pPacket->GetSocketKey()->NetType))
        return;

    //LOG_DEBUG("world proxy 1. disconnect." << pPacket);

    // 情况一：Account标签存在 -> 玩家连接断开
    TagValue* pTagValue = pPacket->GetTagKey()->GetTagValue(TagType::Account);
    if (pTagValue != nullptr)
    {
        const auto pPlayerCollector = GetComponent<PlayerCollectorComponent>();
        if (pPlayerCollector == nullptr)
            return;

        const auto pPlayer = pPlayerCollector->GetPlayerBySocket(pPacket->GetSocketKey()->Socket);
        if (pPlayer == nullptr)
            return;

        //LOG_DEBUG("world proxy 2. disconnect." << pPacket);

        auto pCollector = GetComponent<PlayerCollectorComponent>();
        pCollector->RemovePlayerBySocket(pPacket->GetSocketKey()->Socket);

        SendPacketToWorld(Proto::MsgId::MI_NetworkDisconnect, pPlayer);
    }
    // 情况二：App标签 -> 某个服务器进程断线
    else
    {
        // 可能是space, login, appmgr，dbmgr断线
        auto pTags = pPacket->GetTagKey();
        const auto pTagApp = pTags->GetTagValue(TagType::App);
        if (pTagApp == nullptr)
            return;

        const auto appKey = pTagApp->KeyInt64;
        const auto appType = GetTypeFromAppKey(appKey);
        const auto appId = GetIdFromAppKey(appKey);

        // 只关心所代理的真实世界所在space是否挂了
        if (appType != APP_SPACE || _spaceAppId != appId)
            return;

        //LOG_DEBUG("world proxy 2. disconnect." << pPacket);

        // 如果space进程断线了，说明可能真实世界没了，所以代理失去了存在的意义
        // 玩家需要全部断线
        auto pPlayerCollector = GetComponent<PlayerCollectorComponent>();
        pPlayerCollector->RemoveAllPlayerAndCloseConnect();

        // locator
        auto pWorldLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<WorldProxyLocator>();
        pWorldLocator->Remove(_worldId, GetSN());

        // worldproxy 需要销毁
        GetSystemManager()->GetEntitySystem()->RemoveComponent(this);
    }
}

void WorldProxy::HandleTeleport(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::Teleport>();
    const auto playerSn = proto.player_sn();

    // 1、把玩家加入到本代理的集合中（传进来的是玩家的完整数据）
    auto pCollector = GetComponent<PlayerCollectorComponent>();     
    auto pPlayer = pCollector->AddPlayer(pPacket, proto.account());
    if (pPlayer == nullptr)
    {
        LOG_ERROR("failed to teleport, account:" << proto.account().c_str());
        return;
    }

    pPlayer->ParserFromProto(playerSn, proto.player()); // 恢复玩家状态数据
    pPlayer->AddComponent<PlayerComponentOnlineInGame>(pPlayer->GetAccount()); // 标记

    //LOG_DEBUG("world proxy. recv teleport. map id:" << _worldId << " world sn:" << GetSN() << " account:" << pPlayer->GetAccount().c_str());

    // 2、把玩家数据同步给真实世界（让 World 真正创建/激活这个玩家）
    Proto::SyncPlayer protoSync;
    protoSync.set_account(proto.account().c_str());
    protoSync.mutable_player()->CopyFrom(proto.player());
    SendPacketToWorld(Proto::MsgId::G2S_SyncPlayer, protoSync, pPlayer);

    // 3、通知旧世界"传送成功，可以删掉我这边的玩家了"（定向发给旧世界 SN）
    Proto::TeleportAfter protoTeleportRs;
    protoTeleportRs.set_player_sn(pPlayer->GetPlayerSN());
    NetIdentify indentify;
    indentify.GetTagKey()->AddTag(TagType::Player, pPlayer->GetPlayerSN());
    indentify.GetTagKey()->AddTag(TagType::Entity, proto.last_world_sn());
    MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_TeleportAfter, protoTeleportRs, &indentify);

    // 4、注册 socket 路由：以后这个 socket 发来的包直接路由到本代理
    auto pSocketLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<SocketLocator>();
    pSocketLocator->RegisterToLocator(pPlayer->GetSocketKey()->Socket, GetSN());
}

void WorldProxy::HandleTeleportAfter(Player* pPlayer, Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::TeleportAfter>();
    const auto playerSn = proto.player_sn();

    // 从旧代理移除玩家
    auto pPlayerMgr = GetComponent<PlayerCollectorComponent>();
    pPlayerMgr->RemovePlayerBySocket(pPlayer->GetSocketKey()->Socket);

    // 通知旧真实世界：玩家已走
    Proto::RemovePlayer protoRs;
    protoRs.set_player_sn(playerSn);
    SendPacketToWorld(Proto::MsgId::G2S_RemovePlayer, protoRs, pPlayer);
}

void WorldProxy::HandleC2GEnterWorld(Player* pPlayer, Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::EnterWorld>();
    auto worldId = proto.world_id();
    const auto pResMgr = ResourceHelp::GetResourceManager();
    const auto pWorldRes = pResMgr->Worlds->GetResource(worldId);

    if (pWorldRes == nullptr)
        return;

    auto pTeleportComponent = this->GetComponent<WorldComponentTeleport>();
    if (pTeleportComponent->IsTeleporting(pPlayer)) // 正在传送，防重入
        return;

    // 发起传送
    GetComponent<WorldComponentTeleport>()->CreateTeleportObject(worldId, pPlayer);
}

void WorldProxy::HandleS2GSyncPlayer(Player* pPlayer, Packet* pPacket)
{
    // 方向 S2G（Space→Game）：真实世界把自己的玩家状态推回代理，代理再同步给视野内其他玩家。
    auto proto = pPacket->ParseToProto<Proto::SyncPlayer>();
    // 用世界的权威数据刷新代理侧玩家
    pPlayer->ParserFromProto(pPlayer->GetPlayerSN(), proto.player());
    // 广播给周围玩家
    GetComponent<WorldComponentTeleport>()->BroadcastSyncPlayer(pPlayer->GetPlayerSN());
}

void WorldProxy::HandleBroadcastCreateWorldProxy(Packet* pPacket)
{
    // 收到"某个新 WorldProxy 上线"的广播后，转交给传送组件记录。这样将来要传送到那个世界时，知道目标代理是谁（比如副本动态创建后的寻址）。
    auto proto = pPacket->ParseToProto<Proto::BroadcastCreateWorldProxy>();
    GetComponent<WorldComponentTeleport>()->HandleBroadcastCreateWorldProxy(proto.world_id(), proto.world_sn());
}