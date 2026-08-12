#pragma once

#include "console.h"

// 用于追踪和查看游戏中各种对象的运行时信息
class ConsoleCmdTrace :public ConsoleCmd
{
public:
    void RegisterHandler() override;
    void HandleHelp() override;

private:
    // 显示指定 socket 的连接信息
    void HandleConnect(std::vector<std::string>& params);
    // 追踪指定 socket 相关的所有网络包
    void HandlePacket(std::vector<std::string>& params);
    // 追踪指定 socket 相关的玩家信息
    void HandlePlayer(std::vector<std::string>& params);
    // 按账号名追踪该账号的所有信息
    void HandleAccount(std::vector<std::string>& params);
    // 显示时间相关的追踪信息
    void HandleTime(std::vector<std::string>& params);
    // 清空所有追踪数据
    void HandleClean(std::vector<std::string>& params);
};