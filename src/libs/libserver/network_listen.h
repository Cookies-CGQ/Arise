#pragma once

#include "network.h"

class NetworkListen: public Network, public IAwakeSystem<std::string, int>
{
public:
    // 初始化
    void Awake(std::string ip, int port) override;
    // 帧函数
    void Update();
    // 获取类型名
    const char* GetTypeName() override;

private:
    // 消息处理函数--关闭连接
    void HandleDisconnect(Packet* pPacket);

protected:
    // 接收连接请求
    virtual int Accept();
};
