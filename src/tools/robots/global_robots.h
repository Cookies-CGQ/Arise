#pragma once
#include <mutex>
#include "libserver/singleton.h"

// 线程安全计数器，robot计数
class GlobalRobots : public Singleton<GlobalRobots>
{
public:
	// 每收到状态同步时读取，判断是否所有Robot都上报了
	size_t GetRobotsCount();
	// 批量创建完成时写入总数
	void SetRobotsCount(size_t count);
	// 清理时归零
	void CleanRobotsCount();

private:
	std::mutex _mtx;
	size_t _robotsCnt = 0;
};