#include "account.h"
#include "libserver/common.h"
#include "libserver/packet.h"
#include "libserver/thread_mgr.h"
#include "http_request_account.h"

bool Account::Init()
{
    return true;
}

void Account::RegisterMsgFunction()
{
    // 创建普通版回调器并注册进去
    auto pMsgCallBack = new MessageCallBackFunction();
    AttachCallBackHandler(pMsgCallBack);
    // 注册兴趣协议的回调
    pMsgCallBack->RegisterFunction(Proto::MsgId::C2L_AccountCheck, BindFunP1(this, &Account::HandleAccountCheck));
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_AccountCheckToHttpRs, BindFunP1(this, &Account::HandleAccountCheckToHttpRs));
}

void Account::Update()
{

}

void Account::HandleAccountCheck(Packet* pPacket)
{
    auto protoCheck = pPacket->ParseToProto<Proto::AccountCheck>();
    const auto socket = pPacket->GetSocket();

    // 相同账号正在登录
    auto pPlayer = _playerMgr.GetPlayer(protoCheck.account());
    if(pPlayer != nullptr)
    {
        Proto::AccountCheckRs protoResult;
        protoResult.set_return_code(Proto::AccountCheckRs::ARC_LOGGING);

        auto pRsPacket = new Packet(Proto::MsgId::C2L_AccountCheckRs, socket);
        pRsPacket->SerializeToBuffer(protoResult);
        SendPacket(pRsPacket);

        // 关闭网络
        const auto pPacketDis = new Packet(Proto::MsgId::MI_NetworkDisconnectToNet, socket);
        DispatchPacket(pPacketDis);

        return;
    }
    // 正在验证该账号
    _playerMgr.AddPlayer(socket, protoCheck.account(), protoCheck.password());
    // 异步第三方验证账号
    HttpRequestAccount* pHttp = new HttpRequestAccount(protoCheck.account(), protoCheck.password());
    ThreadMgr::GetInstance()->AddObjToThread(pHttp);
}

void Account::HandleAccountCheckToHttpRs(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::AccountCheckToHttpRs>();
    auto pPlayer = _playerMgr.GetPlayer(proto.account());
    if(pPlayer == nullptr)
    {
        std::cout << "no find player, account: " << proto.account().c_str() << std::endl;
        return;
    }
    // 账号验证成功
    Proto::AccountCheckRs protoResult;
    protoResult.set_return_code(proto.return_code());
    auto pResultPacket = new Packet(Proto::MsgId::C2L_AccountCheckRs, pPlayer->GetSocket());
    pResultPacket->SerializeToBuffer(protoResult);
    SendPacket(pResultPacket);
}