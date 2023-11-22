/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-20 10:08:19
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-22 17:34:02
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/scheduler/scheduler.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "./scheduler.h"
#include "../log/log.h"
#include "../macro.h"

namespace atpdxy
{

static atpdxy::Logger::ptr g_logger=ATPDXY_LOG_NAME("system");

// 正在执行的调度器
static thread_local Scheduler* t_scheduler=nullptr;

// 正在执行的调度器中的协程
static thread_local Fiber* t_scheduler_fiber=nullptr;

Scheduler::Scheduler(size_t threads,bool use_caller,const std::string& name)
    :m_name(name)
{
    ATPDXY_ASSERT(threads>0);

    // 是否使用调用线程作为主线程
    if(use_caller)
    {
        // 获取当前线程主协程
        atpdxy::Fiber::GetThis();
        --threads;

        // 没有已存在的调度器
        ATPDXY_ASSERT(GetThis()==nullptr);

        // 设置当前调度器
        t_scheduler=this;

        // 创建一个主协程，run函数为协程的执行函数
        // 调度器的名称为传入的名称
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        atpdxy::Thread::SetName(m_name);

        // 设置当前调度器的协程
        t_scheduler_fiber=m_rootFiber.get();

        // 设置主线程ID
        m_rootThread=atpdxy::getThreadId();
        m_threadIds.push_back(m_rootThread);
    }
    else 
        m_rootThread=-1; // 主线程未知

    // 更新线程数量
    m_threadCount=threads;
}

Scheduler::~Scheduler()
{
    // 确保在调用析构函数时，调度器处于停止状态
    ATPDXY_ASSERT(m_stopping);
    
    // 当前调度器等于调用析构函数的对象则释放对象
    if(GetThis()==this)
        t_scheduler=nullptr;
}

// 获得当前线程的协程调度器
Scheduler* Scheduler::GetThis()
{
    return t_scheduler;
}

// 获得当前线程的协程调度器主协程
Fiber* Scheduler::GetMainFiber()
{
    return t_scheduler_fiber;
}

void Scheduler::start()
{
    MutexType::Lock lock(m_mutex);
    // 已经在运行
    if(!m_stopping)
        return;
    m_stopping=false;
    // 初始状态线程池必须为空
    ATPDXY_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i=0;i<m_threadCount;++i)
    {
        //通过调度器的run函数和和i来创建线程并将线程ID存放到容器
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this),m_name+"_"+std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop()
{
    // 设置自动停止为真
    m_autoStop=true;

    // 如果主协程存在，且线程数量为0，主协程状态为TERM/INIT表示调度器已经停止
    if(m_rootFiber && m_threadCount==0 
       && (m_rootFiber->getState()==Fiber::TERM 
       || m_rootFiber->getState()==Fiber::INIT))
    {
        ATPDXY_LOG_INFO(g_logger)<<this<<" stopped";

        m_stopping=true;

        if(stopping())
            return;
    }

    // 如果主协程不=-1，那么判断当前线程执行的调度器是否是调用成员函数的对象
    // 为真意味着当前调度器是正在停止的，否则当前调度器不是正在停止的
    if(m_rootThread!=-1)
    {
        ATPDXY_ASSERT(GetThis()==this);
    }
    else
    {
        ATPDXY_ASSERT(GetThis()!=this);
    }

    // 遍历所有线程唤醒它们
    m_stopping=true;
    for(size_t i=0;i<m_threadCount;++i)
        tickle();

    if(m_rootFiber)
        tickle();

    if(m_rootFiber)
    {
        if(!stopping())
            m_rootFiber->call();
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i:thrs)
        i->join();
}

// 将调用的对象设置为当前线程执行的协程调度器
void Scheduler::setThis()
{
    t_scheduler=this;
}

void Scheduler::run()
{
    ATPDXY_LOG_DEBUG(g_logger)<<m_name<<" run";
    // set_hook_enable(true);
    // 将当前调度器设置为当前线程的调度器
    setThis();

    // 正在执行线程不是主线程，设置当前协程
    if(atpdxy::getThreadId()!=m_rootThread)
        t_scheduler_fiber=Fiber::GetThis().get();

    // 创建一个空闲协程，在没有任务时执行
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
    
    // 创建一个空闲协程，执行回调函数
    Fiber::ptr cb_fiber;

    // 创建任务
    FiberAndThread ft;

    while(true)
    {
        // 重置任务
        ft.reset();
        
        // 是否需要唤醒更多的协程
        bool tickle_me=false;
        // 是否有协程被激活执行
        bool is_active=false;
        {
            MutexType::Lock lock(m_mutex);
            // 获得任务队列的首元素
            auto it=m_fibers.begin();
            while(it!=m_fibers.end())
            {
                // 下面判断不需要执行直接跳过的情况
                // 如果任务有指定线程并且指定线程不是当前正在执行线程，跳过
                if(it->thread!=-1 && it->thread!=atpdxy::getThreadId())
                {
                    ++it;
                    // 需要唤醒其他线程
                    tickle_me=true;
                    continue;
                }

                // 任务有一个协程或者函数指针不为空
                ATPDXY_ASSERT(it->fiber || it->cb);

                // 如果任务协程不为空且协程的状态是正在执行，跳过
                if(it->fiber && it->fiber->getState()==Fiber::EXEC)
                {
                    ++it;
                    continue;
                }

                // 获得协程
                ft=*it;
                // 从任务数组中删除该协程
                m_fibers.erase(it++);
                // 有一个任务将要执行，增加活跃的线程数目
                ++m_activeThreadCount;
                is_active=true;
                break;
            }
            tickle_me |= it!=m_fibers.end();
        }
        // 结合上面的异或，也就是容器的数量不为空，还需要唤醒其他线程
        if(tickle_me)
            tickle();
        
        // 如果是协程，状态不是TERM/EXCEPT，执行该协程
        if(ft.fiber && (ft.fiber->getState()!=Fiber::TERM
                        && ft.fiber->getState()!=Fiber::EXCEPT))
        {
            // 将协程切入
            ft.fiber->swapIn();

            // 任务执行完毕，减少活跃的线程数
            --m_activeThreadCount;
            // 协程状态是READY重新调度
            if(ft.fiber->getState()==Fiber::READY)
            {
                schedule(ft.fiber);
            }
            else if(ft.fiber->getState()!=Fiber::TERM 
                    && ft.fiber->getState()!=Fiber::EXCEPT)
            {
                ft.fiber->m_state=Fiber::HOLD;
            }
            // 清空任务
            ft.reset();
        }
        else if(ft.cb)  // 任务是回调函数
        {
            // 获得回调函数存储到cb_fiber中并释放原来的空间
            if(cb_fiber)
                cb_fiber->reset(ft.cb);
            else
                cb_fiber.reset(new Fiber(ft.cb));
            ft.reset();

            // 执行协程
            cb_fiber->swapIn();

            // 执行完毕后活跃的线程数-1
            --m_activeThreadCount;

            // 执行完后是就绪态，重新调度
            if(cb_fiber->getState()==Fiber::READY)
            {
                schedule(cb_fiber);
                cb_fiber.reset();
            }
            else if(cb_fiber->getState()==Fiber::EXCEPT
                    || cb_fiber->getState()==Fiber::TERM)
                cb_fiber->reset(nullptr); // 任务指针置空，表示没有任务要执行
            else
            {
                cb_fiber->m_state=Fiber::HOLD;
                cb_fiber.reset();
            }
        }
        else
        {
            // 这里处理没有任务的情况，执行空闲协程
            // 是否有活跃的任务，有说明协程或者回调任务正在执行，不需要执行空闲协程
            if(is_active)
            {
                --m_activeThreadCount;
                continue;
            }

            // 如果空闲协程的状态时TERM表示空闲协程已经结束执行，说明整个调度器可以退出了，结束调度器
            // 空闲且空闲协程已经执行结束
            if(idle_fiber->getState()==Fiber::TERM)
            {
                ATPDXY_LOG_INFO(g_logger)<<" idle fiber term";
                break;
            }

            // 增加空闲线程数目后减少空闲线程计数
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;

            // 空闲进程正常执行，将其处于暂停状态，等待下一次唤醒
            if(idle_fiber->getState()!=Fiber::TERM
               && idle_fiber->getState()!=Fiber::EXCEPT)
            {
                idle_fiber->m_state=Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle()
{
    ATPDXY_LOG_INFO(g_logger)<<"tickle";
}

// 调度器正在结束，自动停止为真（任务执行完毕），正在停止为真
// 协程的队列为空，活动的线程数目为0
bool Scheduler::stopping()
{
    MutexType::Lock lock(m_mutex);
    return m_autoStop 
        && m_stopping
        && m_fibers.empty() 
        && m_activeThreadCount==0;
}

// 将协程转换到HOLD状态
void Scheduler::idle()
{
    ATPDXY_LOG_INFO(g_logger)<<"idle";
    while(!stopping())
        atpdxy::Fiber::YieldToHold();
}

// 在不同线程间切换协程的执行
void Scheduler::switchTo(int thread)
{
    // 确保有一个协程调度器
    ATPDXY_ASSERT(Scheduler::GetThis()!=nullptr);

    // 需要调度的就是当前正在执行的调度器，返回
    if(Scheduler::GetThis()==this)
    {
        if(thread==-1 ||  thread==atpdxy::getThreadId())
            return;
    }

    // 将当前协程加入到指定线程执行，并让出执行权
    schedule(Fiber::GetThis(),thread);
    
    // 设置当前的协程状态为HOLD
    Fiber::YieldToHold();
}

// 输出调度器的基本信息和线程ID数组
std::ostream& Scheduler::dump(std::ostream& os)
{
    os<<"[Scheduler name="<<m_name
      <<" size="<<m_threadCount
      <<"active_count="<<m_activeThreadCount
      <<" idle_count="<<m_idleThreadCount
      <<" stopping="<<m_stopping
      <<" ]"<<std::endl<<"    ";

    for(size_t i=0;i<m_threadIds.size();++i)
    {
        if(i)
            os<<",";
        os<<m_threadIds[i];
    }
    return os;
}

// 从当前协程调度器切换到目标协程调度器
SchedulerSwitcher::SchedulerSwitcher(Scheduler* target)
{
    // 获得当前线程调度器
    m_caller=Scheduler::GetThis();
    if(target)
        target->switchTo();
}

// 检查调度器是否存在，是就将协程切换回之前的调度器
SchedulerSwitcher::~SchedulerSwitcher()
{
    if(m_caller)
        m_caller->switchTo();
}

}
