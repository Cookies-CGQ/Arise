#pragma once
#include "common.h"

class ISocketObject
{
public:
	virtual ~ISocketObject()
	{

	}
    // 获取Socket
	virtual SOCKET GetSocket() = 0;
};
