#pragma once

#include <map>
#include "common.h"
#include "entity.h"
#include "cache_swap.h"
#include "network_help.h"
#include "connect_obj.h"

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

#define _sock_init( )
#define _sock_nonblock( sockfd ) { int flags = fcntl(sockfd, F_GETFL, 0); fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); }
#define _sock_exit( )
#define _sock_err( )	errno
#define _sock_close( sockfd ) ::close( sockfd ) 
#define _sock_is_blocked()	(errno == EAGAIN || errno == 0)

#define RemoveConnectObj(iter) \
    iter->second->ComponentBackToPool( ); \
    DeleteEvent(_epfd, iter->first); \
    iter = _connects.erase( iter ); 

#else

#define _sock_init( )	{ WSADATA wsaData; WSAStartup( MAKEWORD(2, 2), &wsaData ); }
#define _sock_nonblock( sockfd )	{ unsigned long param = 1; ioctlsocket(sockfd, FIONBIO, (unsigned long *)&param); }
#define _sock_exit( )	{ WSACleanup(); }
#define _sock_err( )	WSAGetLastError()
#define _sock_close( sockfd ) ::closesocket( sockfd )
#define _sock_is_blocked()	(WSAGetLastError() == WSAEWOULDBLOCK)

#define RemoveConnectObj(iter) \
    iter->second->ComponentBackToPool( ); \
    iter = _connects.erase( iter ); 

#endif

#if ENGINE_PLATFORM != PLATFORM_WIN32
#define SetsockOptType void *
#else
#define SetsockOptType const char *
#endif

class Packet;

class Network : public Entity<Network>, public INetwork
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
    // 
    bool CheckSocket(SOCKET socket);
    // 
    bool CreateConnectObj(SOCKET socket, ObjectKey key, ConnectStateType iState);
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
    // 
    virtual void OnEpoll(SOCKET fd, int index) { };
#else
    // 执行一次select
    void Select();
#endif

    // 发送packet
    void OnNetworkUpdate();

protected:  
    std::map<SOCKET, ConnectObj*> _connects;  // 连接集合

#ifdef EPOLL
#define MAX_CLIENT  5120
#define MAX_EVENT   5120
    struct epoll_event _events[MAX_EVENT];
    int _epfd;
#else // selete
    SOCKET _fdMax = INVALID_SOCKET;
    fd_set readfds, writefds, exceptfds;
#endif

    std::mutex _sendMsgMutex;
    CacheSwap<Packet> _sendMsgList; // 待发送packet
    NetworkType _networkType = NetworkType::TcpListen; // 该网络连接的网络类型
};
