#include <sstream>
#include "robot_mgr.h"
#include "libserver/common.h"
#include "libserver/global.h"
#include "libserver/yaml.h"
#include "libserver/entity_system.h"
#include "libserver/message_component.h"
#include "libserver/message_system_help.h"
#include "libserver/update_component.h"
#include "libserver/component_help.h"
#include "global_robots.h"

void RobotMgr::Awake()
{
    // update
    auto pUpdateComponent = AddComponent<UpdateComponent>();
    pUpdateComponent->UpdataFunction = BindFunP0(this, &RobotMgr::Update);

    // 注册消息回调：只关心 Robot 发来的状态同步
    auto pMsgCallBack = new MessageCallBackFunction();
    AddComponent<MessageComponent>(pMsgCallBack);
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_RobotSyncState, BindFunP1(this, &RobotMgr::HandleRobotState));

    // 连接 Login 服务器
    auto pYaml = ComponentHelp::GetYaml();
    const auto pLoginConfig = dynamic_cast<LoginConfig*>(pYaml->GetConfig(APP_LOGIN));
    this->Connect(pLoginConfig->Ip, pLoginConfig->Port);

    // 每2s打印一次状态统计
    AddTimer(0, 2, false, 0, BindFunP0(this, &RobotMgr::ShowInfo));
}

void RobotMgr::HandleRobotState(Packet* pPacket)
{
    Proto::RobotSyncState protoState = pPacket->ParseToProto<Proto::RobotSyncState>();

    // 发送消息，通知服务器进行压测
    if (_robots.empty() && protoState.states_size() > 0)
    {
        std::cout << "test begin" << std::endl;
        Packet* pPacketBegin = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_RobotTestBegin, GetSocket());
        SendPacket(pPacketBegin);
    }

    RobotStateType iType = RobotState_Space_EnterWorld; // 初始化为最后面的状态
    for (int index = 0; index < protoState.states_size(); index++)
    {
        auto proto = protoState.states(index);
        const auto account = proto.account();
        // 更新_robots
        _robots[account] = RobotStateType(proto.state());
        if (_robots[account] < iType)
        {
            iType = _robots[account];
        }
    }

    // _isChange为最小枚举值，表示进度最慢的状态
    _isChange = true;
    NofityServer(iType);
}

void RobotMgr::NofityServer(RobotStateType iType)
{
    // 如果未收齐所有人的状态，则不判断
    if (_robots.size() != GlobalRobots::GetInstance()->GetRobotsCount())
        return;

    // 用 find_if 检查是否有人比 iType 还慢。如果找不到（iter == end()），说明所有人都至少达到了 iType——向服务器发送 MI_RobotTestEnd，告知"所有人都通过了这个阶段"
    auto iter = std::find_if(_robots.begin(), _robots.end(), [&iType](auto pair){
            return pair.second < iType;
        }
    );

    if (iter == _robots.end())
    {
        std::cout << "test over " << GetRobotStateTypeShortName(iType) << std::endl;;
        Packet* pPacketEnd = MessageSystemHelp::CreatePacket(Proto::MsgId::MI_RobotTestEnd, GetSocket());
        Proto::RobotTestEnd protoEnd;
        protoEnd.set_state(iType);
        pPacketEnd->SerializeToBuffer(protoEnd);
        SendPacket(pPacketEnd);
    }
}

void RobotMgr::ShowInfo()
{
    if (!_isChange)
        return;

    _isChange = false;

    // 每个robot状态进行统计
    std::map<RobotStateType, int> statData;
    std::for_each(_robots.cbegin(), _robots.cend(), [&statData](auto one){
            auto state = one.second;
            if (statData.find(state) == statData.end())
            {
                statData[state] = 0;
            }

            ++statData[state];
        });

    std::stringstream show;
    auto curTime = timeutil::NowToString();
    show << "++++++++++++++++++++++++++++ " << std::endl << curTime.c_str() << std::endl;

    // 输出每个状态有多少robot
    for (RobotStateType rss = RobotState_HttpRequest; rss < RobotState_End; rss = static_cast<RobotStateType>(rss + 1))
    {
        if (statData.find(rss) == statData.end())
        {
            show << GetRobotStateTypeName(rss) << 0 << std::endl;
        }
        else
        {
            show << GetRobotStateTypeName(rss) << statData[rss] << std::endl;
        }
    }
    show << "++++++++++++++++++++++++++++" << std::endl;

    std::cout << show.str().c_str() << std::endl;
}
