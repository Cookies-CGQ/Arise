#pragma once
#include "libserver/system.h"
#include "libserver/entity.h"

class WorldComponentGather:public Entity<WorldComponentGather>, public IAwakeSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

private:
	// 定时器 -- 定时向WorldProxyGather同步发送本世界的相关信息
	void SyncWorldInfoToGather() const;
};
