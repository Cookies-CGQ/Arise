#pragma once

#include <vector>
#include <map>
#include <string>

// 配置表单行数据基类
class ResourceBase
{
public:
	virtual ~ResourceBase() = default;
	
	explicit ResourceBase(std::map<std::string, int>& head)
		: _id(0), _head(head) 
	{

	}

	int GetId() const { return _id; }

	// 行级入口
	bool LoadProperty(const std::string line);

	// 检查本行数据是否合法
	virtual bool Check() = 0;

	// CSV 解析器
	static std::vector<std::string> ParserLine(std::string line);

protected:

	// 生成内存结构
	virtual void GenStruct() = 0;

	// 按列名取值
	std::string GetString(std::string name);
	bool GetBool(std::string name);
	int GetInt(std::string name);

	// 输出头部信息
	void DebugHead() const;

private:
	int _id;                            // 第一列，主键
	std::map<std::string, int>& _head;  // 表头（引用）：列名 -> 列索引
	std::vector<std::string> _props;    // 本行的所有字段值
};