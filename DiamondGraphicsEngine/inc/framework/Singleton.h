#pragma once
/////////////////////////////////////////////////////////
///CRTP design based Singleton class, use by inheritance
/////////////////////////////////////////////////////////
//BUG not actually single
template <typename T>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static T& GetInstance()
    {
        if (m_instance == nullptr)
        {
            m_instance = new T();
        }
        return *m_instance;
    }

protected:

    Singleton() = default;

    virtual ~Singleton()
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }
    static T* m_instance;
};

template <class T>
T* Singleton<T>::m_instance = nullptr;
