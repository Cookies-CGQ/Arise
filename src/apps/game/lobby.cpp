#include "lobby.h"
#include "libserver/message_system_help.h"
#include "libplayer/player_collector_component.h"
#include "libplayer/player_component_proto_list.h"
#include "libserver/message_system.h"
#include "world_proxy_component_gather.h"
#include "player_component_onlinegame.h"
#include "player_component_token.h"
#include "libplayer/player.h"
#include "libplayer/player_component_last_map.h"
#include "world_proxy_help.h"
#include "world_proxy_locator.h"
#include "libresource/resource_help.h"
#include "libserver/socket_locator.h"

void Lobby::Awake()
{
    // 获取大厅地图id
    auto pResMgr = ResourceHelp::GetResourceManager();
    _worldId = pResMgr->Worlds->GetRolesMap()->GetId();

    // 管理本进程所有在线玩家
    AddComponent<PlayerCollectorComponent>();
    // 定时发送同步本World信息
    AddComponent<WorldProxyComponentGather>();

    // 把自己注册进全局的世界定位器，方便worldId -> SN 映射
    auto pProxyLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<WorldProxyLocator>();
    pProxyLocator->RegisterToLocator(_worldId, GetSN());

    // 消息注册
    auto pMsgSystem = GetSystemManager()->GetMessageSystem();
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_NetworkDisconnect, BindFunP1(this, &Lobby::HandleNetworkDisconnect));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::C2G_LoginByToken, BindFunP1(this, &Lobby::HandleLoginByToken));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_GameTokenToRedisRs, BindFunP1(this, &Lobby::HandleGameTokenToRedisRs));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::G2DB_QueryPlayerRs, BindFunP1(this, &Lobby::HandleQueryPlayerRs));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::G2M_QueryWorldRs, BindFunP1(this, &Lobby::HandleQueryWorldRs));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_BroadcastCreateWorldProxy, BindFunP1(this, &Lobby::HandleBroadcastCreateWorldProxy));
    pMsgSystem->RegisterFunctionFilter<Player>(this, Proto::MsgId::MI_TeleportAfter, BindFunP1(this, &Lobby::GetPlayer), BindFunP2(this, &Lobby::HandleTeleportAfter));
}

void Lobby::BackToPool()
{
    _waitingForWorld.clear();
    _waitingForDungeon.clear();
}

Player* Lobby::GetPlayer(NetIdentify* pIdentify)
{
    auto pTagValue = pIdentify->GetTagKey()->GetTagValue(TagType::Player);
    if (pTagValue == nullptr)
        return nullptr;

    const auto playerSn = pTagValue->KeyInt64;

    auto pPlayerCollector = GetComponent<PlayerCollectorComponent>();
    return pPlayerCollector->GetPlayerBySn(playerSn);
}

void Lobby::HandleNetworkDisconnect(Packet* pPacket)
{
    auto pTagValue = pPacket->GetTagKey()->GetTagValue(TagType::Account);
    if (pTagValue == nullptr)
        return;

    GetComponent<PlayerCollectorComponent>()->RemovePlayerBySocket(pPacket->GetSocketKey()->Socket);
}

void Lobby::HandleLoginByToken(Packet* pPacket)
{
    auto pPlayerCollector = GetComponent<PlayerCollectorComponent>();
    auto proto = pPacket->ParseToProto<Proto::LoginByToken>();
    // 同账号已在本进程在线 -> 拒绝
    if (pPlayerCollector->GetPlayerByAccount(proto.account()) != nullptr)
    {
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_NetworkRequestDisconnect, pPacket);
        return;
    }
    
    // 添加Player
    auto pPlayer = pPlayerCollector->AddPlayer(pPacket, proto.account());
    if (pPlayer == nullptr)
    {
        // 添加失败，断开底层网络连接
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_NetworkRequestDisconnect, pPacket);
        return;
    }

    // Player添加组件，用于存储token
    pPlayer->AddComponent<PlayerComponentToken>(proto.token());
    // Player添加组件，用于在线管理状态
    pPlayer->AddComponent<PlayerComponentOnlineInGame>(pPlayer->GetAccount(), 1);

    // 发送消息到redisgame组件进行token验证
    Proto::GameTokenToRedis protoToken;
    protoToken.set_account(pPlayer->GetAccount().c_str());
    MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_GameTokenToRedis, protoToken, nullptr);

    // socket注册到SocketLocator（方便后续包路由到lobby）
    auto pSocketLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<SocketLocator>();
    pSocketLocator->RegisterToLocator(pPlayer->GetSocketKey()->Socket, GetSN());
}

void Lobby::HandleGameTokenToRedisRs(Packet* pPacket)
{
    auto protoRs = pPacket->ParseToProto<Proto::GameTokenToRedisRs>();
    auto pPlayer = GetComponent<PlayerCollectorComponent>()->GetPlayerByAccount(protoRs.account());
    if (pPlayer == nullptr)
    {
        LOG_ERROR("HandleGameRequestTokenToRedisRs. pPlayer == nullptr. account:" << protoRs.account().c_str());
        return;
    }

    Proto::LoginByTokenRs protoLoginGameRs;
    protoLoginGameRs.set_return_code(Proto::LoginByTokenRs::LGRC_TOKEN_WRONG);
    const auto pTokenComponent = pPlayer->GetComponent<PlayerComponentToken>();
    // 如果token有效
    if (pTokenComponent->IsTokenValid(protoRs.token_info().token()))
    {
        protoLoginGameRs.set_return_code(Proto::LoginByTokenRs::LGRC_OK);
    }

    // 给客户端发送token验证结果
    MessageSystemHelp::SendPacket(Proto::MsgId::C2G_LoginByTokenRs, protoLoginGameRs, pPlayer);

    if (protoLoginGameRs.return_code() != Proto::LoginByTokenRs::LGRC_OK)
        return;

    // LOG_DEBUG("enter game. account:" << pPlayer->GetAccount().c_str() << " token:" << protoRs.token_info().token().c_str());

    // 向数据库查询玩家数据	
    Proto::QueryPlayer protoQuery;
    protoQuery.set_player_sn(protoRs.token_info().player_sn());
    MessageSystemHelp::SendPacket(Proto::MsgId::G2DB_QueryPlayer, protoQuery, APP_DB_MGR);
}

void Lobby::HandleQueryPlayerRs(Packet* pPacket)
{
    auto protoRs = pPacket->ParseToProto<Proto::QueryPlayerRs>();
    auto account = protoRs.account();
    auto pPlayer = GetComponent<PlayerCollectorComponent>()->GetPlayerByAccount(account);
    if (pPlayer == nullptr)
    {
        LOG_ERROR("HandleQueryPlayer. pPlayer == nullptr. account:" << account.c_str());
        return;
    }

    // 向客户端发送玩家数据
    Proto::SyncPlayer syncPlayer;
    syncPlayer.mutable_player()->CopyFrom(protoRs.player());
    MessageSystemHelp::SendPacket(Proto::MsgId::G2C_SyncPlayer, syncPlayer, pPlayer);

    // 分析进入地图
    auto protoPlayer = protoRs.player();
    const auto playerSn = protoPlayer.sn();
    pPlayer->ParserFromProto(playerSn, protoPlayer);
    const auto pPlayerLastMap = pPlayer->AddComponent<PlayerComponentLastMap>();
    auto pWorldLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<WorldProxyLocator>();
    // 选路逻辑
    // 是否有上次副本记录
    auto pLastMap = pPlayerLastMap->GetLastDungeon();
    if (pLastMap != nullptr)
    {
        // 副本地图代理是否还存在
        if (pWorldLocator->IsExistDungeon(pLastMap->WorldSn))
        {
            // 存在，直接跳转
            WorldProxyHelp::Teleport(pPlayer, GetSN(), pLastMap->WorldSn);
            return;
        }

        if (_waitingForDungeon.find(pLastMap->WorldSn) == _waitingForDungeon.end())
        {
            _waitingForDungeon[pLastMap->WorldSn] = std::set<uint64>();
        }

        if (_waitingForDungeon[pLastMap->WorldSn].empty())
        {
            // 若集合原本为空才向 appmgr 发 G2M_QueryWorld 查询副本实例是否存在
            Proto::QueryWorld protoToMgr;
            protoToMgr.set_world_sn(pLastMap->WorldSn);
            protoToMgr.set_last_world_sn(GetSN());
            MessageSystemHelp::SendPacket(Proto::MsgId::G2M_QueryWorld, protoToMgr, APP_APPMGR);
        }

        _waitingForDungeon[pLastMap->WorldSn].insert(pPlayer->GetPlayerSN());
        return;
    }

    // 进入公共地图
    EnterPublicWorld(pPlayer);
}

void Lobby::EnterPublicWorld(Player* pPlayer)
{
    const auto pPlayerLastMap = pPlayer->GetComponent<PlayerComponentLastMap>();
    auto pWorldLocator = ComponentHelp::GetGlobalEntitySystem()->GetComponent<WorldProxyLocator>();

    // 公共地图代理是否存在
    auto pLastMap = pPlayerLastMap->GetLastPublicMap();
    const auto lastMapSn = pWorldLocator->GetWorldSnById(pLastMap->WorldId);
    if (lastMapSn != (uint64)INVALID_ID)
    {
        //LOG_DEBUG("teleport to public. world id:" << pLastMap->WorldId);
        // 直接跳转到公共地图
        WorldProxyHelp::Teleport(pPlayer, GetSN(), lastMapSn);
        return;
    }

    // 不存在公共地图代理
    if (_waitingForWorld.find(pLastMap->WorldId) == _waitingForWorld.end())
    {
        _waitingForWorld[pLastMap->WorldId] = std::set<uint64>();
    }

    if (_waitingForWorld[pLastMap->WorldId].empty())
    {
        // 若集合原本为空才向 appmgr 发 G2M_RequestWorld 申请创建该地图
        Proto::RequestWorld protoToMgr;
        protoToMgr.set_world_id(pLastMap->WorldId);
        MessageSystemHelp::SendPacket(Proto::MsgId::G2M_RequestWorld, protoToMgr, APP_APPMGR);
    }

    _waitingForWorld[pLastMap->WorldId].insert(pPlayer->GetPlayerSN());
}

void Lobby::HandleQueryWorldRs(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::QueryWorldRs>();
    const auto worldSn = proto.world_sn();

    const auto iter = _waitingForDungeon.find(worldSn);
    if (iter == _waitingForDungeon.end())
        return;

    // 这里直接return是因为appmgr会发送MI_BroadcastCreateWorldProxy 广播（各进程靠它同步 world proxy 信息），Lobby 在广播里才执行传送
    if (proto.return_code() == Proto::QueryWorldRs::QueryWorld_OK)
        return;

    // 到这个说明没有这个副本地图实例
    auto pPlayerMgr = GetComponent<PlayerCollectorComponent>();
    auto players = iter->second;
    for (auto one : players)
    {
        const auto pPlayer = pPlayerMgr->GetPlayerBySn(one);
        if (pPlayer == nullptr)
            continue;

        // 兜底操作，全部回去副本地图
        EnterPublicWorld(pPlayer);
    }
    // 情况等待缓冲区
    _waitingForDungeon.erase(iter);
}

void Lobby::HandleBroadcastCreateWorldProxy(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::BroadcastCreateWorldProxy>();
    const auto worldId = proto.world_id();
    const auto worldSn = proto.world_sn();

    auto pResMgr = ResourceHelp::GetResourceManager();
    auto pWorldRes = pResMgr->Worlds->GetResource(worldId);
    if (pWorldRes == nullptr)
    {
        LOG_ERROR("can't find resouces of world. world id:" << worldId);
        return;
    }

    // 如果是公共地图
    if (pWorldRes->IsType(ResourceWorldType::Public))
    {
        const auto iter = _waitingForWorld.find(worldId);
        if (iter == _waitingForWorld.end())
            return;

        //LOG_DEBUG("recv worldproxy create msg. map id:" << worldId << " world sn:" << worldSn);

        auto pPlayerMgr = GetComponent<PlayerCollectorComponent>();

        // 遍历等待缓冲区，全员跳转
        auto players = iter->second;
        for (auto one : players)
        {
            const auto player = pPlayerMgr->GetPlayerBySn(one);
            if (player == nullptr)
                continue;

            WorldProxyHelp::Teleport(player, GetSN(), worldSn);
        }

        _waitingForWorld.erase(iter);
    }
    // 如果是副本地图
    else
    {
        auto iter = _waitingForDungeon.find(worldSn);
        if (iter == _waitingForDungeon.end())
        {
            LOG_ERROR("can't find player. world id:" << worldId);
            return;
        }
        
        // 遍历等待缓冲区，全员跳转
        auto pPlayerMgr = GetComponent<PlayerCollectorComponent>();
        auto players = iter->second;
        for (auto one : players)
        {
            const auto player = pPlayerMgr->GetPlayerBySn(one);
            if (player == nullptr)
                continue;

            WorldProxyHelp::Teleport(player, GetSN(), worldSn);
        }

        _waitingForDungeon.erase(iter);
    }
}

void Lobby::HandleTeleportAfter(Player* pPlayer, Packet* pPacket)
{
    //LOG_DEBUG("teleport after. remove account:" << pPlayer->GetAccount().c_str());
    auto pPlayerMgr = GetComponent<PlayerCollectorComponent>();
    pPlayerMgr->RemovePlayerBySocket(pPlayer->GetSocketKey()->Socket);
}