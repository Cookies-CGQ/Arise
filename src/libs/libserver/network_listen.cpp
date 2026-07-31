#include <iostream>

#include "common.h"
#include "network_listen.h"
#include "connect_obj.h"
#include "thread_mgr.h"
#include "network_locator.h"
#include "log4_help.h"
#include "message_component.h"
#include "update_component.h"

void NetworkListen::Awake(std::string ip, int port)
{
    // 添加到网络对象路由表中
    auto pNetworkLocator = ThreadMgr::GetInstance()->GetEntitySystem()->GetComponent<NetworkLocator>();
    pNetworkLocator->AddListenLocator(this, NetworkTcpListen);

    // 添加消息组件并注册消息处理函数
    auto pMsgCallBack = new MessageCallBackFunction();
    AddComponent<MessageComponent>(pMsgCallBack);    
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_NetworkRequestDisconnect, BindFunP1(this, &NetworkListen::HandleDisconnect));

    // 添加update组件
    auto pUpdateComponent = AddComponent<UpdateComponent>();
    pUpdateComponent->UpdataFunction = BindFunP0(this, &NetworkListen::Update);

    // 初始化监听socket
    _masterSocket = CreateSocket();
    if (_masterSocket == INVALID_SOCKET)
        return;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);

    if (::bind(_masterSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::cout << "::bind failed. err:" << _sock_err() << std::endl;
        return;
    }

    if (::listen(_masterSocket, SOMAXCONN) < 0)
    {
        std::cout << "::listen failed." << _sock_err() << std::endl;
        return;
    }

#ifdef EPOLL
    LOG_INFO("epoll model. listen " << ip.c_str() << ":" << port);
    InitEpoll();
#else
    LOG_INFO("select model. listen " << ip.c_str() << ":" << port);
#endif

    return;
}

int NetworkListen::Accept()
{
    struct sockaddr socketClient;
    socklen_t socketLength = sizeof(socketClient);

    int rs = 0;
    // 循环接收，因为_masterSocket也是非阻塞的
    while (true)
    {
        // accept -> 设置socket -> 创建连接对象
        const SOCKET socket = ::accept(_masterSocket, &socketClient, &socketLength);
        if (socket == INVALID_SOCKET)
            return rs;

        SetSocketOpt(socket);
        CreateConnectObj(socket);

        ++rs;
    }

    return rs;
}

const char* NetworkListen::GetTypeName()
{
    return typeid(NetworkListen).name();
}

#ifndef EPOLL

void NetworkListen::Update()
{
    // 进行以此select
    Select();

    // 是否有连接请求
    if (FD_ISSET(_masterSocket, &readfds))
    {
        Accept();
    }

    // 处理待发送的packet
    Network::OnNetworkUpdate();
}

#else

void NetworkListen::Update()
{
    Epoll();

    if (_mainSocketEventIndex >= 0)
    {
        Accept();
    }

    Network::OnNetworkUpdate();
}

#endif

void NetworkListen::HandleDisconnect(Packet* pPacket)
{
    auto socket = pPacket->GetSocket();
    auto iter = _connects.find(socket);
    if (iter == _connects.end())
    {
        std::cout << "dis connect failed. socket not find. socket:" << socket << std::endl;
        return;
    }

    RemoveConnectObj(iter);
    std::cout << "logical layer requires shutdown. socket:" << socket << std::endl;
}
