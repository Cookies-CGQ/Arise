#pragma once

#include "common.h"

class SnObject
{
public:
    // 初始化SN
    SnObject();
    // 获取SN
    uint64 GetSN() const;
    // 
    void ResetSN(bool isClean = false);

protected:
    uint64 _sn;
};