#include <iostream>
#include "connect_obj.h"
#include "network_connector.h"
#include "packet.h"

bool NetworkConnector::Init()
{
    return true;
}

bool NetworkConnector::IsConnected() const
{
    return _connects.size() > 0;
}

bool NetworkConnector::Connect(std::string ip, int port)
{
    _ip = ip;
    _port = port;

    if(_ip == "" || _port == 0)
        return false;

    _masterSocket = CreateSocket();
    if(_masterSocket == INVALID_SOCKET)
        return false;

#ifdef EPOLL
	InitEpoll();
#endif

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    if (::inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) != 1)
    {
        std::cout << "invalid connect ip: " << _ip << std::endl;
        _sock_close(_masterSocket);
        _masterSocket = INVALID_SOCKET;
        return false;
    }
    addr.sin_port = htons(port);

    int ret = ::connect(_masterSocket, (sockaddr*)&addr, sizeof(sockaddr));
    if(ret == 0)
    {
        CreateConnectObj(_masterSocket);
    }
    // 到这里也可能连接还没建立
    return true;
}

void NetworkConnector::TryCreateConnectObj()
{
	int optval = -1;
	socklen_t optlen = sizeof(optval);
	int rs = ::getsockopt(_masterSocket, SOL_SOCKET, SO_ERROR, (char*)(&optval), &optlen);
	if (rs == 0 && optval == 0)
	{
		CreateConnectObj(_masterSocket);
	}
	else
	{
		std::cout << "connect failed. socket:" << _masterSocket << std::endl;
		Dispose();
	}
}

#ifdef EPOLL
void NetworkConnector::Update()
{
	// 如果断线，重新连接
	if (_masterSocket == INVALID_SOCKET)
	{
		if (!Connect(_ip, _port))
			return;

		std::cout << "re connect. socket:" << _masterSocket << std::endl;
	}

	Epoll();

    if (!IsConnected()) 
    {
        if (_mainSocketEventIndex >= 0)
        {
            int fd = _events[_mainSocketEventIndex].data.fd;
            if (fd != _masterSocket)
                return;

            if (_events[_mainSocketEventIndex].events & EPOLLIN || _events[_mainSocketEventIndex].events & EPOLLOUT)
            {
                TryCreateConnectObj();
            }
        }
    }

    Network::Update();
}
#else

void NetworkConnector::Update()
{
	// 如果断线，重新连接
	if (_masterSocket == INVALID_SOCKET)
	{
		if (!Connect(_ip, _port))
			return;

		std::cout << "re connect. socket:" << _masterSocket << std::endl;
	}

	Select();

	if (!IsConnected())
	{
		// 有异常出现
		if (FD_ISSET(_masterSocket, &exceptfds))
		{
			std::cout << "connect except. socket:" << _masterSocket << " re connect." << std::endl;

			// 关闭当前socket，重新connect
			Clean();
			return;
		}

		if (FD_ISSET(_masterSocket, &readfds) || FD_ISSET(_masterSocket, &writefds))
		{
			TryCreateConnectObj();
		}
	}
    Network::Update();
}

#endif