/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-18 23:17:06
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/thread/thread.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "./thread.h"
#include "../log/log.h"
#include "../utils/utils.h"

namespace atpdxy
{

// thread_local表名变量是线程局部的，每个线程都有自己的实例互不影响，存储在每个线程的栈上而不是堆或全局数据区
// 用于多线程下跟踪线程的一些信息，将这样的信息存储在线程局部变量中，确保每个线程有自己的独立存储空间，而不会干
// 扰其他线程数据
static thread_local Thread* t_thread=nullptr;            // 线程
static thread_local std::string t_thread_name="UNKNOW";  // 线程名称

static atpdxy::Logger::ptr g_logger =ATPDXY_LOG_NAME("system");

Semaphore::Semaphore(uint32_t count)
{
    if(sem_init(&m_semaphore,0,count))
    {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()
{
    // 避免被打断
    if(sem_wait(&m_semaphore))
        throw std::logic_error("sem_wait error");
}

void Semaphore::notify()
{
    if(sem_post(&m_semaphore))
        throw std::logic_error("sem_post error");
}

Thread* Thread::GetThis()
{
    return t_thread;
}

const std::string& Thread::GetName()
{
    return t_thread_name;
}

void Thread::SetName(const std::string& name)
{
    if(t_thread)
        t_thread->m_name=name;
    t_thread_name=name;
}

Thread::Thread(std::function<void()> cb,const std::string& name)
    :m_cb(cb),m_name(name)
{
    if(name.empty())
        m_name="UNKNOW";
    // 创建成功返回0，否则返回错误码
    int rt=pthread_create(&m_thread,nullptr,&Thread::run,this);
    if(rt)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"pthread_create thread fail, rt="<<rt
            <<" name="<<name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread()
{
    // 分离线程，执行完毕后自动回收资源
    if(m_thread)
        pthread_detach(m_thread);
}

void Thread::join()
{
    if(m_thread)
    {
        int rt=pthread_join(m_thread,nullptr);
        if(rt)
        {
            ATPDXY_LOG_ERROR(g_logger)<<"pthread_join thread fail, rt="<<rt
                <<" name="<<m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread=0;
    }
}

void* Thread::run(void* arg)
{
    Thread* thread=(Thread*)arg;
    t_thread=thread;
    t_thread_name=thread->m_name;
    thread->m_id=atpdxy::getThreadId();
    // pthread_self返回线程名称。设置当前线程的名称
    pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str());
    std::function<void()> cb;
    // 交换cb和thread->m_cb的内容，在当前线程执行回调函数前，将回调函数从线程对象中取出来，避免回调执行期间被修改
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();
    cb();
    return 0;
}

}
