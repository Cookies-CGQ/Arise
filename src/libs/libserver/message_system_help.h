#pragma once

#include "common.h"
#include "app_type.h"
#include "packet.h"

enum class NetworkType;
class INetwork;
struct http_message;

struct ParseUrlInfo
{
    std::string Host;
    int Port;
    std::string Mothed;
    std::string Params;
};

class MessageSystemHelp
{
public:
    // 创建一个packet
    static Packet* CreatePacket(Proto::MsgId msgId, NetworkIdentify* pIdentify);

    // 创建并分发一个packet（向内分发）
    static void DispatchPacket(const Proto::MsgId msgId, NetworkIdentify* pIdentify);
    static void DispatchPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, NetworkIdentify* pIdentify);

    // 创建并发送一个packet（向外分发 -- 走网络出去）
    static void SendPacket(const Proto::MsgId msgId, NetworkIdentify* pIdentify, google::protobuf::Message& proto);
    static void SendPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, APP_TYPE appType, int appId = 0);

    // http
    // 发送请求
    static void SendHttpRequest(NetworkIdentify* pIdentify, std::string ip, int port, std::string method, std::map<std::string, std::string>* pParams);

    // 发送响应
    static void SendHttpResponse(NetworkIdentify* pIdentify, const char* content, int size);
    static void SendHttpResponse404(NetworkIdentify* pIdentify);

    // 将 Mongoose 收到的原始 HTTP 报文转换为服务器内部的 Packet 对象，流入消息系统处理
    static Packet* ParseHttp(NetworkIdentify* pIdentify, const char* s, unsigned int bodyLen, const bool isChunked, http_message* hm);
    // 将原始 URL 字符串拆解为结构化信息ParseUrlInfo
    static bool ParseUrl(const std::string& url, ParseUrlInfo& info);

protected:
    static void DispatchPacket(Packet* packet);
    static void SendPacket(Packet* pPacket);

    // http响应
    static void SendHttpResponseBase(NetworkIdentify* pIdentify, int status_code, const char* content, int size);
};