#pragma once

#include <map>
#include "common.h"
#include "entity.h"
#include "cache_swap.h"
#include "network_help.h"
#include "connect_obj.h"
#include "trace_component.h"

#if ENGINE_PLATFORM != PLATFORM_WIN32

#define MAX_CLIENT  5120

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

#define _sock_init( )
#define _sock_nonblock( sockfd ) { int flags = fcntl(sockfd, F_GETFL, 0); fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); }
#define _sock_exit( )
#define _sock_err( )	errno
#define _sock_close( sockfd ) ::close( sockfd ) 
//#define _sock_close( sockfd ) ::shutdown(sockfd, SHUT_RDWR);
#define _sock_is_blocked()	(errno == EAGAIN || errno == 0)

#define RemoveConnectObj(socket) \
    _connects[socket]->ComponentBackToPool( ); \
    _connects[socket] = nullptr; \
    DeleteEvent(_epfd, socket); \
    _sockets.erase(socket); 

#else

#define MAX_CLIENT  10000

#define _sock_init( )	{ WSADATA wsaData; WSAStartup( MAKEWORD(2, 2), &wsaData ); }
#define _sock_nonblock( sockfd )	{ unsigned long param = 1; ioctlsocket(sockfd, FIONBIO, (unsigned long *)&param); }
#define _sock_exit( )	{ WSACleanup(); }
#define _sock_err( )	WSAGetLastError()
#define _sock_close( sockfd ) ::closesocket( sockfd )
#define _sock_is_blocked()	(WSAGetLastError() == WSAEWOULDBLOCK)

#define RemoveConnectObj(socket) \
    _connects[socket]->ComponentBackToPool( ); \
    _connects[socket] = nullptr; \
    _sockets.erase(socket); 

#define RemoveConnectObjByItem(iter) \
    _connects[*iter]->ComponentBackToPool(); \
    _connects[*iter] = nullptr; \
    iter = _sockets.erase(iter);

#endif

#if ENGINE_PLATFORM != PLATFORM_WIN32
#define SetsockOptType void *
#else
#define SetsockOptType const char *
#endif

class Packet;

class Network : public Entity<Network>, public INetwork
#if LOG_TRACE_COMPONENT_OPEN
    , public CheckTimeComponent
#endif
{
public:
    // 归还对象池前的资源清理
    void BackToPool() override;
    // 发送packet
    void SendPacket(Packet*& pPacket) override;
    // 获取该网络连接的网络类型
    NetworkType GetNetworkType() const 
    { 
        return _networkType; 
    }

protected:
    // 设置socket
    void SetSocketOpt(SOCKET socket);
    // 创建socket
    SOCKET CreateSocket();
    // 检查socket是否有误，有误则断开socket连接
    bool CheckSocket(SOCKET socket);
    // 创建连接
    bool CreateConnectObj(SOCKET socket, TagType tagType, TagValue tagValue, ConnectStateType iState);
    // 消息处理 -- 断开连接
    void HandleDisconnect(Packet* pPacket);

#ifdef EPOLL
    // 初始化Epoll
    void InitEpoll();
    // 执行一次epoll
    void Epoll();
    // 添加事件监听
    void AddEvent(int epollfd, int fd, int flag);
    // 修改事件监听
    void ModifyEvent(int epollfd, int fd, int flag);
    // 删除事件监听
    void DeleteEvent(int epollfd, int fd);
    // 对于epoll，用于标记监听套接字对应哪个epoll_event[index]，方便后续监听套接字接收连接
    virtual void OnEpoll(SOCKET fd, int index) { };
#else
    // 执行一次select
    void Select();
#endif

    // 发送packet
    void OnNetworkUpdate();

protected:  
    // std::map<SOCKET, ConnectObj*> _connects;  // 连接集合
    // 连接集合
    ConnectObj* _connects[MAX_CLIENT]{};
    std::set<SOCKET> _sockets;

#ifdef EPOLL
#define MAX_EVENT   5120
    struct epoll_event _events[MAX_EVENT];
    int _epfd = -1;
#else // selete
    SOCKET _fdMax = INVALID_SOCKET;
    fd_set readfds, writefds, exceptfds;
#endif

    std::mutex _sendMsgMutex;
    CacheSwap<Packet> _sendMsgList; // 待发送packet
    NetworkType _networkType = NetworkType::TcpListen; // 该网络连接的网络类型
};
