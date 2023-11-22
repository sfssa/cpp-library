/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-22 16:05:57
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/fiber/fiber.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <memory>
#include <functional>
#include <ucontext.h>
// #include "../mutex/mutex.h"
// #include "../thread/thread.h"

namespace atpdxy
{

class Scheduler;
// 对象应该由智能指针管理而不是在栈上创建
class Fiber : public std::enable_shared_from_this<Fiber>
{
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State
    {
        INIT, // 初始化状态
        HOLD, // 暂停状态
        EXEC, // 执行状态
        TERM, // 执行完毕状态
        READY,// 就绪状态
        EXCEPT// 异常状态
    };
private:
    // 每个线程第一个协程的构造
    Fiber();
public:
    Fiber(std::function<void()> cb,size_t stacksize=0,bool use_caller=false);
    ~Fiber();

    // 重置协程的执行函数并重置状态INIT/TERM
    void reset(std::function<void()> cb);

    // 切换到当前协程执行
    void swapIn();

    // 把当前协程切换到后台
    void swapOut();

    // 将当前线程切换到执行状态，执行的为当前线程的主协程
    void call();

    // 将当前线程切换到后台，执行的为该协程，返回线程的主协程
    void back();

public:
    // 设置线程的运行协程
    static void SetThis(Fiber* f);

    // 返回当前执行点的协程
    static Fiber::ptr GetThis();

    // 协程切换到后台并设置为Ready状态
    static void YieldToReady();

    // 协程切换后台并设置为Hold状态
    static void YieldToHold();

    // 获得总协程数
    static uint64_t TotalFibers();

    // 协程的主要执行函数，执行完毕返回线程主协程
    static void MainFunc();

    // 协程执行函数，执行完毕返回线程调度协程
    static void CallerMainFunc();

    // 获取协程ID
    static uint64_t GetFiberId();

    // 获取当前协程的id
    uint64_t getId() const {return m_id;}

    // 获得状态
    State getState() const {return m_state;}

    // void setState(const State state){m_state=state;}
private:
    uint64_t m_id=0;            // 协程ID
    uint32_t m_stacksize=0;     // 协程栈大小
    State m_state=INIT;         // 协程状态
    ucontext_t m_ctx;           // 协程上下文
    void* m_stack=nullptr;      // 协程栈指针
    std::function<void()> m_cb; // 协程回调函数
};

}
 