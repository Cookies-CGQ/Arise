#pragma once

#include <memory>
#include "network.h"

class ConnectObj;
class Packet;

class NetworkConnector : public Network, virtual public IAwakeSystem<std::string, int>, public virtual IAwakeSystem<int, int>
{
public:
    // 初始化 -- 两种方式
    void Awake(std::string ip, int port);
    void Awake(int appType, int appId);
    // 帧函数
    virtual void Update();
    // 是否连接
    bool IsConnected() const;
    // 获取类型名
    const char* GetTypeName() override;
    // 对象池为空时只创建一个对象，不进行预创建
    static bool IsSingle() { return true; }

protected:
    // 发出连接请求
    bool Connect(std::string ip, int port);

private:
    void TryCreateConnectObj();

protected:
    std::string _ip = "";
    int _port = 0;
};

