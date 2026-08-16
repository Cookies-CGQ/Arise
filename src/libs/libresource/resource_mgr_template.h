#pragma once

#include "libserver/log4_help.h"
#include "libserver/res_path.h"
#include "libserver/global.h"
#include "resource_base.h"

// ResourceManagerTemplate<T>           // 通用加载器（模板，只管"怎么读"）
//     ├── ResourceBase                 // 数据基类（只管"一行长什么样"）
//     │     └── ResourceWorld          // 具体表的数据类（重写 GenStruct/Check）
//     └── ResourceWorldMgr             // 具体表的 Manager（只重写 AfterInit）

// CSV 配置表加载框架，模板参数T必须继承ResourceBase
template <class T>
class ResourceManagerTemplate
{
public:
    virtual ~ResourceManagerTemplate()
    {
        for(auto& one : _refs)
        {
            delete one.second;
        }

        _refs.clear();
    }
    // 初始化读取配置表数据到内存中
    bool Initialize(std::string table, ResPath *pResPath);
    // 给具体Manager做加载后的二次处理
    virtual bool AfterInit() { return true; }
    // 根据id获取一行数据
    T *GetResource(int id);

protected:
    // csv头部数据解析
    bool ParserHead(std::string line);
    // 一行 csv 数据 -> 内存对象
    bool LoadReference(std::string line);

protected:
    std::string _cvsName;              // 表名，对于 resource/xxx.csv
    std::map<std::string, int> _head;  // 表头映射
    std::map<int, T*> _refs;           // 对外查询用，id -> T*
};

template <class T>
bool ResourceManagerTemplate<T>::Initialize(std::string table, ResPath *pResPath)
{
    _cvsName = table;
    std::string path = pResPath->FindResPath("/resource");
    path = strutil::format("%s/%s.csv", path.c_str(), table.c_str());
    // 读取配置表
    std::ifstream reader(path.c_str(), std::ios::in);
    if (!reader)
    {
        LOG_ERROR("can't open file. " << path.c_str());
        return false;
    }

    LOG_DEBUG("load file. " << path.c_str());

    if (reader.eof())
    {
        LOG_ERROR("read head failed. stream is eof.");
        return false;
    }

    // 表头解析
    std::string line;
    std::getline(reader, line);
    std::transform(line.begin(), line.end(), line.begin(), ::tolower); // 转小写

    if (!ParserHead(line))
    {
        LOG_ERROR("parse head failed. " << path.c_str());
        return false;
    }

    // 循环读取每行数据
    while (true)
    {
        if (reader.eof())
            break;

        std::getline(reader, line);

        if (line.empty())
            continue;

        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        if (!LoadReference(line))
        {
            LOG_ERROR("loadreference failed. " << line.c_str());
        }
    }

    if (!AfterInit())
        return false;

    return true;
}

template <class T>
T *ResourceManagerTemplate<T>::GetResource(int id)
{
    auto iter = _refs.find(id);
    if (iter == _refs.end())
        return nullptr;

    return iter->second;
}

template <class T>
bool ResourceManagerTemplate<T>::ParserHead(std::string line)
{
    if (line.empty())
        return false;

    std::vector<std::string> propertyList = ResourceBase::ParserLine(line);

    for (size_t i = 0; i < propertyList.size(); i++)
    {
        _head.insert(std::make_pair(propertyList[i], i));
    }

    return true;
}

template <class T>
bool ResourceManagerTemplate<T>::LoadReference(std::string line)
{
    auto pT = new T(_head);
    if (pT->LoadProperty(line) && pT->Check())
    {
        _refs.insert(std::make_pair(pT->GetId(), pT));
        return true;
    }
    else
    {
        delete pT;
    }

    return false;
}