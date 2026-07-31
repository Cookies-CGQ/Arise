#include <iostream>
#include <cstring>
#include "base_buffer.h"
#include "log4_help.h"

unsigned int Buffer::GetEmptySize()
{
    return _bufferSize - _endIndex;
}

void Buffer::ReAllocBuffer(const unsigned int dataLength)
{
	// 如果缓冲区超过最大缓冲值，发出警告
	if (_bufferSize >= MAX_SIZE) 
    {
		std::cout << "Buffer::Realloc except!! Max size:" << _bufferSize << std::endl;
	}

    char* tempBuffer = new char[_bufferSize + ADDITIONAL_SIZE];
    unsigned int _newEndIndex = 0;
    // 此时数据未成环且一定有数据
    if(_beginIndex < _endIndex)
    {
        ::memcpy(tempBuffer, _buffer + _beginIndex, _endIndex - _beginIndex);
        _newEndIndex = _endIndex - _beginIndex;
    }
    // 此时有几种情况：1、数据为空；2、数据为满；3、数据成环；2 3情况同一处理
    else
    {
        // 数据为空
        if(_beginIndex == _endIndex && dataLength <= 0)
        {
            _newEndIndex = 0;
        }
        // 数据为满/数据成环 
        else
        {
            // 从_beginIndex拷贝到缓冲区结束
            ::memcpy(tempBuffer, _buffer + _beginIndex, _bufferSize - _beginIndex);
            _newEndIndex = _bufferSize - _beginIndex;
            // 拷贝循环的数据
            if(_endIndex > 0)
            {
                ::memcpy(tempBuffer + _newEndIndex, _buffer, _endIndex);
                _newEndIndex += _endIndex;
            }
        }
    }

    delete[] _buffer;
    _buffer = tempBuffer;
    _bufferSize += ADDITIONAL_SIZE;
    _beginIndex = 0;
    _endIndex = _newEndIndex;

#if TestNetwork
	LOG_WARN("Buffer::Realloc. _bufferSize:" << _bufferSize );
#endif
}