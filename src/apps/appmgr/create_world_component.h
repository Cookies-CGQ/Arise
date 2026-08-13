#pragma once

#include "libserver/entity.h"
#include "libserver/sync_component.h"

class Packet;

// 公共地图：game请求 -> 向appmgr申请 -> appmgr分配space -> space创建
// 副本地图（创建不走appmgr）：game挑选space发送创建指令 -> space创建 --------> appmgr只负责事后登记

/* 整个服务器中地图的创建调度中心，
   Game进程需要某个地图时向这里申请并挑选一个Space进程去创建地图，
   并登记所有 已创建/创建中 的地图，配合其他进程的代码，可以串起一条完整的“创建世界链路”
*/
class CreateWorldComponent :public SyncComponent, public Entity<CreateWorldComponent>, public IAwakeSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

private:
	// 分配Space进程去创建地图
    int ReqCreateWorld(int worldId);

	// 消息处理 -- 控制台调试命令
    void HandleCmdCreate(Packet* pPacket);
	// 消息处理 -- 其他进程同步状态
    void HandleAppInfoSync(Packet* pPacket);
	// 消息处理 -- 网络断开
	void HandleNetworkDisconnect(Packet* pPacket) override;
	// 消息处理 -- Game进程申请公共地图，没有则创建，有则返回
	void HandleRequestWorld(Packet* pPacket);
	// 消息处理 -- Game玩家传送到副本地图前向appmgr查询副本实例是否还存在
	void HandleQueryWorld(Packet* pPacket);
	// 消息处理 -- 收到地图创建成功的信息
	void HandleBroadcastCreateWorld(Packet* pPacket);

private:
	// <world id, space Id>
	std::map<int, int> _creating;       // 正在创建中的公共地图
	// <world id, world sn>
	std::map<int, uint64> _created;     // 已经创建的公共地图信息
	// <world sn, world id>
	std::map<uint64, int> _dungeons;	// 已经创建的副本地图
};