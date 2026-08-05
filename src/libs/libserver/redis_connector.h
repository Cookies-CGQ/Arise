#pragma once

#include <hiredis/hiredis.h>
#include "entity.h"
#include "system.h"

// 连接redis，提供redis基础操作
class RedisConnector :public Entity<RedisConnector>, public IAwakeSystem<>
{
public:
	void Awake() override;	
	void BackToPool() override;

    // 连接redis
	bool Connect();
    // 断开redis
	bool Disconnect();

protected:
    // 注册消息处理函数，由子类实现
	virtual void RegisterMsgFunction() = 0;

    // 检测redis连接状态
	bool Ping() const;
    // 定时器 -- 定时检测redis连接状态，如果断线则自动重连
	void CheckPing();

    // set操作
	bool Setex(std::string key, std::string value, int timeout) const;
	bool Setex(std::string key, uint64 value, int timeout) const;
	bool SetnxExpire(std::string key, int value, int timeout) const;

    // 删除指定key
	void Delete(std::string key) const;

    // get操作
	int GetInt(std::string key) const;
	std::string GetString(std::string key) const;
	uint64 GetUint64(std::string key) const;

private:
	bool Setex(std::string command) const;

protected:
	redisContext * _pRedisContext = nullptr; // redis 句柄
	std::string _ip = "127.0.0.1"; // redis server ip
	int _port = 6379;              // redis server port
};