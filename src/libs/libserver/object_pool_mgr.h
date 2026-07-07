#pragma once

#include <mutex>
#include <list>
#include "singleton.h"
#include "disposable.h"
#include "object_pool_interface.h"

class DynamicObjectPoolMgr :public Singleton<DynamicObjectPoolMgr>, public IDisposable
{
public:
	// 添加对象池进入管理
	void AddPool(IDynamicObjectPool* pPool);
	// 调用所有对象池的Update
	void Update();
	// 释放自身资源--销毁所有的对象池
	void Dispose() override;
private:
	std::mutex _lock;
	std::list<IDynamicObjectPool*> _pools; // 对象池列表
};