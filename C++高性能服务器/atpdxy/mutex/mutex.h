#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <list>

#include "../fiber/fiber.h"

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

}