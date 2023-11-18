/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-18 10:09:12
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 00:16:06
 * @FilePath: /cpp-library/C++高性能服务器/test/test_thread.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "../atpdxy/atpdxy.h"
#include <unistd.h>


atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();
int count=0;
// atpdxy::RWMutex s_mutex;
atpdxy::Mutex m_mutex;
// 加上锁后所需要的时间变长了

void fun1()
{
    ATPDXY_LOG_INFO(g_logger)<<"name: "<<atpdxy::Thread::GetName()
                            <<" this.name: "<<atpdxy::Thread::GetThis()->getName()
                            <<" id: "<<atpdxy::getThreadId()
                            <<" this.id: "<<atpdxy::Thread::GetThis()->getId();
    // sleep(20);
    for(int i=0;i<1000000;++i)
    {
        atpdxy::Mutex::Lock lock(m_mutex);
        ++count;
    }
}

void func2()
{
    while(true)
        ATPDXY_LOG_INFO(g_logger)<<"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

void func3()
{
    while(true)
        ATPDXY_LOG_INFO(g_logger)<<"==================================";
    
}

int main(int argc,char** argv)
{
    ATPDXY_LOG_INFO(g_logger)<<"thread test begin";
    YAML::Node root=YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/log2.yml");
    atpdxy::Config::loadFromYaml(root);
    std::vector<atpdxy::Thread::ptr> thrs;
    for(int i=0;i<2;++i)
    {
        // atpdxy::Thread::ptr thr(new atpdxy::Thread(&fun1,"name_"+std::to_string(i)));
        // thrs.push_back(thr);

        atpdxy::Thread::ptr thr1(new atpdxy::Thread(&func2,"name_"+std::to_string(i*2)));
        atpdxy::Thread::ptr thr2(new atpdxy::Thread(&func3,"name_"+std::to_string(i*2+1)));
        thrs.push_back(thr1);
        thrs.push_back(thr2);
    }
    
    for(size_t i=0;i<thrs.size();++i)
        thrs[i]->join();

    ATPDXY_LOG_INFO(g_logger)<<"thread test end";
    ATPDXY_LOG_INFO(g_logger)<<"count="<<count;
    
    return 0;
}