#pragma once
#include "libserver/console.h"
#include <list>

class RobotConsoleLogin :public ConsoleCmd
{
public:
	void RegisterHandler() override;
	void HandleHelp() override;

private:
    // 子命令：单个登录
	void HandleLogin(std::vector<std::string>& params);
	// 子命令：批量登录
    void HandleLoginEx(std::vector<std::string>& params) const;
	// 子命令：清理
    void HandleLoginClean(std::vector<std::string>& params);

private:
	std::list<uint64> _threads;
};