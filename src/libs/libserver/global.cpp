#include "global.h"

#if ENGINE_PLATFORM != PLATFORM_WIN32
#include <sys/time.h>
#endif

Global::Global(APP_TYPE appType, int appId)
{
    _appType = appType;
    _appId = appId;
    std::cout << "app type: " << GetAppName(appType) << " id: " << _appId << std::endl;
    UpdateTime();
}

uint64 Global::GenerateSN()
{
    // 65535 的容量在一个时间单位内足够应对任何突发情况，单个服务器很少会在一毫秒内生成上万个 ID。真溢出了也只是等到下一个时间 tick，因为有锁保护，不会重复
    std::lock_guard<std::mutex> guard(_mtx);
    // (40, 8, 16)
    const uint64 ret = (TimeTick >> 8 << 24) + (_serverId << 16) + _snTicket;
    _snTicket += 1;
    return ret;
}

APP_TYPE Global::GetCurAppType() const
{
    return _appType;
}

int Global::GetCurAppId() const
{
    return _appId;
}

void Global::UpdateTime()
{
#if ENGINE_PLATFORM != PLATFORM_WIN32
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    TimeTick = tv.tv_sec * 1000 + tv.tv_usec * 0.001;
#else
    auto timeValue = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    TimeTick = timeValue.time_since_epoch().count();
#endif
}