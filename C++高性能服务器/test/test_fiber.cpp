/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-19 15:11:43
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-22 16:51:38
 * @FilePath: /cpp-library/C++高性能服务器/test/test_fiber.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "../atpdxy/atpdxy.h"

atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

void run_in_fiber() {
    ATPDXY_LOG_INFO(g_logger) << "run_in_fiber begin";
    atpdxy::Fiber::YieldToHold();
    ATPDXY_LOG_INFO(g_logger) << "run_in_fiber end";
    atpdxy::Fiber::YieldToHold();
}

void test_fiber() {
    ATPDXY_LOG_INFO(g_logger) << "main begin -1";
    {
        atpdxy::Fiber::GetThis();
        ATPDXY_LOG_INFO(g_logger) << "main begin";
        atpdxy::Fiber::ptr fiber(new atpdxy::Fiber(run_in_fiber));
        fiber->swapIn();
        ATPDXY_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        ATPDXY_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    ATPDXY_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    atpdxy::Thread::SetName("main");

    std::vector<atpdxy::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(atpdxy::Thread::ptr(
                    new atpdxy::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}