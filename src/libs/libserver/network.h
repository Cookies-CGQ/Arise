#pragma once

#include <map>
#include "common.h"
#include "thread_obj.h"
#include "socket_object.h"
#include "cache_swap.h"

#if ENGINE_PLATFORM != PLATFORM_WIN32
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#ifdef EPOLL
#include <sys/epoll.h>
#endif

#define _sock_init()
#define _sock_nonblock(sockfd)                      \
    {                                               \
        int flags = fcntl(sockfd, F_GETFL, 0);      \
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); \
    }
#define _sock_exit()
#define _sock_err() errno
#define _sock_close(sockfd) ::close(sockfd)
#define _sock_is_blocked() (errno == EAGAIN || errno == EWOULDBLOCK)

#else

#define FD_SETSIZE 1024

#include <Ws2tcpip.h>
#include <windows.h>

#define _sock_init()                          \
    {                                         \
        WSADATA wsaData;                      \
        WSAStartup(MAKEWORD(2, 2), &wsaData); \
    }
#define _sock_nonblock(sockfd)                                 \
    {                                                          \
        unsigned long param = 1;                               \
        ioctlsocket(sockfd, FIONBIO, (unsigned long *)&param); \
    }
#define _sock_exit()  \
    {                 \
        WSACleanup(); \
    }
#define _sock_err() WSAGetLastError()
#define _sock_close(sockfd) ::closesocket(sockfd)
#define _sock_is_blocked() (WSAGetLastError() == WSAEWOULDBLOCK)

#endif

// 前向声明
class ConnectObj;
class Packet;

// NetworListen 和 NetworkConnector 的基类
class Network : public ThreadObject, public ISocketObject
{
public:
    // 释放资源
    void Dispose() override;
    // 注册感兴趣的协议
    void RegisterMsgFunction() override;
    // 获取套接字
    SOCKET GetSocket() override { return _masterSocket; }
    // 发送packet包
    void SendPacket(Packet* packet);
    // 设置是否为广播模式
    bool IsBroadcast() { return _isBroadcast; }

protected:
    // 设置套接字
    static void SetSocketOpt(SOCKET socket);
    // 创建套接字
    static SOCKET CreateSocket();
    // 创建连接对象
    void CreateConnectObj(SOCKET socket);
    // 清理
    void Clean();

#ifdef EPOLL
    // 初始化epoll
    void InitEpoll();
    // 进行一次epoll_wait，处理tcp数据读写
    void Epoll();
    // 添加事件监听
    void AddEvent(int epollfd, int fd, int flag);
    // 修改事件监听
    void ModifyEvent(int epollfd, int fd, int flag);
    // 删除事件监听
    void DeleteEvent(int epollfd, int fd);
#else
    // 进行一次select，处理tcp数据读写
    void Select();
#endif
    // 帧函数
    void Update() override;

private:
    // 取消连接
    void HandleDisconnect(Packet* pPacket);

protected:
    SOCKET _masterSocket{INVALID_SOCKET};       // 对于NetworkListen来说是监听socket；对于NetworkConnector来说是连接socket
    std::map<SOCKET, ConnectObj*> _connects;   // 对于NetworkListen来说是用于存储多个客户端连接；对于NetworkConnector来说是存储自生连接

#ifdef EPOLL
#define MAX_CLIENT  5120
#define MAX_EVENT   5120
	struct epoll_event _events[MAX_EVENT];
	int _epfd; // epoll fd
	int _mainSocketEventIndex{ -1 };
#else
    fd_set readfds, writefds, exceptfds;  
#endif      
    // 用于发送协议的锁
    std::mutex _sendMsgMutex;
    // 待发送协议队列
    CacheSwap<Packet> _sendMsgList;
    // 收到的协议是否需要广播到全线程
    bool _isBroadcast{ true };
};
