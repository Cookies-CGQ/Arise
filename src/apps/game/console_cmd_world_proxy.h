#pragma once

#include "libserver/console.h"

// 用于game进程控制台输入 proxy -all 打印出所有world_proxy的在线人数
class ConsoleCmdWorldProxy :public ConsoleCmd
{
public:
	void RegisterHandler() override;
	void HandleHelp() override;

protected:
	void HandleShowAllWorld(std::vector<std::string>& params);
};