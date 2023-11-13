#pragma once 
#include <memory>

namespace atpdxy
{
template <class T,class X,int N>
T& getInstanceX()
{
    static T v;
    return v;
}

template <class T,class X,int N>
std::shared_ptr<T> getInstancePtr()
{
    static std::shared_ptr<T> v(new T);
    return v;
}

template <class T,class X=void,int N=0>
class Singleton
{
public:
    static T* getInstance()
    {
        static T v;
        return &v;
    }
};

template <class T,class X=void,int N=0>
class SingletonPtr
{
public:
    static std::shared_ptr<T> getInstancePtr()
    {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}
