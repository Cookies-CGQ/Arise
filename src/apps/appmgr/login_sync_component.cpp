#include "login_sync_component.h"
#include "libserver/message_component.h"
#include "libserver/message_system.h"

void LoginSyncComponent::Awake()
{
    // 初始化Json序列化器
    Json::StreamWriterBuilder jsonBuilder;
    _jsonWriter = jsonBuilder.newStreamWriter();

    // message
    auto pMsgCallBack = new MessageCallBackFunction();
    AddComponent<MessageComponent>(pMsgCallBack);

    // http请求处理
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_HttpRequestLogin, BindFunP1(this, &LoginSyncComponent::HandleHttpRequestLogin));
    // sync同步处理
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_AppInfoSync, BindFunP1(this, &LoginSyncComponent::AppInfoSyncHandle));
}

void LoginSyncComponent::BackToPool()
{
    delete _jsonWriter;
    _apps.clear();
}

void LoginSyncComponent::HandleHttpRequestLogin(Packet* pPacket)
{
    // 获取并构建响应消息
    Json::Value responseObj;
    AppInfo info;
    if (!GetOneApp(APP_LOGIN, info))
    {
        responseObj["returncode"] = (int)Proto::LoginHttpReturnCode::LHRC_TIMEOUT;
        responseObj["ip"] = "";
        responseObj["port"] = 0;
    }
    else
    {
        responseObj["returncode"] = (int)Proto::LoginHttpReturnCode::LHRC_OK;
        responseObj["ip"] = info.Ip;
        responseObj["port"] = info.Port;
    }

    std::stringstream jsonStream;
    _jsonWriter->write(responseObj, &jsonStream);

    // 发送HTTP响应
    MessageSystemHelp::SendHttpResponse(pPacket, jsonStream.str().c_str(), jsonStream.str().length());
}
