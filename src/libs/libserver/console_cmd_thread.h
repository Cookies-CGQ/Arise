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
    // 查看entity信息
	void HandleEntity(std::vector<std::string>& params);
    // 查看pool信息
    void HandlePool(std::vector<std::string>& params);
    // 查看connect信息
    void HandleConnect(std::vector<std::string>& params);
};