/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-18 09:07:29
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-18 23:27:30
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/thread/thread.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>

namespace atpdxy
{

class Semaphore
{
public:
    Semaphore(uint32_t count=0);
    ~Semaphore();
    void wait();
    void notify();
private:
    Semaphore(const Semaphore&)=delete;
    Semaphore(const Semaphore&&)=delete;
    Semaphore& operator=(const Semaphore&)=delete;
private:
    sem_t m_semaphore;
};

// RAII释放锁资源
template <class T>
struct ScopedLockImpl
{
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.lock();
        m_locked=true;
    }

    ~ScopedLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.lock();
            m_locked=true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked=false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template <class T>
struct ReadScopeLockImpl
{
public:
    ReadScopeLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.rdlock();
        m_locked=true;
    }

    ~ReadScopeLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.rdlock();
            m_locked=true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked=false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template <class T>
struct WriteScopeLockImpl
{
public:
    WriteScopeLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.wrlock();
        m_locked=true;
    }

    ~WriteScopeLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.wrlock();
            m_locked=true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked=false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class Mutex
{
public:
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex()
    {
        pthread_mutex_init(&m_mutex,nullptr);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

class NullMutex
{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    NullMutex(){}
    ~NullMutex(){}
    void lock(){}
    void unlock(){}
};

class RWMutex
{
public:
    typedef ReadScopeLockImpl<RWMutex> ReadLock;
    typedef WriteScopeLockImpl<RWMutex> WriteLock;
    RWMutex()
    {
        pthread_rwlock_init(&m_lock,nullptr);
    }

    ~RWMutex()
    {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock()
    {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock()
    {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock()
    {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

// 空锁验证logger输出有重叠
class NullRWMutex
{
public:
    typedef ReadScopeLockImpl<NullMutex> ReadLock;
    typedef WriteScopeLockImpl<NullMutex> WriteLock;
    NullRWMutex(){}
    ~NullRWMutex(){}
    void rdlock(){}
    void wrlock(){}
    void unlock(){}
};

// 自旋锁：不会阻塞线程而是不断的忙等直到获取锁为止，如果短时间内获取到锁，那么自旋锁会比互斥锁更高效
// 因为没有线程挂起和唤醒的开销
class Spinlock
{
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock()
    {
        pthread_spin_init(&m_mutex,0);
    }

    ~Spinlock()
    {
        pthread_spin_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_spin_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

/*
    CAS是一种原子操作，确保在多线程中对共享变量进行原子的读取和写入
*/
class CASLock
{
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock()
    {
        m_mutex.clear();
    }

    ~CASLock()
    {

    }

    // 如果锁已经被占用，那么一直循环等待，直到获得锁
    void lock()
    {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex,std::memory_order_acquire));
    }

    // 释放锁
    void unlock()
    {
        std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
    }
private:
    // 原子标志，用来实现自旋锁（C++11中atomic自身提供了原子性操作，不需要volatile修饰）
    volatile std::atomic_flag m_mutex;
};

class Thread
{
public:
    // 智能指针
    typedef std::shared_ptr<Thread> ptr;

    // 接受一个线程执行的函数和线程的名称
    Thread(std::function<void()> cb,const std::string& name);

    ~Thread();

    // 获取线程的名称
    const std::string& getName(){return  m_name;}

    // 获取线程的ID
    pid_t getId() const {return m_id;}

    // 等待线程执行结束，系统释放资源
    void join();

    // 获取线程ID
    static Thread* GetThis();

    // 获取线程名称
    static const std::string& GetName();

    // 设置线程名称
    static void SetName(const std::string& name);
private:
    // 禁止拷贝
    Thread(const Thread&)=delete;
    Thread(const Thread&&)=delete;
    Thread& operator=(const Thread&)=delete;

    // 线程执行函数
    static void* run(void* arg);
private:
    pid_t m_id=-1;                    // 线程id
    pthread_t m_thread=0;             // 线程
    std::function<void()> m_cb;       // 线程执行函数
    std::string m_name;               // 线程名称
    Semaphore m_semaphore;            // 信号量
};


}
