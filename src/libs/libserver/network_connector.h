#pragma once 

#include <memory>
#include <string>
#include "network.h"

// 前向声明
class ConnectObj;
class Packet;

class NetworkConnector : public Network
{
public: 
    bool Init() override;
    // 连接服务器
	virtual bool Connect(std::string ip, int port);
	// 帧函数
    void Update() override;
    // 是否连接
	bool IsConnected() const;

private:
	void TryCreateConnectObj();

private:
    std::string _ip {""};
    int _port {0};
};