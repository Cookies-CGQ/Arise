#pragma once

#include "network.h"

class NetworkListen :public Network, public IAwakeSystem<std::string, int>, public IAwakeSystem<int, int>
{
public:
    // 初始化 -- 用于开启HTTP Listen
    void Awake(std::string ip, int port) override;
    // 初始化 -- 用于开启TCP Listen
    void Awake(int appType, int appId) override;
    // 初始化
    void Awake(std::string ip, int port, NetworkType iType);

    void BackToPool() override;

    virtual void Update();
    const char* GetTypeName() override;
    uint64 GetTypeHashCode() override;

    // 用于控制台命令显示具体信息
    void CmdShow();

    static bool IsSingle() { return true; }

private:
    void HandleListenKey(Packet* pPacket);

protected:
    virtual int Accept();
#ifdef EPOLL
    virtual void OnEpoll(SOCKET fd, int index) override;
#endif

private:

#ifdef EPOLL
    int _mainSocketEventIndex{ -1 };   
#endif

    SOCKET _masterSocket = INVALID_SOCKET;    
};