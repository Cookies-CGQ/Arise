#pragma once

#include "console.h"

// 查询线程效率信息
class ConsoleCmdEfficiency :public ConsoleCmd
{
public:
	void RegisterHandler() override;
	void HandleHelp() override;

private:
	void HandleThread(std::vector<std::string>& params);
};