#pragma once

#include "libserver/entity.h"
#include "libplayer/player.h"

// 挂载在world实体下的Player管理器，是一张地图里玩家的唯一事实来源
class PlayerManagerComponent:public Entity<PlayerManagerComponent>, public IAwakeFromPoolSystem<>
{
public:
	void Awake() override;
    void BackToPool() override;

    // 添加玩家
	Player* AddPlayer(uint64 playerSn, uint64 worldSn, NetIdentify* pNetIdentify);
	// 获取玩家
    Player* GetPlayerBySn(uint64 playerSn);
	// 删除玩家
    void RemovePlayerBySn(uint64 playerSn);
    // 删除玩家
    void RemoveAllPlayers(NetIdentify* pNetIdentify);
    // 获取world实体的负载人数
	int OnlineSize() const;
    // 获取全部
	std::map<uint64, Player*>* GetAll();

private:
	std::map<uint64, Player*> _players; // 玩家sn : Player*
};