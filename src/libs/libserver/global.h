#pragma once

#include <mutex>
#include "common.h"
#include "util_time.h"
#include "app_type.h"
#include "singleton.h"

class Global: public Singleton<Global>
{
public:
    Global(APP_TYPE appType, int appId);
    // 更新全局时间
    void UpdateTime();

    // 实体sn获取服务ID
    static int GetAppIdFromSN(uint64 sn);
    // 获取SN，SN = 时间 + 服务器ID + ticket，共64位
    uint64 GenerateSN();
    // 生成UUID
    static std::string GenerateUUID();

    // 当前服务类型
    APP_TYPE GetCurAppType() const;
    // 当前同类服务ID
    int GetCurAppId() const;

    // 当前日期在一年中的序号
    int YearDay;
    // 全局时间 -- 毫秒
    timeutil::Time TimeTick;

    // 控制服务器停止
    bool IsStop = false;

private:
    std::mutex _mtx;
    unsigned int _snTicket = 1;  // 全局SN生成递增序列
    APP_TYPE _appType;           // 当前进程服务类型
    int _appId = 0;              // 同类服务ID
};