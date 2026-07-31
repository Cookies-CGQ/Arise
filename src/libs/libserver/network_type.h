#pragma once

// 网络连接类型
enum class NetworkType
{
    None = 0,
    // TCP
    TcpListen = 1 << 0,
    TcpConnector = 1 << 1,
    // HTTP
    HttpListen = 1 << 2,
    HttpConnector = 1 << 3,
};

inline const char* GetNetworkTypeName(const NetworkType appType)
{
    if (appType == NetworkType::TcpListen)
        return "TcpListen";
    else if (appType == NetworkType::TcpConnector)
        return "TcpConnector";
    else if (appType == NetworkType::HttpListen)
        return "HttpListen";
    else if (appType == NetworkType::HttpConnector)
        return "HttpConnector";
    else
        return "None";
}
