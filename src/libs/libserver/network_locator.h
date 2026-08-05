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
	// 获取Connector
    INetwork* GetConnector(NetworkType networkType);

	// 添加服务连接
	void AddNetworkIdentify(uint64 appKey, SocketKey socket, ObjectKey objKey);
    // 移除服务连接
	void RemoveNetworkIdentify(uint64 appKey);
	// 获取服务连接
	NetworkIdentify GetNetworkIdentify(const APP_TYPE appType, const int appId);

    // 添加Listen
    void AddListenLocator(INetwork* pNetwork, NetworkType networkType);
    // 获取Listen
	INetwork* GetListen(NetworkType networkType);

private:
    std::mutex _lock;

    // <apptype + appId, NetworkIdentify>，服务连接映射表
    std::map<uint64, NetworkIdentify> _netIdentify;

    // TCP Connector / HTTP Connector
    std::map<NetworkType, INetwork*> _connectors;

    // TCP Listen / HTTP Listen
    std::map<NetworkType, INetwork*> _listens;
};