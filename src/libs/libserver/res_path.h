#pragma once

#include <string>
#include "system.h"
#include "component.h"

// 全局资源路径组件，用于确定项目的res目录在哪里，并为其他模块生成资源文件的绝对路径。
class ResPath: public Component<ResPath>, public IAwakeSystem<>
{
public:
    void Awake();
    void BackToPool();
    
    // 资源路径拼接
    std::string FindResPath(const std::string& res);
    std::string FindResPath(const char* res);

private:
    std::string _resPath = "";
};