/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 12:23:16
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/fiber/fiber.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <memory>
#include <functional>
#include <ucontext.h>
#include "../thread/thread.h"

namespace atpdxy
{

// 对象应该由智能指针管理而不是在栈上创建
class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State
    {
        INIT, // 初始化状态
        HOLD, // 阻塞状态
        EXEC, // 执行状态
        TERM, // 执行完毕状态
        READY,// 就绪状态
        EXCEPT// 出错状态
    };
private:
    Fiber();
public:
    Fiber(std::function<void()> cb,size_t stacksize=0);
    ~Fiber();

    // 重置协程的执行函数并重置状态INIT/TERM
    void reset(std::function<void()> cb);

    // 切换到当前协程执行
    void swapIn();

    // 把当前协程切换到后台
    void swapOut();

public:
    // 设置协程
    static void SetThis(Fiber* f);

    // 返回当前执行点的协程
    static Fiber::ptr GetThis();

    // 协程切换到后台并设置为Ready状态
    static void YieldToReady();

    // 协程切换后台并设置为Hold状态
    static void YieldToHold();

    // 获得总协程数
    static uint64_t TotalFibers();

    static void MainFunc();

    static uint64_t GetFiberId();

    uint64_t getId() const {return m_id;}
private:
    uint64_t m_id=0;            // 协程ID
    uint32_t m_stacksize=0;     // 协程栈大小
    State m_state=INIT;         // 协程状态
    ucontext_t m_ctx;           // 协程
    void* m_stack=nullptr;      // 协程栈指针
    std::function<void()> m_cb; // 协程回调函数
};

}
 
