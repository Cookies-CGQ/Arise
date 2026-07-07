#include "network.h"
#include "connect_obj.h"
#include "packet.h"
#include "common.h"

#include <iostream>
#include "object_pool.h"

void Network::Dispose()
{
    Clean();
    ThreadObject::Dispose();
}

void Network::Clean()
{
    for (auto iter = _connects.begin(); iter != _connects.end(); ++iter)
    {
        auto pObj = iter->second;
        pObj->Dispose();
    }
    _connects.clear();

#ifdef EPOLL
    ::close(_epfd);
#endif
    _sock_close(_masterSocket);
    _masterSocket = INVALID_SOCKET;
}

void Network::RegisterMsgFunction()
{
    auto pMsgCallBack = new MessageCallBackFunction();
    AttachCallBackHandler(pMsgCallBack);
    pMsgCallBack->RegisterFunction(Proto::MsgId::MI_NetworkDisconnectToNet, BindFunP1(this, &Network::HandleDisconnect));
}

// 跨平台兼容：Linux 下 setsockopt 的 optval 参数类型为 void*，
// Windows(WinSock2) 下为 const char*。通过宏统一接口，避免平台相关的类型转换问题。
#ifndef WIN32
#define SetsockOptType void *
#else
#define SetsockOptType const char *
#endif

// 配置 socket 的各项选项，包括地址复用、收发超时、TCP KeepAlive（仅Linux）和非阻塞模式。
// 该方法在 CreateSocket() 之后、bind()/connect() 之前调用。
void Network::SetSocketOpt(SOCKET socket)
{
	// 1. SO_REUSEADDR：允许复用处于 TIME_WAIT 状态的本地地址和端口。
	//    服务器重启后可以立即绑定同一端口，无需等待 2MSL（约 60~120 秒）。
	//    对于服务器监听 socket 和快速重连的客户端都至关重要。
	bool isReuseaddr = true;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (SetsockOptType)&isReuseaddr, sizeof(isReuseaddr));

	// 2. 设置 socket 发送和接收的超时时间。
	//    SO_SNDTIMEO：send() 阻塞等待的最长时间。
	//    SO_RCVTIMEO：recv() 阻塞等待的最长时间。
	//    超时后系统调用会返回错误（Linux: EAGAIN/EWOULDBLOCK，Windows: WSAETIMEDOUT），
	//    避免线程在网络异常时无限期阻塞。
	int netTimeout = 3000; // 3000 毫秒 = 3 秒（1000 = 1秒）
	setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (SetsockOptType)&netTimeout, sizeof(netTimeout));
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (SetsockOptType)&netTimeout, sizeof(netTimeout));

#ifndef WIN32

	// 3. TCP KeepAlive 保活机制（仅 Linux/Unix 平台）。
	//    Windows 下 TCP KeepAlive 的配置方式不同（需通过 SIO_KEEPALIVE_VALS + WSAIoctl），
	//    因此这部分仅在非 Windows 平台编译。
	//    KeepAlive 用于检测"静默断开"的连接：对端崩溃、网络中断、防火墙切断空闲连接等。
	//    不开启的话，程序可能永远不知道连接已死，造成资源泄漏。

	int keepAlive = 1;       // 开启 TCP KeepAlive 机制（0=关闭，1=开启）
	socklen_t optlen = sizeof(keepAlive);

	// TCP_KEEPIDLE：连接在空闲多久（秒）后开始发送 KeepAlive 探测包。
	// 60 * 2 = 120 秒。对于游戏服务器等实时性要求较高的场景，这个值相对合理。
	int keepIdle = 60 * 2;
	// TCP_KEEPINTVL：两个 KeepAlive 探测包之间的间隔（秒）。
	// 设为 10 秒，探测失败后等 10 秒再发下一个。
	int keepInterval = 10;
	// TCP_KEEPCNT：KeepAlive 探测包的最大发送次数。
	// 设为 5 次，连续 5 次探测无响应则判定连接已死。
	// 从开始探测到判定死亡的耗时 = keepIdle + keepInterval * keepCount
	//                            ≈ 120 + 10*5 = 170 秒
	int keepCount = 5;

	// 开启 SO_KEEPALIVE 选项
	setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (SetsockOptType)&keepAlive, optlen);
	// 防御性校验：读取回设置的值，确认 KeepAlive 是否成功开启。
	if (getsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, &optlen) < 0)
	{
		std::cout << "getsockopt SO_KEEPALIVE failed." << std::endl;
	}

	// 设置 KeepAlive 空闲时间。SOL_TCP 表示选项属于 TCP 协议层。
	setsockopt(socket, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, optlen);
	if (getsockopt(socket, SOL_TCP, TCP_KEEPIDLE, &keepIdle, &optlen) < 0)
	{
		std::cout << "getsockopt TCP_KEEPIDLE failed." << std::endl;
	}

	// 设置 KeepAlive 探测间隔
	setsockopt(socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, optlen);
	// 设置 KeepAlive 探测次数
	setsockopt(socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, optlen);

#endif

	// 4. 将 socket 设为非阻塞模式。
	_sock_nonblock(socket);
}

SOCKET Network::CreateSocket()
{
    _sock_init();
    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket == INVALID_SOCKET)
    {
        std::cout << "::socket failed, err: " << _sock_err() << std::endl;
        return socket;
    }
    SetSocketOpt(socket);
    return socket;
}

void Network::CreateConnectObj(SOCKET socket)
{
    if (_connects.find(socket) != _connects.end())
    {
        std::cout << "Network::CreateConnectObj. socket is exist. socket:" << socket << std::endl;
        return;
    }
    ConnectObj* pConnectObj = DynamicObjectPool<ConnectObj>::GetInstance()->MallocObject(this, socket);
    _connects[socket] = pConnectObj;

#ifdef EPOLL
    AddEvent(_epfd, socket, EPOLLIN | EPOLLET | EPOLLRDHUP);
#endif
}

#ifdef EPOLL

#define RemoveConnectObj(iter) \
    iter->second->Dispose( ); \
    DeleteEvent(_epfd, iter->first); \
    iter = _connects.erase( iter ); 

void Network::AddEvent(int epollfd, int fd, int flag)
{
    struct epoll_event ev;
    ev.events = flag;
    ev.data.ptr = nullptr;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void Network::ModifyEvent(int epollfd, int fd, int flag)
{
    struct epoll_event ev;
    ev.events = flag;
    ev.data.ptr = nullptr;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void Network::DeleteEvent(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

void Network::InitEpoll()
{
    _epfd = epoll_create(MAX_CLIENT);
    AddEvent(_epfd, _masterSocket, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
}

void Network::Epoll()
{
    _mainSocketEventIndex = -1;
    const int nfds = epoll_wait(_epfd, _events, MAX_EVENT, 0);
    for (int index = 0; index < nfds; index++)
    {
        int fd = _events[index].data.fd;

        if (fd == _masterSocket)
        {
            _mainSocketEventIndex = index;
        }

        auto iter = _connects.find(fd);
        if (iter == _connects.end())
        {
            continue;
        }

        if (_events[index].events & EPOLLRDHUP || _events[index].events & EPOLLERR || _events[index].events & EPOLLHUP)
        {
            RemoveConnectObj(iter);
            continue;
        }

        if (_events[index].events & EPOLLIN)
        {
            if (!iter->second->Recv())
            {
                RemoveConnectObj(iter);
                continue;
            }
        }

        if (_events[index].events & EPOLLOUT)
        {
            if (!iter->second->Send())
            {
                RemoveConnectObj(iter);
                continue;
            }

            ModifyEvent(_epfd, iter->first, EPOLLIN | EPOLLRDHUP);
        }
    }

}
#else

#define RemoveConnectObj(iter) \
    iter->second->Dispose( ); \
    iter = _connects.erase( iter ); 

void Network::Select()
{
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(_masterSocket, &readfds);
    FD_SET(_masterSocket, &writefds);
    FD_SET(_masterSocket, &exceptfds);

    SOCKET fdmax = _masterSocket;

    for (auto iter = _connects.begin(); iter != _connects.end(); ++iter)
    {
        ConnectObj* pObj = iter->second;
        if (iter->first > fdmax)
            fdmax = iter->first;

        FD_SET(iter->first, &readfds);
        FD_SET(iter->first, &exceptfds);

        if (pObj->HasSendData())
            FD_SET(iter->first, &writefds);
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    const int nfds = ::select(fdmax + 1, &readfds, &writefds, &exceptfds, &timeout);
    if (nfds <= 0)
        return;

    auto iter = _connects.begin();
    while (iter != _connects.end())
    {
        if (FD_ISSET(iter->first, &exceptfds))
        {
            std::cout << "socket except!! socket:" << iter->first << std::endl;
            RemoveConnectObj(iter);
            continue;
        }

        if (FD_ISSET(iter->first, &readfds))
        {
            if (!iter->second->Recv())
            {
                RemoveConnectObj(iter);
                continue;
            }
        }

        if (FD_ISSET(iter->first, &writefds))
        {
            if (!iter->second->Send())
            {
                RemoveConnectObj(iter);
                continue;
            }
        }

        ++iter;
    }
}

#endif

void Network::Update()
{
    _sendMsgMutex.lock();
    if (_sendMsgList.CanSwap())
    {
        _sendMsgList.Swap();
    }
    _sendMsgMutex.unlock();

    auto pList = _sendMsgList.GetReaderCache();
    for (auto iter = pList->begin(); iter != pList->end(); ++iter)
    {
        Packet* pPacket = (*iter);
        auto socket = pPacket->GetSocket();
        auto itConnectObj = _connects.find(socket);
        if (itConnectObj == _connects.end())
        {
            std::cout << "send packet. can't find socket:" << socket << " msgId:" << pPacket->GetMsgId() << std::endl;
            continue;
        }

        itConnectObj->second->SendPacket(pPacket);

#ifdef  EPOLL
        ModifyEvent(_epfd, socket, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
#endif
    }
    pList->clear();
}

void Network::HandleDisconnect(Packet* pPacket)
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

void Network::SendPacket(Packet* pPacket)
{
    std::lock_guard<std::mutex> guard(_sendMsgMutex);
    _sendMsgList.GetWriterCache()->emplace_back(pPacket);
}