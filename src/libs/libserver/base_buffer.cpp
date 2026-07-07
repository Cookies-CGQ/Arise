#include "base_buffer.h"
#include <iostream>
#include <cstring>
#include <string>

unsigned Buffer::GetEmptySize()
{
    return _bufferSize - _endIndex;
}

void Buffer::ReAllocBuffer(unsigned int dataLength)
{
    // 如果缓冲区已经超过最大缓冲区，可能有异常
    if(_bufferSize >= MAX_SIZE)
    {
        std::cout << "Buffer::ReAllocBuffer: Buffer size is too large!" << std::endl;
        return;
    }

    // 新扩容缓冲区
    char* tempBuffer = new char[_bufferSize + ADDITIONAL_SIZE];
    unsigned int newEndIndex;
    // 情况一：未环回
    if(_beginIndex < _endIndex)
    {
        ::memcpy(tempBuffer, _buffer + _beginIndex, _endIndex - _beginIndex);
        newEndIndex = _endIndex - _beginIndex;
    }
    // 情况二：环回
    else
    {
        // 为空
        if(_beginIndex == _endIndex && dataLength <= 0)
        {
            newEndIndex = 0;
        }
        // 为满
        else
        {
            // 先copy尾部
            ::memcpy(tempBuffer, _buffer + _beginIndex, _bufferSize - _beginIndex);
            newEndIndex = _bufferSize - _beginIndex;
            // 再追加头部
            if(_endIndex > 0)
            {
                ::memcpy(tempBuffer + newEndIndex, _buffer, _endIndex);
                newEndIndex += _endIndex;
            }
        }
    }

    _bufferSize += ADDITIONAL_SIZE;
    delete[] _buffer;
    _buffer = tempBuffer;
    _beginIndex = 0;
    _endIndex = newEndIndex;
}