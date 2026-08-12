#pragma once

#include "libserver/common.h"

// Player实体的组件的基类
class PlayerComponent
{
public:
	// proto数据 -> 组件内存数据，需要根据子类组件根据业务实现
	virtual void ParserFromProto(const Proto::Player& proto) = 0;
	// 组件内存数据 -> proto数据，需要根据子类组件根据业务实现
	virtual void SerializeToProto(Proto::Player* pProto) = 0;
};