#pragma once

#include "common.h"
#include "entity.h"
#include "system.h"
#include "socket_object.h"

class RecvNetworkBuffer;
class SendNetworkBuffer;
class Packet;

#define PingTime 1000 // 1秒
#define PingDelayTime  10 * 1000 // 10秒

// 表示连接的状态
enum class ConnectStateType
{
    None,        
    Connecting, // 正在连接
    Connected,  // 已连接
};

class ConnectObj : public Entity<ConnectObj>, public NetworkIdentify, public IAwakeFromPoolSystem<SOCKET, NetworkType, ObjectKey, ConnectStateType>
{
public:
    ConnectObj();
    virtual ~ConnectObj();

    // 对象池初始化
    void Awake(SOCKET socket, NetworkType networkType, ObjectKey key, ConnectStateType state) override;
	// 归还对象池前资源清理
    virtual void BackToPool() override;

	// 是否有接收信息
    bool HasRecvData() const;
    // 接收并分发packet
	bool Recv();

    // 是否有发送信息
	bool HasSendData() const;
	// 发送packet
    void SendPacket(Packet* pPacket) const;

	// 发送
    bool Send() const;
    
    void Close();
    
    // 获取状态
    ConnectStateType GetState() const;
    // 状态转移 -> 已连接
    void ChangeStateToConnected();
    // 修改ObjectKey
    void ModifyConnectKey(ObjectKey key);

protected:
    ConnectStateType _state = ConnectStateType::None; // 连接状态

    RecvNetworkBuffer* _recvBuffer = nullptr; // 接收缓冲区
    SendNetworkBuffer* _sendBuffer = nullptr; // 发送缓冲区
};