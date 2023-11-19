/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 10:26:36
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/utils/utils.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "utils.h"
#include "../log/log.h"
#include <execinfo.h>
#include "../fiber/fiber.h"

namespace atpdxy
{

atpdxy::Logger::ptr g_logger =ATPDXY_LOG_NAME("system");

pid_t getThreadId()
{
    return syscall(SYS_gettid);
}

uint32_t getFiberId()
{
    return atpdxy::Fiber::GetFiberId();
}

void BackTrace(std::vector<std::string>& bt,int size,int skip)
{
    // 指针数组
    void** array=(void**)malloc(sizeof(void*)*size);
    size_t s=::backtrace(array,size);

    char** strings=backtrace_symbols(array,s);
    if(strings==NULL)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"backtrace_symbols error";
        return;
    }
    
    for(size_t i=skip;i<s;++i)
    {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size,int skip,const std::string& prefix)
{
    std::vector<std::string> bt;
    BackTrace(bt,size,skip);
    std::stringstream ss;
    for(size_t i=0;i<bt.size();++i)
    {
        ss<<prefix<<bt[i]<<std::endl;
    }
    return ss.str();
}


}
