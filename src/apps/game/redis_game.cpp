#include "redis_game.h"
#include "libserver/redis_constants.h"
#include "libserver/message_system_help.h"
#include "libserver/message_system.h"

void RedisGame::RegisterMsgFunction()
{
    auto pMsgSystem = GetSystemManager()->GetMessageSystem();

    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_PlayerSyncOnlineToRedis, BindFunP1(this, &RedisGame::HandlePlayerSyncOnlineToRedis));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_PlayerDeleteOnlineToRedis, BindFunP1(this, &RedisGame::HandlePlayerDeleteOnlineToRedis));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_GameTokenToRedis, BindFunP1(this, &RedisGame::HandleGameTokenToRedis));
}

void RedisGame::HandlePlayerSyncOnlineToRedis(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::PlayerSyncOnlineToRedis>();
    auto curValue = proto.version();

    const std::string key = RedisKeyAccountOnlineGame + proto.account();
    const auto onlineVersion = this->GetInt(key); // 读取redis中已有的版本号

    // 如果版本过低，则丢弃。防止乱序消息导致旧状态覆盖新状态（例如下线消息先到、上线信息后到的情况）
    if (curValue < onlineVersion)
        return;

    Setex(key, curValue, RedisKeyAccountOnlineGameTimeout);
    // 完成login key -> game key的交接，此时才消费一次性token
    Delete(RedisKeyAccountTokey + proto.account());
}

void RedisGame::HandlePlayerDeleteOnlineToRedis(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::PlayerDeleteOnlineToRedis>();
    auto curValue = proto.version();

    const std::string key = RedisKeyAccountOnlineGame + proto.account();
    const auto onlineVersion = this->GetInt(key);

    // 如果版本过低，则丢弃。防止旧的下线消息误删新上线的记录
    if (curValue < onlineVersion)
        return;

    Delete(key);
}

void RedisGame::HandleGameTokenToRedis(Packet* pPacket)
{
    auto protoToken = pPacket->ParseToProto<Proto::GameTokenToRedis>();

    Proto::GameTokenToRedisRs protoRs;
    protoRs.set_account(protoToken.account().c_str());

    const std::string tokenValue = GetString(RedisKeyAccountTokey + protoToken.account());
    protoRs.mutable_token_info()->ParseFromString(tokenValue);

    // token延后删除，使token存在周期覆盖login key -> game key的交接窗口
    // Delete(RedisKeyAccountTokey + protoToken.account());

    // 发送消息，后续进行token验证
    MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_GameTokenToRedisRs, protoRs, nullptr);
}
