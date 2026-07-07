#pragma once 

#include "disposable.h"

// 一次缓冲区扩容大小
#define ADDITIONAL_SIZE 1024 * 128

// 最大缓存空间
#define MAX_SIZE 1024 * 1024

class Buffer
{
public:
	virtual unsigned int GetEmptySize();
    // 追加缓冲区空间(参数dataLength是有效数据的大小，用于判断当_beginIndex == _endIndex时，是否为空还是为满)
	void ReAllocBuffer(unsigned int dataLength);

	unsigned int GetEndIndex() const
	{
		return _endIndex;
	}

	unsigned int GetBeginIndex() const
	{
		return _beginIndex;
	}

	unsigned int GetTotalSize() const
	{
		return _bufferSize;
	}

protected:
    char* _buffer = nullptr;        // 缓冲区
    unsigned int _beginIndex = 0;   // 可读起始位置
    unsigned int _endIndex = 0;     // 可写起始位置
    unsigned int _bufferSize = 0;   // 缓冲区大小
};