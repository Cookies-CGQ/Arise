#pragma once

#include <yaml-cpp/yaml.h>
#include "common.h"
#include "app_type.h"
#include "component.h"
#include "system.h"

// 配置结构继承层次
// YamlConfig                      (虚基类，带虚析构)
// └── AppConfig                   (线程池配置)
//     ├── CommonConfig            (IP / 端口，单实例 App 列表)
//     │   ├── AppListForOneConfig (单个 App 实例的完整端点)
//     │   └── DBMgrConfig         (数据库连接池配置)
//     ├── AppListConfig           (多实例 App 列表)
//     │   └── LoginConfig         (登录服特有 URL)
//     └── RobotConfig             (用于压测机器人)
// DBConfig                        (独立结构，不参与继承)

// 多态根类型
struct YamlConfig
{
    virtual ~YamlConfig()
    {

    }
};

struct AppConfig: public YamlConfig
{
    int LogicThreadNum = 0;    // 逻辑线程个数
    int MysqlThreadNum = 0;    // mysql线程个数
    int ListenThreadNum = 1;   // 监听线程个数
    int ConnectThreadNum = 1;  // 连接线程个数
};

struct CommonConfig: public AppConfig
{
    std::string Ip = "127.0.0.1";
    int Port = 6661;
    int HttpPort = 5051;
};

struct AppListForOneConfig : public CommonConfig
{
    int Id = 0;
};

// 同一个服务存在多个实例
struct AppListConfig : public AppConfig
{
    std::vector<AppListForOneConfig> Apps;

    AppListForOneConfig* GetOne(int id)
    {
        for (decltype(Apps.size()) index = 0; index < Apps.size(); index++)
        {
            if (Apps[index].Id == id)
                return &Apps[index];
        }

        return nullptr;
    }
};

struct LoginConfig : public AppListConfig
{
    std::string UrlLogin;  // url
    std::string UrlMethod; // method
};

// 数据库连接数据
struct DBConfig
{
    std::string DBType;
    std::string Ip = "127.0.0.1";
    int Port = 3306;
    std::string User = "";
    std::string Password = "";
    std::string DatabaseName = "";
    std::string CharacterSet = "";
    std::string Collation = "";
};

struct DBMgrConfig : public CommonConfig
{
    static std::string DBTypeMysql;
    static std::string DBTypeRedis;

    std::vector<DBConfig> DBs;

    DBConfig* GetDBConfig(const std::string dbType)
    {
        for (decltype(DBs.size()) index = 0; index < DBs.size(); index++)
        {
            if (DBs[index].DBType != dbType)
                continue;

            return &DBs[index];
        }

        return nullptr;
    }
};

struct RobotConfig : public AppConfig
{

};

class Yaml : public Component<Yaml>, public IAwakeSystem<>
{
public:
    void Awake();
    void BackToPool();

    YamlConfig* GetConfig(APP_TYPE appType);
    // 如果配置是AppListConfig，按appId找对应实例；否则尝试转成CommonConfig返回唯一IP/端口。
    CommonConfig* GetIPEndPoint(APP_TYPE appType, int appId = 0);

private:
    void LoadConfig(APP_TYPE appType, YAML::Node& config);
    DBConfig LoadDbConfig(YAML::Node node) const;
    void LoadAppList(AppListConfig* pConfig, YAML::Node node) const;

private:
    std::map<APP_TYPE, YamlConfig*> _configs; // 配置注册表，服务类型：配置文件内存对象
};