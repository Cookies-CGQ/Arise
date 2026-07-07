#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "thread_obj.h"

#define ConsoleMaxBuffer 512

// 控制台命令处理函数。
// 参数是拆分后的字符串向量，第一个元素为命令名，后续为参数。
typedef std::function<void(std::vector<std::string>&)> HandleConsole;

// 一组二级控制台命令的基类。
// 典型输入格式为：
//   <cmd> <sub_cmd> [arg1] [arg2]
// 其中 <cmd> 由 Console::_handles 定位到某个 ConsoleCmd 对象，
// <sub_cmd> 再由 ConsoleCmd::_handles 定位到具体处理函数。
class ConsoleCmd : public IDisposable
{
public:
	// 子类在这里调用 OnRegisterHandler() 注册自身支持的二级命令。
	virtual void RegisterHandler() = 0;
	void Dispose() override;
	// 按参数数组分发命令。params[0] 是一级命令，params[1] 是二级命令。
	void Process(std::vector<std::string>& params);

protected:
	// 注册二级命令和对应处理函数；同名 key 会覆盖旧处理函数。
	void OnRegisterHandler(std::string key, HandleConsole handler);
    static bool CheckParamCnt(std::vector<std::string>& params, const size_t count);

private:
	// key: 二级命令名；value: 对应的处理函数。
	std::map<std::string, HandleConsole> _handles;
};

// 控制台输入入口。
// Init() 创建一个后台线程阻塞读取 stdin；Update() 在业务线程中取出输入并分发。
class Console : public ThreadObject
{
public:
	bool Init() override;
	void RegisterMsgFunction() override;
	void Update() override;
	void Dispose() override;

protected:
	// 注册一级命令。
	// T 必须继承 ConsoleCmd，且默认构造后能在 RegisterHandler() 中注册二级命令。
	template<class T>
	void Register(std::string cmd);

protected:
	// key: 一级命令名；value: 负责该一级命令下所有二级命令的处理对象。
	std::map<std::string, std::shared_ptr<ConsoleCmd>> _handles;

	// 后台输入线程与 Update() 之间共享命令队列，因此入队/出队都需要加锁。
	std::mutex _lock;
	std::thread _thread;
	std::queue<std::string> _commands;
    bool _isRun{ true };
};

template<class T>
void Console::Register(std::string cmd)
{
	std::shared_ptr<T> pObj = std::make_shared<T>();
	// 先让命令对象完成自身二级命令注册，再挂到 Console 的一级命令表中。
	pObj->RegisterHandler();
	this->_handles.insert(std::make_pair(cmd, pObj));
}
