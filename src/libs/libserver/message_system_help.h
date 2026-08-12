#pragma once

#include "common.h"
#include "app_type.h"
#include "packet.h"
#include "network_type.h"

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
    // 对象池分配一个packet，绑定消息类型和网络/对象标识
    static Packet* CreatePacket(Proto::MsgId msgId, NetIdentify* pIdentify);

    // 创建网络连接请求
    static void CreateConnect(NetworkType iType, TagType tagType, TagValue& tagValue, std::string ip, int port);

    // 创建并分发一个packet（向内分发）
    static void DispatchPacket(const Proto::MsgId msgId, NetIdentify* pIdentify);
    static void DispatchPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, NetIdentify* pIdentify);

    // 创建并发送一个packet（向外分发 -- 走网络出去）
    static void SendPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, NetIdentify* pIdentify);
    static void SendPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, APP_TYPE appType, int appId = 0);
    static void SendPacket(const Proto::MsgId msgId, google::protobuf::Message& proto, TagKey* pTagKey, APP_TYPE appType, int appId = 0);
    static void SendPacket(const Proto::MsgId msgId, TagKey* pTagKey, APP_TYPE appType, int appId = 0);
    static void SendPacket(Packet* pPacket);
    static void SendPacket(Packet* pPacket, APP_TYPE appType, int appId);

    // 发送packet到指定服务
    static void SendPacketToAllApp(Proto::MsgId msgId, google::protobuf::Message& proto, APP_TYPE appType);

    // http
    // 发送请求
    static void SendHttpRequest(NetIdentify* pIdentify, std::string ip, int port, std::string method, std::map<std::string, std::string>* pParams);

    // 发送响应
    static void SendHttpResponse(NetIdentify* pIdentify, const char* content, int size);
    static void SendHttpResponse404(NetIdentify* pIdentify);

    // 将 Mongoose 收到的原始 HTTP 报文转换为服务器内部的 Packet 对象，流入消息系统处理
    static Packet* ParseHttp(NetIdentify* pIdentify, const char* s, unsigned int bodyLen, const bool isChunked, http_message* hm);
    // 将原始 URL 字符串拆解为结构化信息ParseUrlInfo
    static bool ParseUrl(const std::string& url, ParseUrlInfo& info);

protected:
    static void DispatchPacket(Packet* packet);

    // http响应
    static void SendHttpResponseBase(NetIdentify* pIdentify, int status_code, const char* content, int size);
};