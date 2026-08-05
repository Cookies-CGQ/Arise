#pragma once

// 世界基类
class IWorld
{
public:
    int GetWorldId() const;

protected:
    int _worldId = 0; // 世界ID
};