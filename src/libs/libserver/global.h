#pragma once 

#include "common.h"
#include "util_time.h"
#include "singleton.h"

#include <mutex>

class Global : public Singleton<Global>
{
public:
    // 生成SN码，SN = 64位，时间 + 服务器ID + ticket
    uint64 GenerateSN();  

    // 用于控制全局的时间，由主服务控制更新，工作线程的时间都来源于这里，已达到时间的统一
	int YearDay;
	timeutil::Time TimeTick;

    // 系统是否停止
    bool IsStop {false};

private:
    std::mutex _mtx;
    unsigned int _snTicket {1};
    unsigned int _serverId {0};
};