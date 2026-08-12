#pragma once

// 单次追加大小
#define ADDITIONAL_SIZE 1024 * 128 // 128KB

// 最大缓冲
#define MAX_SIZE 1024 * 1024       // 1M

class Buffer
{
public:
    // 返回剩余空间
    virtual unsigned int GetEmptySize();
    
    // 申请一次空间，dataLength表示有效数据长度，这里主要是为了用于判断缓冲区是为满还是为空（因为这两种情况都是_beginIndex == _endIndex， 所以传入有效数据长度解决这个问题）
    void ReAllocBuffer(unsigned int dataLength);
    
    // 返回数据起始位置
    unsigned int GetBeginIndex() const 
    {
        return _beginIndex;
    }

    // 返回数据终止位置
    unsigned int GetEndIndex() const 
    {
        return _endIndex;
    }

    // 返回缓冲区大小
    unsigned int GetTotalSize() const
    {
        return _bufferSize;
    }

protected:
    char* _buffer = nullptr;        // 缓冲区
    unsigned int _beginIndex = 0;   // _buffer数据的起始位置
    unsigned int _endIndex = 0;     // _buffer数据的终止位置的下一个可写入位置
    unsigned int _bufferSize = 0;   // 缓冲区大小
};