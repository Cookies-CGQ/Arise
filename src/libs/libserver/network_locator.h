#pragma once

#include "network.h"
#include "system.h"
#include "app_type.h"
#include "socket_object.h"

class NetworkLocator : public Entity<NetworkLocator>, public IAwakeSystem<>
{
public:
    void Awake() override;;
    void BackToPool() override;

    // 添加Connector
    void AddConnectorLocator(INetwork* pNetwork, NetworkType networkType);
	// 添加服务连接 -- 服务连接注册信息，并发送本服务信息到对端服务（连接了对方，要告诉对方是谁）
    void AddNetworkIdentify(SocketKey* pSocket, uint64 appKey);
    // 按应用类型查询所有已连接实例
    std::list<NetIdentify> GetAppNetworks(const APP_TYPE appType);
	// 获取服务连接
    NetIdentify GetNetworkIdentify(const APP_TYPE appType, const int appId);
    // 添加Listen
    void AddListenLocator(INetwork* pNetwork, NetworkType networkType);
    // 获取网络对象
    INetwork* GetNetwork(NetworkType networkType);

protected:
    // 消息处理 -- 收到对端服务的自我介绍，并修改网络底层标识
    void HandleAppRegister(Packet* pPacket);
    // 消息处理 -- 服务连接断开，尝试重连
    void HandleNetworkDisconnect(Packet* pPacket);

private:
    std::mutex _lock;

    // <apptype + appId, NetIdentify>，管理服务间的连接
    std::map<uint64, NetIdentify> _netIdentify;

    // TCP Connector / HTTP Connector，管理本地主动连出去的网络对象
    std::map<NetworkType, INetwork*> _connectors;

    // TCP Listen / HTTP Listen，管理被动监听的网络对象
    std::map<NetworkType, INetwork*> _listens;
};