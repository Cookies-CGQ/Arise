#pragma once

#include <mutex>
#include <map>
#include "libserver/component.h"
#include "login_obj.h"

// 负责管理所有的LoginObj实例
class LoginObjMgr :public Component<LoginObjMgr>
{
public:
    // 客户端连接登陆服并提交账号密码后，创建LoginObj并加入管理
    LoginObj* AddPlayer(NetworkIdentify* pIdentify, std::string account, std::string password);
    // 删除LoginObj对象
	void RemovePlayer(SOCKET socket);

    // 获取LoginObj对象
	LoginObj* GetPlayer(SOCKET socket);
	LoginObj* GetPlayer(std::string account);

	// 账号个数
    size_t Count();

    // 归还对象池之前先释放资源
	void BackToPool() override;

private:
	// 正在验证的账号
	// <socket, loginPlayer*>
	std::map<SOCKET, LoginObj*> _players;        // 通过socket查找对象
	// <account, socket>
	std::map <std::string, SOCKET> _accounts;    // 通过账号查找socket
};

