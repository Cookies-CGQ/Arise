#pragma once

#include <mysql/mysql.h>
#include "libserver/common.h"
#include "libserver/yaml.h"

// 数据库基本操作
class MysqlBase
{
public:
    // 数据库连接初始化
    bool ConnectInit();
    // 断开数据库连接
    virtual void Disconnect();

    // 出错原因
    int CheckMysqlError() const;

    // 执行SQL
    bool Query(const char* sql, my_ulonglong& affected_rows);
    // 获取一行数据
    MYSQL_ROW Fetch() const;

    // 获取相应类型的数据
    static int GetInt(MYSQL_ROW row, int index);
    static unsigned int GetUint(MYSQL_ROW row, int index);
    static uint64 GetUint64(MYSQL_ROW row, int index);
    static char* GetString(MYSQL_ROW row, int index);
    int GetBlob(MYSQL_ROW row, int index, char* buf, unsigned long size) const;
    void GetBlob(MYSQL_ROW row, int index, std::string& protoStr) const;

protected:
    DBConfig* _pDbCfg = nullptr;            // 配置
    
    // MYSQL操作相关
    MYSQL* _pMysql = nullptr;              
    MYSQL_RES* _pMysqlRes = nullptr;        
    int _numFields = 0; // 列数
    MYSQL_FIELD* _pMysqlFields = nullptr;
};