#include "world_operator_component.h"
#include "world.h"
#include "libserver/thread_mgr.h"
#include "libserver/message_system.h"
#include "libresource/resource_manager.h"
#include "libresource/resource_help.h"

void WorldOperatorComponent::Awake()
{
    auto pMsgSystem = GetSystemManager()->GetMessageSystem();
    pMsgSystem->RegisterFunction(this, Proto::MsgId::G2S_CreateWorld, BindFunP1(this, &WorldOperatorComponent::HandleCreateWorld));
}

void WorldOperatorComponent::BackToPool()
{

}

void WorldOperatorComponent::HandleCreateWorld(Packet* pPacket)
{
    auto protoWorld = pPacket->ParseToProto<Proto::CreateWorld>();
    int worldId = protoWorld.world_id();
    const int gameAppId = protoWorld.game_app_id();
    const uint64 lastWorldSn = protoWorld.last_world_sn();

    // 生成地图sn并创造地图
    auto worldSn = Global::GetInstance()->GenerateSN();
    ThreadMgr::GetInstance()->CreateComponentWithSn<World>(worldSn, worldId);

    //LOG_DEBUG("create world. map id:" << worldId << " world sn:" << newWorldSn);

    const auto pResMgr = ResourceHelp::GetResourceManager();
    const auto pWorldRes = pResMgr->Worlds->GetResource(worldId);
    if(pWorldRes == nullptr)
    {
        LOG_ERROR("GetResource error.");
        return;
    }
    if (pWorldRes->IsType(ResourceWorldType::Dungeon) && lastWorldSn == 0)
    {
        // 副本必须挂在传送链子上（玩家总是从某个世界传送进副本），所以lastWorldSn==0
        LOG_ERROR("create world error. dungeon is created. but requestWorldSn == 0");
    }

    Proto::BroadcastCreateWorld protoRs;
    protoRs.set_world_id(worldId);
    protoRs.set_world_sn(worldSn);
    protoRs.set_last_world_sn(lastWorldSn);

    if ((Global::GetInstance()->GetCurAppType() & APP_APPMGR) == 0)
    {
        // 本进程中不包括 AppMgr, 向AppMgr发送消息
        MessageSystemHelp::SendPacket(Proto::MsgId::MI_BroadcastCreateWorld, protoRs, APP_APPMGR);
    }

    // 本进程中不包括 AppGame
    if ((Global::GetInstance()->GetCurAppType() & APP_GAME) == 0)
    {
        if (gameAppId != 0)
        {
            // 向指定Game发送数据
            MessageSystemHelp::SendPacket(Proto::MsgId::MI_BroadcastCreateWorld, protoRs, APP_GAME, gameAppId);
        }
        else
        {
            // 向所有Game进程发送数据
            MessageSystemHelp::SendPacketToAllApp(Proto::MsgId::MI_BroadcastCreateWorld, protoRs, APP_GAME);
        }
    }

    if ((Global::GetInstance()->GetCurAppType() & APP_GAME) != 0 || (Global::GetInstance()->GetCurAppType() & APP_APPMGR) != 0)
    {
        // 本进程中包括了AppGame AppMgr其中一个，需要中转消息
        MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_BroadcastCreateWorld, protoRs, nullptr);
    }
}