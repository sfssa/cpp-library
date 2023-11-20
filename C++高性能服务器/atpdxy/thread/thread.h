/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-18 09:07:29
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-20 12:20:00
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/thread/thread.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "../mutex/mutex.h"

namespace atpdxy
{
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
