#include <iostream>
#include "network_listen.h"
#include "connect_obj.h"
#include "common.h"

bool NetworkListen::Init()
{
    return true;
}

bool NetworkListen::Listen(std::string ip, int port)
{
    // 创建并设置套接字
    _masterSocket = CreateSocket();
    if(_masterSocket == INVALID_SOCKET)
        return false;
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    if (ip.empty() || ip == "0.0.0.0")
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr) != 1)
    {
        std::cout << "invalid listen ip: " << ip << std::endl;
        _sock_close(_masterSocket);
        _masterSocket = INVALID_SOCKET;
        return false;
    }
    addr.sin_port = htons(port);
    
    // bind
    if(::bind(_masterSocket, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cout << "::bind failed, err: " << _sock_err() << std::endl;
        _sock_close(_masterSocket);
        _masterSocket = INVALID_SOCKET;
        return false;
    }
    
    // listen
    if (::listen(_masterSocket, SOMAXCONN) < 0)
    {
        std::cout << "::listen failed." << _sock_err() << std::endl;
        return false;
    }

#ifdef EPOLL
	InitEpoll();
#endif

    return true;
}

int NetworkListen::Accept()
{
    sockaddr socketClient;
    socklen_t socketLength = sizeof(socketClient);
    
    // 获取连接个数
    int cnt = 0;
    // socket非阻塞，一次性全部读完
    while(true)
    {
        // 接收并接收套接字
        SOCKET socket = ::accept(_masterSocket, &socketClient, &socketLength);
        if(socket == INVALID_SOCKET)
            return cnt;
        SetSocketOpt(socket);
        CreateConnectObj(socket);
        ++cnt;
    }
}

#ifndef EPOLL
void NetworkListen::Update()
{
	Select();

	if (FD_ISSET(_masterSocket, &readfds))
	{
		Accept();
	}
    Network::Update();
}
#else
void NetworkListen::Update()
{
	Epoll();

	if (_mainSocketEventIndex >= 0)
	{
		Accept();
	}
    Network::Update();
}
#endif
