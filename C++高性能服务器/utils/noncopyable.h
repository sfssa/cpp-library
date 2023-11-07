#pragma once

/*
    不可复制对象封装
    通常，这种做法用于基类，以确保派生类不会无意继承了赋值构造函数和赋值操作符，
    从而防止派生类的对象被复制。这种设计模式是C++中的一种“禁止拷贝语义”的技巧，
    通常用于类的设计，以确保对象的唯一性，或者强制使用特定的手动管理资源的方式。

    示例用途：
        1、单例模式：确保只有一个实例存在，因为拷贝构造和赋值操作符可能导致多个实例出现；
        2、管理资源：如果类管理某些资源，比如文件句柄或内存，为了避免这些资源被不小心复制，
        使用禁止拷贝来进行保护。
*/

namespace atpdxy
{

class NonCopyable
{
public:
    // 默认构造
    NonCopyable()=default;

    // 默认析构
    ~NonCopyable()=default;

    // 禁止拷贝构造
    NonCopyable(const NonCopyable&)=delete;

    // 禁止赋值
    NonCopyable& operator=(const NonCopyable&)=delete;
};

}