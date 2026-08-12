#pragma once

#include "common.h"

class SnObject
{
public:
    // 初始化SN
    SnObject();
    // 获取SN
    uint64 GetSN() const;
    // 设置SN，SN == 0为哨兵值，表示未启动等
    void SetSN(uint64 sn);

protected:
    uint64 _sn;
};