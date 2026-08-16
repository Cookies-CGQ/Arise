#include "player_component_detail.h"

#include "libplayer/player.h"
#include "libserver/message_system_help.h"

void PlayerComponentDetail::Awake()
{
	Player* pPlayer = dynamic_cast<Player*>(_parent);
	ParserFromProto(pPlayer->GetPlayerProto());
}

void PlayerComponentDetail::BackToPool()
{
}

void PlayerComponentDetail::ParserFromProto(const Proto::Player& proto)
{
	auto protoBase = proto.base();
	_gender = protoBase.gender();
}

void PlayerComponentDetail::SerializeToProto(Proto::Player* pProto)
{
    // 性别不更改，可以不写回
}

Proto::Gender PlayerComponentDetail::GetGender() const
{
	return _gender;
}