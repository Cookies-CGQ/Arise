#pragma once

#include <functional>
#include "component.h"
#include "system.h"

// update组件，如果实体需要update，则包含这个组件，并写入UpdateFunction
class UpdateComponent :public Component<UpdateComponent>, public IAwakeFromPoolSystem<>
{
public:
    // 初始化
	void Awake() override;
    // 归还对象池
    void BackToPool() override;

    // update函数
	std::function<void()> UpdataFunction = nullptr;
};

