/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-20 09:38:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-20 10:15:11
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
#include "../fiber/fiber.h"
#include "../thread/thread.h"

namespace atpdxy
{

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

    const std::string& getName() const {return m_name;}

    // 获得当前协程调度器
    static Scheduler* GetThis(); 

    // 协程调度器主协程   
    static Fiber* GetMainFiber();

    // 启动调度器  
    void start();

    // 停止调度器
    void stop();

    // 将协程或回调安排到执行队列
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
                need_tickle=scheduleNoBlock(&*begin) || need_tickle;
            }
        }
        if(need_tickle)
            tickle();
    }
protected:
    // 唤醒调度器
    virtual void tickle();

private:
    template <class FiberOrCb>
    bool scheduleNoBlock(FiberOrCb fc,int thread)
    {
        bool need_tickle=m_fibers.empty();
        FiberAndThread ft(fc,thread);
        if(ft.fiber || ft.cb)
        {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    struct FiberAndThread
    {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread; // 线程ID
        FiberAndThread(Fiber::ptr f,int thr)
            :fiber(f),thread(thr){}

        FiberAndThread(Fiber::ptr* f,int thr)
            :thread(thr)
        {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f,int thr)
            :cb(f),thread(thr)
        {
            
        }

        FiberAndThread(std::function<void()>* f,int thr)
            :thread(thr)
        {
            cb.swap(*f);
        }
        
        FiberAndThread()
            :thread(-1)
        {

        }

        void reset()
        {
            fiber=nullptr;
            cb=nullptr;
            thread=-1;
        }
    };
private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;
    std::string m_name;
};

}
