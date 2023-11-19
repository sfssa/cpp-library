/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-19 09:06:21
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 10:40:31
 * @FilePath: /cpp-library/C++高性能服务器/test/test_utils.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "../atpdxy/atpdxy.h"
#include <assert.h>

atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

void test_assert()
{
    ATPDXY_LOG_INFO(g_logger)<<atpdxy::BacktraceToString(10);
    // ATPDXY_ASSERT(false);
    ATPDXY_ASSERT2(0==1,"atpdxy");
}

int main(int argc,char** argv)
{
    // assert(1);
    // assert(0);
    test_assert();
    return 0;
}

