/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-20 09:38:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-20 23:28:02
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/scheduler/scheduler.h
 * @Description: 线程池：分配一组线程；
 *               协程调度器：将协程分配到指定线程上执行
 *               具有线程池支持的协程调度器
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include "../fiber/fiber.h"
#include "../thread/thread.h"
#include "../utils/noncopyable.h"

namespace atpdxy
{

// 内部有一个线程池，支持协程在线程池中切换
class Scheduler
{
public:
    // 调度器智能指针
    typedef std::shared_ptr<Scheduler> ptr;

    // 互斥锁
    typedef Mutex MutexType;

    // 线程池大小，是否使用调用线程作为调度器线程之一，线程池名称
    Scheduler(size_t threads=1,bool use_caller=true,const std::string& name="");

    // 析构函数
    virtual ~Scheduler();

    // 返回协程调度器的名称
    const std::string& getName() const {return m_name;}

    // 获得当前协程调度器
    static Scheduler* GetThis(); 

    // 协程调度器主协程（调度协程）   
    static Fiber* GetMainFiber();

    // 启动调度器  
    void start();

    // 停止调度器
    void stop();

    // 将协程或回调安排到执行队列
    // 第一个参数是协程或者函数
    // 第二个参数是协程执行的线程ID
    template <class FiberOrCb>
    void schedule(FiberOrCb fc,int thread=-1)
    {
        bool need_tickle=false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle=scheduleNoBlock(fc,thread);
        }

        if(need_tickle)
        {
            tickle();
        }
    }

    // 将协程或回调范围安排到执行队列
    template <class InputIterator>
    void schedule(InputIterator begin,InputIterator end)
    {
        bool need_tickle=false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin!=end)
            {
                need_tickle=scheduleNoBlock(&*begin,-1) || need_tickle;
                ++begin;
            }
        }
        // 需要唤醒调度器
        if(need_tickle)
            tickle();
    }

    // 切换调度器
    void switchTo(int thread=-1);

    // 将调度器的状态信息输出到给定的输出流，查看调度器的运行状态
    std::ostream& dump(std::ostream& os);
protected:
    // 唤醒调度器，通知协程调度器有任务到来
    virtual void tickle();

    // 协程调度函数
    void run();

    // 返回是否可以停止调度器
    virtual bool stopping();

    // 协程无任务时调度执行idle协程
    virtual void idle();

    // 设置当前的协程调度器
    void setThis();

    // 是否有空闲线程
    bool hasIdleThreads(){return m_idleThreadCount>0;}
private:
    // 协程调度启动，无锁版
    template <class FiberOrCb>
    bool scheduleNoBlock(FiberOrCb fc,int thread)
    {
        // 协程容器是否为空，判断是否需要唤醒调度器
        bool need_tickle=m_fibers.empty();
        FiberAndThread ft(fc,thread);
        // 创建任务并压入协程中
        if(ft.fiber || ft.cb)
        {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    // 协程/函数/线程组
    struct FiberAndThread
    {
        // 协程
        Fiber::ptr fiber;

        // 协程执行函数
        std::function<void()> cb;
        
        // 执行任务的线程ID
        int thread; 

        // 接受一个协程和线程ID
        FiberAndThread(Fiber::ptr f,int thr)
            :fiber(f),thread(thr){}

        // 接受协程指针和线程ID
        FiberAndThread(Fiber::ptr* f,int thr)
            :thread(thr)
        {
            // swap函数将*f（也就是智能指针）和fiber交换，不产生内存分配而交换智能指针的所有权
            fiber.swap(*f);
        }

        // 线程执行函数和线程ID
        FiberAndThread(std::function<void()> f,int thr)
            :cb(f),thread(thr)
        {
            
        }

        FiberAndThread(std::function<void()>* f,int thr)
            :thread(thr)
        {
            cb.swap(*f);
        }
        
        // 声明thread为-1表示创建对象时没有指定有效的线程ID
        FiberAndThread()
            :thread(-1)
        {

        }

        // 重置数据
        void reset()
        {
            fiber=nullptr;
            cb=nullptr;
            thread=-1;
        }
    };
private:
    MutexType m_mutex;                           // 互斥锁
    std::vector<Thread::ptr> m_threads;          // 线程池
    std::list<FiberAndThread> m_fibers;          // 待执行的任务队列
    Fiber::ptr m_rootFiber;                      // 调度协程，当use_caller为true时有效，也就是主协程
    std::string m_name;                          // 协程调度器的名称
protected:
    std::vector<int> m_threadIds;                // 协程下的线程id数组
    size_t m_threadCount=0;                      // 线程数量
    std::atomic<size_t> m_activeThreadCount={0}; // 工作线程数量
    std::atomic<size_t> m_idleThreadCount={0};   // 空闲线程数量
    bool m_stopping=true;                        // 是否正在停止，表示调度器正在停止
    bool m_autoStop=false;                       // 是否自动停止，执行完任务后自动触发停止操作
    int m_rootThread=0;                          // 主线程id(use_caller)
};

class SchedulerSwitcher : public NonCopyable
{
public:
    SchedulerSwitcher(Scheduler* target=nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler* m_caller;
};

}
