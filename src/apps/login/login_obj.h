#pragma once 

#include <string>
#include "libserver/socket_object.h"
#include "libserver/common.h"

class LoginObj : public ISocketObject
{
public:
    LoginObj(SOCKET socketInfo, std::string account, std::string password);
    std::string GetAccount() const;
    SOCKET GetSocket() override;

private:
    std::string _account;  // 用户账号
    std::string _password; // 用户密码
    SOCKET _socket;        
};