#pragma once

#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <cstdint>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

namespace server
{

//简单封装的信号量
class my_semaphore:public noncopyable
{
public: 
    //接收一个uint32_4类型的参数来初始化信号量的初始计数值，表示信号量的可用资源数量
    explicit my_semaphore(uint32_t count);

    //析构函数，释放信号量所占资源
    ~my_semaphore();

    //等待信号量，当计数值>0时会减少计数值并返回；若为0则表示没有可用资源，线程会被阻塞直到被唤醒
    void wait();

    //通知信号量，增加计数器，当某个线程完成了对共享资源的访问就调用这个函数来唤醒其他等待这个资源的线程
    void notify();

private:
    //私有信号量
    sem_t my_semaphore_;
};

//作用域锁，用来确保在作用域结束后自动释放锁，防止资源泄漏和死锁，用来实现线程锁的包装
//注意下面my_mutex_调用的lock和unlock是对象的my_mutex_的成员函数而不是本类中封装的成员函数
template<class T>
class scoped_lock_impl
{
public:
    explicit scoped_lock_impl(T* mutex)
        :my_mutex_(mutex)
    {
        my_mutex_->lock();
        is_locked_=true;
    }

    ~scoped_lock_impl()
    {
        unlock();
    }

    void lock()
    {
        if(!is_locked_)
        {
            my_mutex_->lock();
            is_locked_=true;
        }
    }

    void unlock()
    {
        if(is_locked_)
        {
            is_locked_=true;
            my_mutex_->unlock();
        }
    }
private:
    T* my_mutex_;       //指向锁对象的指针
    bool is_locked_;    //是否已经上锁
};

//
}
