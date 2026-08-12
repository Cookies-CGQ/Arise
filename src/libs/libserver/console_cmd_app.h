#pragma once

#include "console.h"

// 查看App整体信息
class ConsoleCmdApp :public ConsoleCmd
{
public:
    void RegisterHandler() override;
    void HandleHelp() override;

private:
    void HandleAppInfo(std::vector<std::string>& params);
};