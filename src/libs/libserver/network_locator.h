#pragma once

#include "network.h"
#include "system.h"

// 进程内的网络对象路由表（连接/监听）
class NetworkLocator : public Component<NetworkLocator>, public IAwakeSystem<>
{
public:
	void Awake() override {};
    // 归还对象池之前清理资源
	void BackToPool() override;
    // 添加连接对象
	void AddConnectorLocator(INetwork* pNetwork, APP_TYPE appType, int appId);
	// 添加监听对象
    void AddListenLocator(INetwork* pNetwork, NetworkType networkType);

    // 获取监听对象
	INetwork* GetListen(NetworkType networkType);
	// 获取连接对象
    INetwork* GetNetworkConnector(const SOCKET socket);
	// 获取连接对象
    INetwork* GetNetworkConnector(const APP_TYPE appType, const int appId);
    // 获取连接对象
	std::tuple<APP_TYPE, int> GetNetworkConnectorInfo(const SOCKET socket);
    // 从 _connectors 中取出某个 APP_TYPE 下的所有 INetwork*，放进一个 std::list返回。
	std::list<INetwork*> GetNetworks(const APP_TYPE appType);

    // 获取socket对应的连接对象类型
	APP_TYPE GetNetworkAppType(const int socket);
    // 获取socket对应的连接对象的Id
    int GetNetworkAppId(const SOCKET socket);

private:
	std::mutex _lock;
	std::map<APP_TYPE, std::map<int, INetwork*>> _connectors; // 连接对象
	std::map<NetworkType, INetwork*> _listens;                // 监听对象
};