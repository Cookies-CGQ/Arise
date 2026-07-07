#pragma once

#include <map>
#include <iostream>

#define DynamicStateCreate(classname, enumType) \
    static void* CreateState() { return new classname; } \
    RobotStateType GetState( ) override { return enumType; }

#define DynamicStateBind(classname) \
    reinterpret_cast<CreateIstancePt>( &( classname::CreateState ) )

// 状态类模板(enumType:状态枚举类型, T:管理状态的类)
template <typename enumType, class T>
class StateTemplate 
{
public:
    StateTemplate(){}
    // 设置状态管理对象
    void SetParentObj(T* pObj)
    {
        _pParentObj = pObj;
    }

    virtual ~StateTemplate() {}
    // 获取状态类型
    virtual enumType GetState() = 0;
    // 更新状态，返回更新之后的状态类型
    virtual enumType Update() = 0;
    // 进入状态行为
    virtual void EnterState() = 0;
    // 离开状态行为
    virtual void LeaveState() = 0;
    
protected:
    // 状态管理对象
    T* _pParentObj;
};

template <typename enumType, class StateClass, class T>
class StateTemplateMgr
{
public:
    virtual ~StateTemplateMgr()
    {
        if(_pState != nullptr)
        {
            delete _pState; // 释放状态对象资源
            _pState = nullptr;
        }
    }

    // 初始化默认状态，并注册创建状态对象回调
    void InitStateTemplateMgr(enumType defaultState)
    {
        _defaultState = defaultState;
        RegisterState();
    }

    // 切换状态
    // 1、创建新状态对象
    // 2、调用旧状态对象的LeaveState
    // 3、释放旧状态对象
    // 4、调用新状态对象的EnterState
    void ChangeState(enumType stateType)
    {
        StateClass* pNewState = CreateStateObj(stateType);
        if(pNewState == nullptr)
        {
            std::cout << "ChangeState failed, stateType: " << stateType << std::endl;
            return;
        }
        else
        {
            if(_pState != nullptr)
            {
                _pState->LeaveState();
                delete _pState;
            }
            _pState = pNewState;
            _pState->EnterState(); // 进入新状态
        }
    }

    // 状态更新
    void UpdateState()
    {
        // 起始切换到默认起始状态
        if(_pState == nullptr)
        {
            ChangeState(_defaultState);
        }
        enumType curState = _pState->Update();
        // 如果更新之后的状态与旧状态不一致就切换状态
        if(curState != _pState->GetState())
        {
            ChangeState(curState); // 状态类型切换
        }
    }

protected:
    // 批量注册创建状态对象回调函数
    virtual void RegisterState() = 0;

public:
    // 创建状态对象回调函数类型
    typedef StateClass* (*CreateIstancePt)();

    // 根据状态类型创建相应的状态对象
    StateClass* CreateStateObj(enumType enumValue) 
    {
        auto iter = _dynCreateMap.find(enumValue);
        if (iter == _dynCreateMap.end())
            return nullptr;

        CreateIstancePt np = iter->second;
        StateClass* pState = np();
        pState->SetParentObj(static_cast<T*>(this));
        return pState;
    }

    // 注册创建状态对象回调函数
    void RegisterStateClass(enumType enumValue, CreateIstancePt np)
    {
        _dynCreateMap[enumValue] = np;
    }

protected:
    // 状态类型与创建该状态对象的函数的映射
    std::map<enumType, CreateIstancePt> _dynCreateMap;
    // 状态对象
    StateClass* _pState = nullptr;
    // 默认起始状态
    enumType _defaultState;
};