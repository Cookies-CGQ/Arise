#pragma once 

#include <mutex>
#include <map>
#include "login_obj.h"
#include "libserver/disposable.h"

class LoginObjMgr : public IDisposable
{
public:
    // 添加Player
	void AddPlayer(SOCKET socket, std::string account, std::string password);
	// 删除Player
    void RemovePlayer(SOCKET socket);
	// 通过socket获取Player
    LoginObj* GetPlayer(SOCKET socket);
	// 通过account获取Player
    LoginObj* GetPlayer(std::string account);
	// 释放自身资源
    void Dispose() override;

private:
    // 正在验证的账号; <socket, LoginObj*>
    std::map<SOCKET, LoginObj*> _players;
    // <account, socket>，账号和socket的映射
    std::map<std::string, SOCKET> _accounts;
};