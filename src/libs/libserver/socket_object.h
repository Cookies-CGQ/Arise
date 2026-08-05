#pragma once

#include <log4cplus/streams.h>
#include <ostream>
#include "common.h"
#include "network_type.h"

struct SocketKey
{
    SocketKey(SOCKET socket, NetworkType netType);
	// 清除
    void Clean();

	// !=重载
    bool operator != (const SocketKey other)
    {
        if (Socket != other.Socket)
            return false;

        if (NetType != other.NetType)
            return false;

        return true;
    };

	// ==重载
    bool operator == (const SocketKey other)
    {
        return (Socket == other.Socket) && (NetType == other.NetType);
    };

	SOCKET Socket;
    NetworkType NetType;

    static SocketKey None;
};

enum class ObjectKeyType
{
    None = Proto::NetworkObjectKeyType::ObjectKeyTypeNone,
    Account = Proto::NetworkObjectKeyType::ObjectKeyTypeAccount,
    App = Proto::NetworkObjectKeyType::ObjectKeyTypeApp,
};

inline const char* GetConnectKeyTypeName(const ObjectKeyType iType)
{
    if (iType == ObjectKeyType::Account)
        return "Account";
    else if (iType == ObjectKeyType::App)
        return "App";
    else
        return "None";
}

struct ObjectKeyValue
{
    void Clean();

    bool operator != (const ObjectKeyValue other)
    {
        if (KeyInt64 != other.KeyInt64)
            return false;

        if (KeyStr != other.KeyStr)
            return false;

        return true;
    };

    bool operator == (const ObjectKeyValue other)
    {
        return (KeyInt64 == other.KeyInt64) && (KeyStr == other.KeyStr);
    };

	uint64 KeyInt64 = 0;           // 用于标识Account
    std::string KeyStr = "";       // 用于标识App
};

struct ObjectKey
{
	// proto -> ObjectKey
    void ParseFromProto(Proto::NetworkObjectKey protoKey);
    // ObjectKey -> proto
	void SerializeToProto(Proto::NetworkObjectKey* pProto) const;
    // 清除
	void Clean();

    bool operator != (const ObjectKey other)
    {
        if (KeyType != other.KeyType)
            return false;

        if (KeyValue != other.KeyValue)
            return false;

        return true;
    };

    bool operator == (const ObjectKey other)
    {
        return (KeyType == other.KeyType) && (KeyValue == other.KeyValue);
    };

    ObjectKeyType KeyType = ObjectKeyType::None;
    ObjectKeyValue KeyValue { 0, "" };
};

// 网络与对象标识
struct NetworkIdentify
{
public:
    NetworkIdentify() = default;
    NetworkIdentify(SocketKey socketKey, ObjectKey objKey);

    virtual ~NetworkIdentify() = default;
    SocketKey GetSocketKey() const { return _socketKey; }
    ObjectKey GetObjectKey() const { return _objKey; }

protected:
    SocketKey _socketKey { INVALID_SOCKET, NetworkType::None };
    ObjectKey _objKey { ObjectKeyType::None , {0, ""} };
};

// << 重载，方便输出NetworkIdentify信息
std::ostream& operator <<(std::ostream& os, NetworkIdentify* pIdentify);