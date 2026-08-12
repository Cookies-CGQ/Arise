#pragma once

#include <log4cplus/streams.h>
#include <ostream>
#include "common.h"
#include "network_type.h"

// 网络连接标识系统：网络连接socket + 业务标签

struct SocketKey
{
    SocketKey(SOCKET socket, NetworkType netType);
	// 清除
    void Clean();
    // 拷贝指定数据
    void CopyFrom(SocketKey* pSocketKey);

	// !=重载
    bool operator != (const SocketKey other) const
    {
        return (Socket != other.Socket) || (NetType != other.NetType);
    };

	// ==重载
    bool operator == (const SocketKey other)
    {
        return (Socket == other.Socket) && (NetType == other.NetType);
    };

	SOCKET Socket;          // socket
    NetworkType NetType;    // 网络类型

    static SocketKey None;  // 静态空哨兵
};

// 标签类型
enum class TagType
{
    None = Proto::TagType::TagTypeNone,         // 空标签
    Account = Proto::TagType::TagTypeAccount,   // 账号标签
    App = Proto::TagType::TagTypeApp,           // 服务标签
    Entity = Proto::TagType::TagTypeEntity,     // Entity标签
    ToWorld = Proto::TagType::TagTypeToWorld,   // 目标世界标签
    Player = Proto::TagType::TagTypePlayer,     // 玩家标签
};

// 标签类型是否是字符串
inline bool IsTagTypeStr(const TagType iType)
{
    return iType == TagType::Account;
}

// 获取标签名
inline const char* GetTagTypeName(const TagType iType)
{
    if (iType == TagType::Account)
        return "Account";
    else if (iType == TagType::App)
        return "App";
    else if (iType == TagType::Entity)
        return "world";
    else if (iType == TagType::Player)
        return "player";
    else
        return "None";
}

// 标签值
struct TagValue
{
    std::string KeyStr = "";
    uint64 KeyInt64 = 0;

    bool operator != (const TagValue& other) const
    {
        return (KeyStr != other.KeyStr) || (KeyInt64 != other.KeyInt64);
    };

    bool operator == (const TagValue& other) const
    {
        return (KeyStr == other.KeyStr) && (KeyInt64 == other.KeyInt64);
    };
};

// 标签
struct TagKey
{
public:
    void Clear();

    std::map<TagType, TagValue>* GetTags() 
    { 
        return &_tags; 
    }

    // 添加标签
    void AddTag(TagType tagType, std::string value);
    void AddTag(TagType tagType, uint64 value);
    void AddTag(TagType tagType, TagValue value);

    // 根据标签类型获取标签值
    TagValue* GetTagValue(TagType tagType);
    // 拷贝
    void CopyFrom(TagKey* pNetIdentify);
    // 比较
    bool CompareTags(TagKey* pIdentify);

protected:
    static bool CompareTags(TagKey* pA, TagKey* pB, TagType tagtype);

protected:
    std::map<TagType, TagValue> _tags; // 多标签
};

struct NetIdentify
{
public:
    NetIdentify() = default;
    ~NetIdentify()
    {
        _socketKey.Clear();
        _tagKey.Clear();
    }

    SocketKey* GetSocketKey() { return &_socketKey; }
    TagKey* GetTagKey() { return &_tagKey; }

protected:
    SocketKey _socketKey{  INVALID_SOCKET, NetworkType::None };
    TagKey _tagKey;
};

std::ostream& operator <<(std::ostream& os, TagKey* pTagKey);
std::ostream& operator <<(std::ostream& os, NetIdentify* pIdentify);

#if ENGINE_PLATFORM == PLATFORM_WIN32
log4cplus::tostream& operator <<(log4cplus::tostream& os, TagKey* pTagKey);
log4cplus::tostream& operator <<(log4cplus::tostream& os, NetIdentify* pIdentify);
#endif