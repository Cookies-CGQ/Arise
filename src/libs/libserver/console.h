#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include "system.h"
#include "disposable.h"
#include "entity.h"

// 控制台命令系统
// 额外的一个线程当作控制台输入线程
// 命令格式：采用二级命令：<一级命令> <二级命令> [参数...]
// 例如：login -a ayanami，login是一级命令为登录模块，二级命令选择模块的具体操作，ayanami表示参数

#define ConsoleMaxBuffer 512

typedef std::function<void(std::vector<std::string>&)> HandleConsole;

// 管理一个一级命令模块
class ConsoleCmd : public IDisposable
{
public:
    // 注册模块支持的二级命令
	virtual void RegisterHandler() = 0;
	// 显示模块帮助
    virtual void HandleHelp() = 0;

	void Dispose() override;
    // 分发二级命令
	void Process(std::vector<std::string>& params);

protected:
    // 注册二级命令
	void OnRegisterHandler(std::string key, HandleConsole handler);
    // 检查业务参数数量
	static bool CheckParamCnt(std::vector<std::string>& params, const size_t count);

private:
	std::map<std::string, HandleConsole> _handles; // 二级命令集合
};

// 控制台管理器
class Console : public Entity<Console>, public IAwakeSystem<>
{
public:
    // 初始化
	void Awake() override;
	void BackToPool() override;

    // 帧函数 -- 执行命令
	void Update();

    // 注册命令
	template<class T>
	void Register(std::string cmd);

protected:
	std::map<std::string, ConsoleCmd*> _handles; // 保存一级命令

	std::mutex _lock;
	std::thread _thread; // 控制台输入线程，用于等待控制台输入（没有输入回车会阻塞），输入线程只负责读取和排队，真正的命令处理函数在服务器主更新线程执行。
	std::queue<std::string> _commands; // 待执行命令队列

	bool _isRun = true;
};

template<class T>
void Console::Register(std::string cmd)
{
	T* pObj = new T();	
	pObj->RegisterHandler(); // 二级命令
	this->_handles[cmd] = pObj; 
}

