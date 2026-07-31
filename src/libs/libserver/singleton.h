#pragma once 

#include <stdexcept>

template<typename T>
class Singleton
{
public:
    // 初始化单例
    template<typename ... Args>
    static T* Instance(Args&& ... args)
    {
        if(m_pInstance == nullptr)
            m_pInstance = new T(std::forward<Args>(args)...);

        return m_pInstance;
    }

    // 获取单例
    static T* GetInstance()
    {
        if(m_pInstance == nullptr)
            throw std::logic_error( "the instance is not init, please initialize the instance first" );
            
        return m_pInstance;
    }

    // 销毁单例
    static void DestroyInstance()
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }

private:
    static T* m_pInstance;
};

template<class T> T* Singleton<T>::m_pInstance = nullptr;