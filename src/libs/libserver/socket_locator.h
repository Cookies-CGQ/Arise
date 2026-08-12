#pragma once

#include "system.h"
#include "entity.h"

class Packet;

// Socket -> Entity 路由，作为网络层和逻辑层的桥梁组件
class SocketLocator :public Entity<SocketLocator>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

    // 添加到路由
    void RegisterToLocator(SOCKET socket, uint64 sn);
    // 从路由删除
    void Remove(SOCKET socket);
    // 通过socket获取到entitySN
    uint64 GetTargetEntitySn(SOCKET socket);

private:
    // 消息处理 -- 连接断开清理
    void HandleNetworkDisconnect(Packet* pPacket);

private:
    std::mutex _lock;
    // 路由表，socket : entitySN
    std::map<SOCKET, uint64> _componentes;
};