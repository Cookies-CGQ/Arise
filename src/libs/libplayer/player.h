#pragma once

#include "libserver/common.h"
#include "libserver/socket_object.h"
#include "libserver/entity.h"
#include "libserver/system.h"

// 玩家实体类
class Player : public Entity<Player>, public NetIdentify, virtual public IAwakeFromPoolSystem<NetIdentify*, std::string>, virtual public IAwakeFromPoolSystem<NetIdentify*, uint64, uint64>
{
public:
	// 初始化场景 -- 用于玩家首次账号登录，暂时还没SN的情况
    void Awake(NetIdentify* pIdentify, std::string account) override;
    // 初始化场景 -- 玩家重连 / 进入世界（已分配了SN的情况）
	void Awake(NetIdentify* pIdentify, uint64 playerSn, uint64 worldSn) override;
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
	// 从内存数据结构 -> proto结构，同样遍历所有组件
    void SerializeToProto(Proto::Player* pProto) const;

protected:
	std::string _account = ""; // 账号
	std::string _name = "";    // 昵称
  
	uint64 _playerSn = 0;      // 序列号
	Proto::Player _player;     // 自己数据的主副本，是完整一份大结构，但是各个字段分属不同组件
};