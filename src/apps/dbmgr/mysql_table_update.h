#pragma once

#include <functional>
#include "mysql_base.h"
#include "libserver/singleton.h"

// Mysql数据库表自动版本管理和迁移
class MysqlTableUpdate :public MysqlBase, public Singleton<MysqlTableUpdate>
{
public:
    MysqlTableUpdate();
    virtual ~MysqlTableUpdate();

    // 检查是否需要更新
    void Check();

private:
    // 创建数据库/表
    bool CreateDatabaseIfNotExist();

    // 检查DB数据，更新到最新版本
    bool UpdateToVersion();

    // 版本更新函数
    bool Update00();
    // 可添加其他版本更新函数
    // ......

private:
    // update
    typedef std::function<bool(void)> OnUpdate;
    std::vector<OnUpdate> _update_func; // 版本更新函数

    int const _version = 0; // 代码当前版本
};