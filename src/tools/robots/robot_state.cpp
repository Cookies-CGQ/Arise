#include "robot_state.h"
#include "libserver/packet.h"
#include "libserver/network_connector.h"
#include "robot.h"
#include "libserver/thread_mgr.h"

// 检测是否已断线
RobotStateType RobotState::Update()
{
    const auto state = GetState();
    // 如果当前状态已经过了连接阶段（> Connecting），说明之前 TCP 是连上的；并排除 RobotState_Game_Connecting（后续进游戏场景的连接状态，有单独处理）
    if (state > RobotState_Login_Connecting && state != RobotState_Game_Connecting)
    {
        // 如果断线了，直接返回 RobotState_Login_Connecting，状态机自动回退到最初的连接状态，触发重连
        if (!_pParentObj->IsConnected())
        {
            return RobotState_Login_Connecting;
        }
    }

    // 调用虚函数OnUpdate()，交给子类实现具体逻辑。这是模板方法模式——基类处理通用的断线检测，子类只需关心正常流程
    return OnUpdate();
}

void RobotState::EnterState()
{
    // 每进入一个状态，通知 robot mgr
    Proto::RobotSyncState protoState;
    auto pState = protoState.add_states();
    pState->set_account(_pParentObj->GetAccount());
    pState->set_state(GetState());

    // 只发送给主线程
    auto pPacket = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_RobotSyncState, 0);
    pPacket->SerializeToBuffer(protoState);
    ThreadMgr::GetInstance()->GetMessageSystem()->AddPacketToList(pPacket);
    
    // 非常规途径，手动打开Ref
    pPacket->OpenRef();

    OnEnterState();
}

void RobotState::LeaveState()
{
    OnLeaveState();
}