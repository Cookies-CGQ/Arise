#pragma once
#include "libserver/component.h"
#include "libserver/system.h"

// 挂载在player下，用于缓存和验证token是否一致
class PlayerComponentToken :public Component<PlayerComponentToken>, public IAwakeFromPoolSystem<std::string>
{
public:
	// 初始化并记录token
	void Awake(const std::string token) override;
	void BackToPool() override;
	// 验证token与缓存的是否一致
	bool IsTokenValid(std::string token) const;

private:
	std::string _token; // 缓存Player的token
};

