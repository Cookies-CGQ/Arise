#include "resource_base.h"
#include "libserver/log4_help.h"
#include "libserver/util_string.h"

std::vector<std::string> ResourceBase::ParserLine(std::string line)
{
    line = strutil::trim(line);

    std::vector<std::string> propertyList;

    // blank line (or whitespace only) has no fields
    if (line.empty())
        return propertyList;

    while (true)
    {
        if (line.at(0) == '\"')
        {
            // quoted field: ends at the closing quote
            line.erase(0, 1);
            const std::size_t index = line.find('\"');

            if (index == std::string::npos)
            {
                // malformed line missing its closing quote: take the rest as-is
                propertyList.push_back(line);
                break;
            }

            propertyList.push_back(line.substr(0, index));
            line.erase(0, index + 1);

            if (line.empty())
                break;

            // the char right after the closing quote is the field separator
            if (line.at(0) == ',')
            {
                line.erase(0, 1);
                if (line.empty())
                    break;
            }
        }
        else
        {
            // normal field: ends at the next comma
            const std::size_t index = line.find(',');

            if (index == std::string::npos)
            {
                // last field
                propertyList.push_back(line);
                break;
            }

            // delimiter at position 0 means an empty field
            propertyList.push_back(line.substr(0, index));
            line.erase(0, index + 1);

            if (line.empty())
                break;
        }
    }

    return propertyList;
}

bool ResourceBase::LoadProperty(const std::string line)
{
    std::vector<std::string> propertyList = ParserLine(line);

    // csv 行最后可能一个看不见的字符，所以propertyList一定是大于_head的
    if (propertyList.size() < _head.size())
    {
        LOG_ERROR("LoadProperty failed. " << "line size:" << propertyList.size() << " head size:" << _head.size() << " \t" << line.c_str());
        return false;
    }

    for (size_t i = 0; i < propertyList.size(); i++)
    {
        _props.push_back(strutil::trim(propertyList[i]));
    }

    _id = std::stoi(_props[0]);

    GenStruct();
    return true;
}

void ResourceBase::DebugHead() const
{
    for (auto one : _head)
    {
        LOG_DEBUG("head name:[" << one.first.c_str() << "] head index:" << one.second);
    }
}

bool ResourceBase::GetBool(std::string name)
{
    const auto iter = _head.find(name);
    if (iter == _head.end())
    {
        LOG_ERROR("GetInt Failed. id:" << _id << " name:[" << name.c_str() << "]");
        DebugHead();
        return false;
    }

    return std::stoi(_props[iter->second]) == 1;
}

int ResourceBase::GetInt(std::string name)
{
    const auto iter = _head.find(name);
    if (iter == _head.end())
    {
        LOG_ERROR("GetInt Failed. id:" << _id << " name:[" << name.c_str() << "]");
        DebugHead();
        return 0;
    }

    return std::stoi(_props[iter->second]);
}

std::string ResourceBase::GetString(std::string name)
{
    const auto iter = _head.find(name);
    if (iter == _head.end())
    {
        LOG_ERROR("GetString Failed. id:" << _id << " name:[" << name.c_str() << "]");
        DebugHead();
        return "";
    }

    return _props[iter->second];
}