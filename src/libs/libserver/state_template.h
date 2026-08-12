#pragma once

#include <map>
#include "log4_help.h"

// 状态机模板

// 宏展开获得函数：工厂方法 + 返回自己的枚举值
#define DynamicStateCreate(classname, enumType)          \
    static void *CreateState() { return new classname; } \
    RobotStateType GetState() override { return enumType; }

#define DynamicStateBind(classname) \
    reinterpret_cast<CreateIstancePt>(&(classname::CreateState))

// 状态基类模板 -- enumType：状态枚举类型；T：状态拥有者的类型
template <typename enumType, class T>
class StateTemplate
{
public:
    StateTemplate()
    {

    }

    // 设置状态拥有者
    void SetParentObj(T *pObj)
    {
        _pParentObj = pObj;
    }

    virtual ~StateTemplate() {}

    // 返回当前状态的枚举值
    virtual enumType GetState() = 0;
    // 帧函数，返回新状态（状态不变则返回当前）
    virtual enumType Update() = 0;
    // 进入状态时调用
    virtual void EnterState() = 0;
    // 离开状态时调用
    virtual void LeaveState() = 0;

protected:
    T *_pParentObj = nullptr; // 指向拥有这个状态的对象的指针
};

// 状态管理器 -- enumType：状态枚举；StateClass：状态的基类；T：拥有者的类型
template <typename enumType, class StateClass, class T>
class StateTemplateMgr
{
public:
    virtual ~StateTemplateMgr()
    {
        if (_pState != nullptr)
        {
            delete _pState;
        }
    }

    // 初始化
    void InitStateTemplateMgr(enumType defaultState)
    {
        _defaultState = defaultState;
        RegisterState();
    }

    // 状态切换
    void ChangeState(enumType stateType)
    {
        // 状态相同，不处理
        if (_pState != nullptr && _pState->GetState() == stateType)
        {
            LOG_ERROR("ChangeState: same state type:" << GetRobotStateTypeShortName(stateType));
            return;
        }
        
        StateClass *pNewState = CreateStateObj(stateType);
        if (pNewState == nullptr)
        {
            return;
        }

        if (pNewState != nullptr)
        {
            if (_pState != nullptr)
            {
                // 状态转移调用
                _pState->LeaveState();
                delete _pState;
            }

            _pState = pNewState;
            _pState->EnterState();
        }
    }

    // 帧函数 -- 状态更新
    void UpdateState()
    {
        if (_pState == nullptr)
        {
            ChangeState(_defaultState);
        }

        // 执行状态对象Update，如果状态切换执行ChangeState
        enumType curState = _pState->Update();
        if (curState != _pState->GetState())
        {
            ChangeState(curState);
        }
    }

protected:
    // 批量注册 -- 状态枚举：工厂函数
    virtual void RegisterState() = 0;

public:
    // 创建状态对象函数类型
    typedef StateClass *(*CreateIstancePt)();

    // 根据状态类型创建状态对象
    StateClass *CreateStateObj(enumType enumValue)
    {
        auto iter = _dynCreateMap.find(enumValue);
        if (iter == _dynCreateMap.end())
            return nullptr;

        // 创建函数
        CreateIstancePt np = iter->second;
        StateClass *pState = np();
        pState->SetParentObj(static_cast<T*>(this)); // 状态拥有者
        
        return pState;
    }

    // 注册 -- 状态枚举：工厂函数
    void RegisterStateClass(enumType enumValue, CreateIstancePt np)
    {
        _dynCreateMap[enumValue] = np;
    }

protected:
    std::map<enumType, CreateIstancePt> _dynCreateMap;  // 状态枚举：工厂函数
    StateClass *_pState = nullptr;                      // 当前状态对象
    enumType _defaultState;                             // 初始状态
};