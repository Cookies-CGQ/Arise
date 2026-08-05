#pragma once

#include <string>
#include "libserver/common.h"

// 3D向量
struct Vector3
{
    Vector3(const float x, const float y, const float z)
        : X(x), Y(y), Z(z)
    {

    }

    // proto -> 内存变量
	void ParserFromProto(Proto::Vector3 position);
	// 内存变量 -> proto
    void SerializeToProto(Proto::Vector3* pProto) const;
	// 欧氏距离（仅 XZ 平面）
    double GetDistance(Vector3 point) const;

    // 坐标
    float X = 0; // 水平x
    float Y = 0; // 高度y
    float Z = 0; // 水平z

    // 静态零向量(0,0,0)
    static Vector3 Zero;
};

// 符号 << 重载
std::ostream& operator <<(std::ostream& os, Vector3& v);