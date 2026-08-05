#pragma once

#include "libserver/common.h"
#include "libserver/socket_object.h"
#include "libserver/entity.h"
#include "libserver/system.h"

// 玩家实体类
class Player: public Entity<Player>, public NetworkIdentify, virtual public IAwakeFromPoolSystem<NetworkIdentify*, std::string>
{
public:
	void Awake(NetworkIdentify* pIdentify, std::string account) override;
    void BackToPool() override;

    // 获取账号
	std::string GetAccount() const;
	// 获取昵称
    std::string GetName() const;
	// 获取序列号
    uint64 GetPlayerSN() const;
    // 获取proto结构
	Proto::Player& GetPlayerProto();

    // 从protobuf二进制流 -> 玩家数据
	void ParseFromStream(uint64 playerSn, std::stringstream* pOpStream);
	// 从 proto 对象反序列化，并遍历所有 PlayerComponent 子组件同步解析
    void ParserFromProto(uint64 playerSn, const Proto::Player& proto);
	// 玩家数据 -> proto
    void SerializeToProto(Proto::Player* pProto) const;

protected:
	std::string _account = ""; // 账号
	std::string _name = "";    // 昵称
  
	uint64 _playerSn = 0;      // 序列号
	Proto::Player _player;     // proto结构：sn、name、PlayerBase（gender、level）、PlayerMisc（last_world、last_dungeon、online_version）
};

