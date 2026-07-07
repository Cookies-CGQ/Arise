#pragma once 

#include "common.h"
#include "global.h"

class SnObject
{
public:
    virtual ~SnObject() {}

    SnObject()
    {
        _sn = Global::Instance()->GenerateSN();
    }

    SnObject(uint64 sn)
    {
        _sn = sn;
    }

    // 获取SN码
    uint64 GetSN() const
    {
        return _sn;
    }
    

    // 重置SN码（对象池复用对象时调用）
    void ResetSN()
    {
        _sn = Global::Instance()->GenerateSN();
    }
protected:
    uint64 _sn;
};