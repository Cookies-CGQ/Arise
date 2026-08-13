#pragma once

#include <mysql/mysql.h>
#include <vector>
#include "libserver/common.h"
#include "libserver/entity.h"
#include "libserver/system.h"
#include "libserver/socket_object.h"
#include "mysql_base.h"

class Packet;

// 数据库预编译语句键值枚举
enum class DatabaseStmtKey
{
	Create,
	Save,
};

// 数据库预编译语句结构体
struct DatabaseStmt
{
	MYSQL_STMT *stmt = nullptr;
	MYSQL_BIND *bind = nullptr;
	char *bind_buffer = nullptr;
	int bind_index;
	int bind_buffer_index;

    // 资源清理
	void Close()
	{
        if (bind != nullptr) 
        {
            delete[] bind;
            bind = nullptr;
        }

        if (bind_buffer != nullptr) 
        {
            delete[] bind_buffer;
            bind_buffer = nullptr;
        }

        if (stmt != nullptr) 
        {
            mysql_stmt_close(stmt);
            stmt = nullptr;
        }
	}
};

#define MAX_BIND_BUFFER    40960
#define MAX_BIND_STR       30000

class MysqlConnector : public MysqlBase, public Entity<MysqlConnector>, public IAwakeSystem<>
{
public:
    // 初始化
	void Awake() override;
	// 归还对象池之前资源清理
    void BackToPool() override;
    // 将协议消息类型与对应的处理函数进行绑定
    void InitMessageComponent();
    // 建立MySQL数据库连接
	bool Connect();
    // 断开MySQL连接并清理资源
	void Disconnect() override;

private:
    // 重新连接数据库，当检测到连接断开时，尝试重新建立MySQL连接
	void ReConnect();
    // 心跳检测，定期通过mysql_ping检查连接是否存活，断开则触发重连
	void CheckPing();

    // 初始化所有预编译语句，将业务SQL模板预编译并存入_mapStmt映射表，供后续快速执行
	void InitStmts();
    // 清理所有预编译语句，遍历_mapStmt，逐个调用DatabaseStmt::Close()释放资源
	void CleanStmts();
    // 获取预编译语句
	DatabaseStmt* GetStmt(DatabaseStmtKey stmtKey);

    // 创建一条预编译语句
	DatabaseStmt* CreateStmt(const char *sql) const;
    // 清除预编译语句的所有参数绑定
	void ClearStmtParam(DatabaseStmt* stmt);

    // 向预编译语句添加参数
	static void AddParamStr(DatabaseStmt* stmt, const char* value);
	static void AddParamInt(DatabaseStmt* stmt, int val);
	static void AddParamUint64(DatabaseStmt* stmt, uint64 val);
	static void AddParamBlob(DatabaseStmt* stmt, void *val, int size);

    // 执行预编译指令
	bool ExecuteStmt(DatabaseStmt* stmt, my_ulonglong& affected_rows);
	bool ExecuteStmt(DatabaseStmt* stmt);


	// 协议处理函数 -- 处理“查询玩家角色列表”协议
	void HandleQueryPlayerList(Packet* pPacket);
    void QueryPlayerList(std::string account, NetworkIdentify* pIdentify);

    // 协议处理函数 -- 处理“查询单个玩家详情”协议
	void HandleQueryPlayer(Packet* pPacket);

    // 协议处理函数 -- 处理“创建新玩家”协议
	void HandleCreatePlayer(Packet* pPacket);
    
    // 协议处理函数 -- 处理“保存玩家数据”协议
	void HandleSavePlayer(Packet* pPacket);
	bool OnSavePlayer(DatabaseStmt* stmtSave, Proto::Player& protoPlayer);

protected:
	// stmt
	std::map<DatabaseStmtKey, DatabaseStmt*> _mapStmt; // 预编译语句映射表

	bool _isRunning = false; // 连接运行状态标志
};

