#pragma once

#include "libserver/system.h"
#include "libserver/component.h"
#include "libserver/socket_object.h"

class Player;

// Player收集器，Player对象的起点和终点
class PlayerCollectorComponent :public Component<PlayerCollectorComponent>, public IAwakeFromPoolSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

    // 创建Player实体并注册到两张映射表中
	Player* AddPlayer(NetIdentify* pIdentify, std::string account);

    // Player移除
	void RemovePlayerBySocket(SOCKET socket);
	void RemovePlayerBySn(uint64 playerSn);

	// 移除所有Player并请求断开底层连接
    void RemoveAllPlayerAndCloseConnect();

    // 获取Player
	Player* GetPlayerBySocket(SOCKET socket);
	Player* GetPlayerByAccount(std::string account);
	Player* GetPlayerBySn(uint64 playerSn);

    // 获取Player人数
	int OnlineSize() const;

	std::map<SOCKET, Player*>& GetAll();

protected:
    void RemovePlayer(Player* pPlayer);

private:
	// <socket, Player*>
	std::map<SOCKET, Player*> _players;

	// <account, socket>
	std::map <std::string, SOCKET> _accounts;
};