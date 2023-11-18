/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-18 10:09:12
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-18 10:55:40
 * @FilePath: /cpp-library/C++高性能服务器/test/test_thread.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "../atpdxy/atpdxy.h"
#include <unistd.h>


atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

void fun1()
{
    ATPDXY_LOG_INFO(g_logger)<<"name: "<<atpdxy::Thread::GetName()
                            <<" this.name: "<<atpdxy::Thread::GetThis()->getName()
                            <<" id: "<<atpdxy::getThreadId()
                            <<" this.id: "<<atpdxy::Thread::GetThis()->getId();
    sleep(20);
}

void func2()
{

}

int main(int argc,char** argv)
{
    ATPDXY_LOG_INFO(g_logger)<<"thread test begin";
    std::vector<atpdxy::Thread::ptr> thrs;
    for(int i=0;i<5;++i)
    {
        atpdxy::Thread::ptr thr(new atpdxy::Thread(&fun1,"name_"+std::to_string(i)));
        thrs.push_back(thr);
    }
    
    for(int i=0;i<5;++i)
        thrs[i]->join();

    ATPDXY_LOG_INFO(g_logger)<<"thread test end";
    return 0;
}