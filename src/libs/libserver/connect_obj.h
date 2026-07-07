#pragma once

#include <memory>
#include "disposable.h"
#include "object_block.h"
#include "network.h"
#include "util_time.h"

class RecvNetworkBuffer;
class SendNetworkBuffer;
class Packet;

#define PingTime 1000
#define PingDelayTime 10 * 1000

class ConnectObj : public ObjectBlock
{
public:
    // 因为ConnectObj使用了对象池，作为对象池的对象，这里的初始化只初始化来自哪个对象池，关于对象的初始化使用另外的函数
    ConnectObj(IDynamicObjectPool* pPool);
    virtual ~ConnectObj();

    // 获取socket
    SOCKET GetSocket() const { return _socket; }
    // 接收缓冲区是否有数据
    bool HasRecvData() const;
    // 获取packet
    Packet *GetRecvPacket() const;
    // 接收数据到接收缓冲区，并广播给Actor
    bool Recv() const;

    // 发送缓冲区是否有数据
    bool HasSendData() const;
    // 发送packet到发送缓冲区
    void SendPacket(Packet *pPacket) const;
    // 从发送缓存区读取发送数据
    bool Send() const;
    // 关闭连接
    void Close();

    // 用于对象池中初始化对象的函数
    void TakeoutFromPool(Network* pNetWork, SOCKET socket);
    // 回收对象
    virtual void BackToPool() override;

protected:
    Network *_pNetWork = nullptr;             // 父级网络对象：NetworkListen / NetworkConnector
    SOCKET _socket;                           // socket，生命由父级网络对象创建，由自生销毁
    RecvNetworkBuffer* _recvBuffer = nullptr; // 接收缓冲区
    SendNetworkBuffer* _sendBuffer = nullptr; // 发送缓冲区
};
