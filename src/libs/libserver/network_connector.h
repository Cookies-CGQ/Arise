#pragma once

#include <utility>
#include "network.h"
#include "connect_obj.h"
#include "app_type.h"

class Packet;

// 待连接信息
struct ConnectDetail: public SnObject, public IDisposable
{
public:
    ConnectDetail(ObjectKey key, std::string ip, int port)
    {
        Key = std::move(key);
        Ip = std::move(ip);
        Port = port;
    };

    void Dispose() override 
    { 

    }

    std::string Ip =  "";
    int Port = 0;
    ObjectKey Key { ObjectKeyType::None , { 0, ""} };
};


class NetworkConnector : public Network, public IAwakeSystem<int, int>
{
public:
    // 初始化Connector，iType表示网络连接类型，mixConnectAppType表示需要直接连接的服务
    void Awake(int iType, int mixConnectAppType) override;

    virtual void Update();

    const char* GetTypeName() override;
    uint64 GetTypeHashCode() override;
    // 连接处理
    bool Connect(ConnectDetail* pDetail);

private:
    // 消息处理 -- 请求连接处理
    void HandleNetworkConnect(Packet* pPacket);
    // 预创建连接，用于连接其他服务
    void CreateConnector(APP_TYPE appType, int appId);
    // 对于创建连接，一般有两种：
    //     1、连接其他服务，一般初始化时直接创建连接
    //     2、其他连接，一般通过消息机制通知Connector建立连接，例如：robot

private:
    // 待连接
    CacheRefresh<ConnectDetail> _connecting;
};

