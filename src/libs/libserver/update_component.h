#pragma once

#include <functional>
#include "component.h"
#include "system.h"

using UpdateCallBackFun = std::function<void()>;

// update组件，如果实体需要update，则包含这个组件，并写入UpdateFunction
class UpdateComponent : public Component<UpdateComponent>, public IAwakeFromPoolSystem<UpdateCallBackFun>
{
public:
	void Awake(UpdateCallBackFun fun) override;
    void BackToPool() override;
    void Update() const;

private:
    UpdateCallBackFun _function = nullptr;
};