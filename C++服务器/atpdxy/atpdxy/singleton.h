#pragma once

#include <memory>

/* 这个文件定义两个单例模式的包装类
 * 第一个返回单例对象的指针，适合需要在程序任何地方获取相同实例的场景，但不提供自动释放对象的功能
 * 第二个返回单例对象的智能指针，适合需要计数和自动释放对象的场景
 */

namespace server
{

template<class T>
class singleton final
{
public:
    static T* get_instance()
    {
        static T instance;
        return &instance;
    }

private:
    singleton()=default;
};

template<class T>
class singleton_shared_ptr final
{
public:
    static std::shared_ptr<T> get_instance()
    {
        static auto instance=std::make_shared<T>();
        return instance;
    }
private:
    singleton_shared_ptr()=default;
};

}
