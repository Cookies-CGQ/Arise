#pragma once

#include <string>
#include "resource_base.h"
#include "resource_mgr_template.h"
#include "libserver/vector3.h"

// 读取地图配置文件

enum class ResourceWorldType
{
    Login = 1,
    Roles = 2, // 角色选择场景
    Public = 3,
    Dungeon = 4,
};

// 地图配制文件单行数据
class ResourceWorld : public ResourceBase
{
public:
    explicit ResourceWorld(std::map<std::string, int>& head);
    // 配合合法性检验
    bool Check() override;

    // 获取地图名字
    std::string GetName() const;
    // 获取地图类型
    ResourceWorldType GetType() const;
    // 是否是指定地图类型
    bool IsType(ResourceWorldType iType) const;
    // 是否是玩家初始落地地图
    bool IsInitMap() const;

    // 获取地图默认初始位置（出生点）
    Vector3 GetInitPosition() const;

protected:
    // 读取配置文件数据，生成内存数据
    void GenStruct() override;

private:
    std::string _name = "";                                      // 地图名字
    bool _isInit = false;                                        // 是否是玩家初始落地地图
    ResourceWorldType _worldType = ResourceWorldType::Dungeon;   // 地图类型
    Vector3 _initPosition{ 0,0,0 };                              // 出生点
};

class ResourceWorldMgr :public ResourceManagerTemplate<ResourceWorld>
{
public:
    // 二级遍历获取相关id数据
    bool AfterInit() override;
    
    ResourceWorld* GetInitMap();
    ResourceWorld* GetRolesMap();

private:
    int _initMapId = 0;   // 初始地图
    int _rolesMapId = 0;  // 角色选择场景
};
