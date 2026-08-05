#pragma once
#include <map>
#include <set>
#include "component.h"
#include "system.h"
#include "common.h"

#if true
#define LOG_TRACE_COMPONENT_OPEN 1
#endif

enum class TraceType
{
    Packet = 0,       // packet包追踪
    Connector = 1,    // 网络连接追踪
    Player = 2,       // 玩家追踪
};

class TraceDetail
{
public:
    // 追加一条带时间戳的记录
    void Trace(const std::string& trace);
    // 遍历打印所有记录
    void Show() const;

private:
    std::list<std::string> _details;
};

// 调试追踪系统 -- 单例组件
class TraceComponent :public Component<TraceComponent>, public IAwakeSystem<>
{
public:
    void Awake() override;
    void BackToPool() override;

    // 账户socket追踪 -- 添加socket
    void TraceAccount(std::string account, SOCKET socket);
    // 打印指定账户的socket集合
    void ShowAccount(const std::string& account);

    void Trace(TraceType iType, int key, const std::string& trace);
    void Show(TraceType iType, int key);

    // 清空
    void Clean();

private:
    std::mutex _lock;
    // 三级映射 -- TraceType : key : TraceDetail
    std::map <TraceType, std::map<int, TraceDetail>> _traces;

    // player对应的socket，一个账户可能光联多个socket，记录下来用于调试时查找某个玩家的网络连接
    std::map<std::string, std::set<SOCKET>> _accounts;
};