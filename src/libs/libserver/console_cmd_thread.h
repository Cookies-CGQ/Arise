#pragma once

#include "console.h"

class ConsoleCmdThread :public ConsoleCmd
{
public:
    // 注册二级命令
	void RegisterHandler() override;
	// 帮助
    void HandleHelp() override;

private:
	void HandleEntity(std::vector<std::string>& params);
    void HandlePool(std::vector<std::string>& params);
    void HandleConnect(std::vector<std::string>& params);
};