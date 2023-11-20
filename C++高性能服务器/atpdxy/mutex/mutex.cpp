/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-20 09:44:42
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-20 12:23:30
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/mutex/mutex.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "./mutex.h"
#include "../macro.h"

namespace atpdxy
{
    
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

}