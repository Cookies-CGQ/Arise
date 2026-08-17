#include "robot_state.h"
#include "libserver/packet.h"
#include "robot.h"
#include "libserver/thread_mgr.h"
#include "libserver/message_system.h"
#include "libserver/component_help.h"

// 检测是否已断线
RobotStateType RobotState::Update()
{
    const auto state = GetState();
    if (state > RobotStateType::Login_Connecting&& state != RobotStateType::Game_Connecting)
    {
        const auto socketKey = _pParentObj->GetSocketKey();
        if (socketKey->Socket == INVALID_SOCKET)
        {
            // 登录连接断开：延迟重试，避免"同账号/在线拒绝"导致的重连风暴
            // （拒绝响应可能因连接已关闭而丢失，退避必须放在断线翻转处才能确保生效）
            _pParentObj->SetLoginRetryDelay(3000);
            return RobotStateType::Login_Connecting;
        }
    }

    return OnUpdate();
}

void RobotState::EnterState()
{
    // 每进入一个状态，通知 robot mgr
    Proto::RobotSyncState protoState;
    auto pState = protoState.add_states();
    pState->set_account(_pParentObj->GetAccount());
    pState->set_state((int)GetState());

#ifdef LOG_TRACE_COMPONENT_OPEN
    if (_pParentObj->GetSocketKey()->Socket != INVALID_SOCKET) {
        std::stringstream os;
        os << "enter state:" << GetRobotStateTypeShortName(GetState());
        ComponentHelp::GetTraceComponent()->Trace(TraceType::Player, _pParentObj->GetSocketKey()->Socket, os.str());
    }
#endif

    // 只发送给主线程
    auto pPacket = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_RobotSyncState, nullptr);
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