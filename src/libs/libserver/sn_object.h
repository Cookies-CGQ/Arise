#pragma once

#include "common.h"

class SnObject
{
public:
    // 初始化SN
    SnObject();
    // 指定设置SN
    SnObject(uint64 sn);
    // 获取SN
    uint64 GetSN() const;
    // 重新设置SN
    void ResetSN();
    // 重新设置指定SN
    void ResetSN(uint64 sn);    

protected:
    uint64 _sn;
};