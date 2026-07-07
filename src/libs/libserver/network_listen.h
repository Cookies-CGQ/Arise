#pragma once

#include "network.h"

#include <string>

class NetworkListen : public Network
{
public:
    bool Init() override;
    // 监听
    bool Listen(std::string ip, int port);
    void Update() override;
protected:
    // 接收
    virtual int Accept();
};
