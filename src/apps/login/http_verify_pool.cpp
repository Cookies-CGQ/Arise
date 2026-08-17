#include "http_verify_pool.h"
#include "libserver/log4_help.h"
#include "libserver/message_system_help.h"
#include "libserver/message_system.h"
#include "libserver/component_help.h"
#include "libserver/yaml.h"
#include "libserver/global.h"
#include "libserver/network_type.h"
#include "libserver/timer_component.h"
#include "libserver/thread_type.h"
#include "libserver/util_time.h"
#include "libserver/network_help.h"

#include <jsoncpp/json/reader.h>

// 池连接数（审批确定：N=8）
#define HTTP_VERIFY_POOL_SIZE 8
// 使用中的连接超过该毫秒数视为僵死，强制关闭（Account 侧有 5 秒超时兜底）
#define HTTP_VERIFY_INUSE_TIMEOUT_MS (15 * 1000)

void HttpVerifyPool::Awake()
{
    // 解析第三方验证地址
    auto pYaml = ComponentHelp::GetYaml();
    const auto pLoginConfig = dynamic_cast<LoginConfig*>(pYaml->GetConfig(APP_LOGIN));
    ParseUrlInfo info;
    if (!MessageSystemHelp::ParseUrl(pLoginConfig->UrlLogin, info))
    {
        LOG_ERROR("http verify pool. parse login url failed. url:" << pLoginConfig->UrlLogin.c_str());
        return;
    }

    _ip = info.Host;
    _port = info.Port;
    _method = info.Mothed;

    // 注册消息处理
    auto pMsgSystem = GetSystemManager()->GetMessageSystem();
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_HttpVerifyRequest, BindFunP1(this, &HttpVerifyPool::HandleVerifyRequest));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_NetworkConnected, BindFunP1(this, &HttpVerifyPool::HandleNetworkConnected));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_HttpOuterResponse, BindFunP1(this, &HttpVerifyPool::HandleHttpOuterResponse));
    pMsgSystem->RegisterFunction(this, Proto::MsgId::MI_NetworkDisconnect, BindFunP1(this, &HttpVerifyPool::HandleNetworkDisconnect));

    // 预建 8 条长连接
    for (int i = 0; i < HTTP_VERIFY_POOL_SIZE; i++)
    {
        TagValue tagValue{ "", 0 };
        MessageSystemHelp::CreateConnect(NetworkType::HttpConnector, TagType::None, tagValue, _ip, _port);
    }

    // 定时检查：补足连接 + 清理僵死连接
    AddTimer(0, 2, false, 2, BindFunP0(this, &HttpVerifyPool::CheckConnections));

    LOG_DEBUG("HttpVerifyPool awake. ip:" << _ip.c_str() << " port:" << _port << " method:" << _method.c_str() << " size:" << HTTP_VERIFY_POOL_SIZE);
}

void HttpVerifyPool::BackToPool()
{
    _poolKeys.clear();
    _free.clear();
    _inUseSince.clear();

    while (!_waitQueue.empty())
        _waitQueue.pop();
}

void HttpVerifyPool::HandleVerifyRequest(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::HttpVerifyRequest>();
    const auto account = proto.account();
    const auto password = proto.password();

    // 找空闲连接
    SOCKET freeSocket = INVALID_SOCKET;
    for (auto& pair : _free)
    {
        if (pair.second)
        {
            freeSocket = pair.first;
            break;
        }
    }

    if (freeSocket == INVALID_SOCKET)
    {
        // 连接全忙，排队等待
        _waitQueue.push(std::make_pair(account, password));
        return;
    }

    _free[freeSocket] = false;
    _inUseSince[freeSocket] = Global::GetInstance()->TimeTick;
    SendVerify(freeSocket, account, password);
}

void HttpVerifyPool::HandleNetworkConnected(Packet* pPacket)
{
    // 只处理无标签的 HttpConnector 连接（池连接）
    if (pPacket->GetSocketKey()->NetType != NetworkType::HttpConnector)
        return;

    const auto pTagAccount = pPacket->GetTagKey()->GetTagValue(TagType::Account);
    if (pTagAccount != nullptr)
        return;

    const auto socket = pPacket->GetSocketKey()->Socket;
    auto iter = _poolKeys.find(socket);
    if (iter == _poolKeys.end())
    {
        SocketKey key(socket, NetworkType::HttpConnector);
        key.CopyFrom(pPacket->GetSocketKey());
        _poolKeys.emplace(socket, key);
    }
    else
    {
        iter->second.CopyFrom(pPacket->GetSocketKey());
    }
    _free[socket] = true;

    // 连接就绪，处理排队请求
    ProcessQueue();
}

void HttpVerifyPool::HandleHttpOuterResponse(Packet* pPacket)
{
    // 旧流程（带标签）的响应不在这里处理
    const auto pTagAccount = pPacket->GetTagKey()->GetTagValue(TagType::Account);
    if (pTagAccount != nullptr)
        return;

    const auto socket = pPacket->GetSocketKey()->Socket;

    // 归还连接
    auto iterFree = _free.find(socket);
    if (iterFree != _free.end())
        iterFree->second = true;
    _inUseSince.erase(socket);

    // 继续处理排队请求
    ProcessQueue();

    // 解析响应体，按 account 重新路由给 Account
    auto protoHttp = pPacket->ParseToProto<Proto::Http>();
    const auto response = protoHttp.body();

    Json::Value value;
    {
        const Json::CharReaderBuilder readerBuilder;
        Json::CharReader* jsonReader = readerBuilder.newCharReader();
        JSONCPP_STRING errs;
        const bool ok = jsonReader->parse(response.data(), response.data() + response.size(), &value, &errs);
        delete jsonReader;
        if (!ok || !errs.empty())
        {
            LOG_ERROR("http verify pool. json parse failed. body:" << response.c_str());
            return;
        }
    }

    const auto account = value["account"].asString();
    if (account.empty())
    {
        LOG_ERROR("http verify pool. response without account. body:" << response.c_str());
        return;
    }

    // 构造带 Account 标签的响应包转发给 Account（保持其原有处理逻辑不变）
    NetIdentify identify;
    auto iterKey = _poolKeys.find(socket);
    if (iterKey != _poolKeys.end())
        identify.GetSocketKey()->CopyFrom(&iterKey->second);
    else
    {
        identify.GetSocketKey()->Socket = socket;
        identify.GetSocketKey()->NetType = NetworkType::HttpConnector;
    }
    identify.GetTagKey()->AddTag(TagType::Account, account);

    MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_HttpOuterResponse, protoHttp, &identify);
}

void HttpVerifyPool::HandleNetworkDisconnect(Packet* pPacket)
{
    if (pPacket->GetSocketKey()->NetType != NetworkType::HttpConnector)
        return;

    const auto socket = pPacket->GetSocketKey()->Socket;
    _poolKeys.erase(socket);
    _free.erase(socket);
    _inUseSince.erase(socket);
}

void HttpVerifyPool::CheckConnections()
{
    // 1、清理僵死连接（使用中超过阈值仍未响应）
    const auto now = Global::GetInstance()->TimeTick;
    for (auto iter = _inUseSince.begin(); iter != _inUseSince.end(); )
    {
        if (now - iter->second < HTTP_VERIFY_INUSE_TIMEOUT_MS)
        {
            ++iter;
            continue;
        }

        LOG_ERROR("http verify pool. in-use connection timeout. socket:" << iter->first << " force close.");

        auto iterKey = _poolKeys.find(iter->first);
        if (iterKey != _poolKeys.end())
        {
            NetIdentify identify;
            identify.GetSocketKey()->CopyFrom(&iterKey->second);
            MessageSystemHelp::DispatchPacket(Proto::MsgId::MI_NetworkRequestDisconnect, &identify);
        }

        iter = _inUseSince.erase(iter);
    }

    // 2、补足连接数
    int need = HTTP_VERIFY_POOL_SIZE - static_cast<int>(_poolKeys.size());
    for (int i = 0; i < need; i++)
    {
        TagValue tagValue{ "", 0 };
        MessageSystemHelp::CreateConnect(NetworkType::HttpConnector, TagType::None, tagValue, _ip, _port);
    }
}

void HttpVerifyPool::SendVerify(SOCKET socket, const std::string& account, const std::string& password)
{
    NetIdentify identify;
    auto iterKey = _poolKeys.find(socket);
    if (iterKey != _poolKeys.end())
        identify.GetSocketKey()->CopyFrom(&iterKey->second);
    else
    {
        identify.GetSocketKey()->Socket = socket;
        identify.GetSocketKey()->NetType = NetworkType::HttpConnector;
    }

    std::map<std::string, std::string> params;
    params["account"] = account;
    params["password"] = password;
    MessageSystemHelp::SendHttpRequest(&identify, _ip, _port, _method, &params);
}

void HttpVerifyPool::ProcessQueue()
{
    while (!_waitQueue.empty())
    {
        SOCKET freeSocket = INVALID_SOCKET;
        for (auto& pair : _free)
        {
            if (pair.second)
            {
                freeSocket = pair.first;
                break;
            }
        }

        if (freeSocket == INVALID_SOCKET)
            return;

        auto req = _waitQueue.front();
        _waitQueue.pop();
        _free[freeSocket] = false;
        _inUseSince[freeSocket] = Global::GetInstance()->TimeTick;
        SendVerify(freeSocket, req.first, req.second);
    }
}
