#pragma once

/* 这个文件用来禁用拷贝构造函数和拷贝赋值操作符
 * 用来被线程池继承以禁用拷贝构造函数和赋值操作符
 * 将这个类作为基类的派生类可以确保它们无法被拷贝或者赋值，从而增加代码的安全性和可维护性
 * 能够避免多个对象共享资源
 */

namespace server
{

class noncopyable
{
public:
    noncopyable(const noncopyable&)=delete;
    void operator=(const noncopyable&)=delete;
protected:
    noncopyable()=default;
    ~noncopyable()=default;
};

}
