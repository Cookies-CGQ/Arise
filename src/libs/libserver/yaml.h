#pragma once

#include <yaml-cpp/yaml.h>
#include "common.h"
#include "app_type.h"
#include "component.h"
#include "system.h"

//   YamlConfig
//   └── AppConfig                        线程数量
//       ├── CommonConfig                 IP + Port
//       │   ├── AppListForOneConfig      单个应用实例 + Id
//       │   ├── LoginConfig              登录验证 URL
//       │   └── DBMgrConfig              数据库配置列表
//       ├── AppListConfig                多实例应用列表
//       └── RobotConfig                  只有线程配置
//   DBConfig                             独立的数据库连接记录

// 多态根类型
struct YamlConfig
{
    virtual ~YamlConfig()
    {

    }
};

struct AppConfig: public YamlConfig
{
    int LogicThreadNum = 0;  // 逻辑线程个数
    int MysqlThreadNum = 0;  // mysql线程个数
};

struct CommonConfig: public AppConfig
{
    std::string Ip = "127.0.0.1";
    int Port = 6666;
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

struct LoginConfig : public CommonConfig
{
    std::string UrlLogin;
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

private:
    std::map<APP_TYPE, YamlConfig*> _configs; // 配置注册表，服务类型：配置文件内存对象
};
