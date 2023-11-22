/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-21 18:44:56
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-22 15:40:08
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/iomanager/iomanager.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "./iomanager.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "../log/log.h"
#include "../macro.h"

namespace atpdxy
{

static atpdxy::Logger::ptr g_logger=ATPDXY_LOG_NAME("system");

// 线程数量-是否将调用线程包含-调度器的名称
IOManager::IOManager(size_t threads,bool use_caller,const std::string& name)
    :Scheduler(threads,use_caller,name) // 构造基类部分
{
    // 创建epoll事件表
    m_epfd=epoll_create(5000);
    ATPDXY_ASSERT(m_epfd>0);

    // 创建通信管道
    int ret=pipe(m_tickleFds);
    ATPDXY_ASSERT(!ret);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    // 关联的文件描述符可读并采用边沿触发
    event.events=EPOLLIN | EPOLLET;
    event.data.fd=m_tickleFds[0];
    // F_SETFL-设置文件属性，将pipe[0]设置为非阻塞
    ret=fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);
    ATPDXY_ASSERT(!ret);

    // 将m_tickerFds[0]添加到epoll事件中，有数据可读时会触发二poll事件
    ret=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
    ATPDXY_ASSERT(!ret);
    // m_fdContexts.resize(64);

    // 重置容器大小
    contextResize(32);
    start();
}

// 析构函数
IOManager::~IOManager()
{
    // 停止协程调度器
    stop();
    // 关闭epoll事件表
    close(m_epfd);
    // 关闭管道
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    // 删除socket事件的指针
    for(size_t i=0;i<m_fdContexts.size();++i)
    {
        if(m_fdContexts[i])
            delete m_fdContexts[i];
    }
}

// 重新定义事件的大小
void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);
    for(size_t i=0;i<m_fdContexts.size();++i)
    {
        if(!m_fdContexts[i])
        {
            m_fdContexts[i]=new FdContext;
            m_fdContexts[i]->fd=i;
        }

    }
}

// 添加事件：socket句柄-事件类型-时间回调函数
int IOManager::addEvent(int fd,Event event,std::function<void()> cb)
{
    // 取出上下文
    FdContext* fd_ctx=nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()>fd)
    {
        fd_ctx=m_fdContexts[fd];       
        lock.unlock();
    }
    else 
    {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd*1.5);
        fd_ctx=m_fdContexts[fd];
    }

    // 设置状态
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    // 已经有该事件了
    if(fd_ctx->events & event)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"addEvent assert fd="<<fd
            <<" event="<<(EPOLL_EVENTS)event
            <<" fd_ctx.event="<<(EPOLL_EVENTS)fd_ctx->events;

        ATPDXY_ASSERT(!(fd_ctx->events & event));
    }
    // 判断修改还是添加
    int op=fd_ctx->events?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
    epoll_event epevent;
    // 原来的事件加上新的事件
    epevent.events=EPOLLET | fd_ctx->events | event;
    epevent.data.ptr=fd_ctx;
    
    int ret=epoll_ctl(m_epfd,op,fd,&epevent);
    if(ret)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"epollctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<ret<<" ("<<errno<<") ("<<strerror(errno)<<") fd_ctx=>events="
            <<fd_ctx->events;
        return -1;
    }
    // 添加事件，等待处理的任务数量+1
    ++m_pendingEventCount;
    fd_ctx->events=(Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
    
    // 有调度器/协程/回调函数
    ATPDXY_ASSERT(!event_ctx.scheduler
        && !event_ctx.fiber
        && !event_ctx.cb);

    event_ctx.scheduler=Scheduler::GetThis();
    if(cb)
    {
        event_ctx.cb.swap(cb);
    }
    else
    {
        event_ctx.fiber=Fiber::GetThis();
        // ATPDXY_ASSERT2(event_ctx.fiber->getState()==Fiber::EXEC
                       // ,"state="<<event_ctx.fiber->getState());
        ATPDXY_ASSERT(event_ctx.fiber->getState()==Fiber::EXEC);
    }
    return 0;
}

// 删除事件：socket句柄-事件类型-不会触发事件
bool IOManager::delEvent(int fd,Event event)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()<=fd)
        return false;
    
    FdContext* fd_ctx=m_fdContexts[fd];
    lock.unlock();

    // 删除的事件在注册表中没有
    FdContext::MutexType::Lock lock1(fd_ctx->mutex);
    if(!(fd_ctx->events & event))
    {
        return false;
    }

    Event new_events=(Event)(fd_ctx->events & ~event);
    int op=new_events ? EPOLL_CTL_MOD:EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events=EPOLLET | new_events;
    epevent.data.ptr=fd_ctx;

    int ret=epoll_ctl(m_epfd,op,fd,&epevent);
    if(ret)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<ret<<" ("<<errno<<") ("<<strerror(errno)<<")";
        return false;
    }
    --m_pendingEventCount;
    fd_ctx->events=new_events;
    FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}
// 取消事件：socket句柄-事件类型-如果事件存在则触发事件
bool IOManager::cancelEvent(int fd,Event event)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()<=fd)
        return false;

    FdContext* fd_ctx=m_fdContexts[fd];
    lock.unlock();

    // 删除的事件在注册表中没有
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event))
    {
        return false;
    }

    Event new_events=(Event)(fd_ctx->events & ~event);
    int op=new_events ? EPOLL_CTL_MOD:EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events=EPOLLET | new_events;
    epevent.data.ptr=fd_ctx;

    int ret=epoll_ctl(m_epfd,op,fd,&epevent);
    if(ret)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<ret<<" ("<<errno<<") ("<<strerror(errno)<<")";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

// 取消所有事件
bool IOManager::cancelAll(int fd)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()<=fd)
        return false;

    FdContext* fd_ctx=m_fdContexts[fd];
    lock.unlock();

    // 删除的事件在注册表中没有
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events))
    {
        return false;
    }

    int op=EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events=0;
    epevent.data.ptr=fd_ctx;

    int ret=epoll_ctl(m_epfd,op,fd,&epevent);
    if(ret)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<ret<<" ("<<errno<<") ("<<strerror(errno)<<")";
        return false;
    }

    if(fd_ctx->events & READ)
    {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }

    if(fd_ctx->events & WRITE)
    {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    ATPDXY_ASSERT(fd_ctx->events==0);
    return true;
}
// 获取当前IOmanager
IOManager* IOManager::GetThis()
{
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event)
{
    switch(event)
    {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            ATPDXY_ASSERT2(false,"getContext");
    }
}

void IOManager::FdContext::resetContext(EventContext& ctx)
{
    ctx.scheduler=nullptr;
    ctx.fiber.reset();
    ctx.cb=nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event)
{
    ATPDXY_ASSERT(events & event);
    events=(Event)(events & ~event);
    EventContext& ctx=getContext(event);
    // 引用的方式会释放
    if(ctx.cb)
    {
        ctx.scheduler->schedule(&ctx.cb);
    }
    else
    {
        ctx.scheduler->schedule(&ctx.fiber);
    }

    ctx.scheduler=nullptr;
    return;
}

void IOManager::tickle()
{
    // if(!hasIdleThreads())
    if(hasIdleThreads())
        return;

    int ret=write(m_tickleFds[1],"T",1);
    ATPDXY_ASSERT(ret==1);
    
}

bool IOManager::stopping(uint64_t& timeout)
{
    return true;
}

bool IOManager::stopping()
{
    // uint64_t timeout=0;
    // return stopping(timeout);
    return Scheduler::stopping()
        && m_pendingEventCount==0;
}

void IOManager::idle()
{
    // 创建一个数组智能指针
    epoll_event* events=new epoll_event[64]();
    
    std::shared_ptr<epoll_event> shared_events(events,[](epoll_event* ptr){
        delete[] ptr;
    });

    while(true)
    {
        if(stopping())
        {
            ATPDXY_LOG_INFO(g_logger)<<"name="<<getName()<<" idle stopping exit";
            break;        
        }

        int ret=0;
        do{
            static const int MAX_TIMEOUT=5000;
            ret=epoll_wait(m_epfd,events,64,MAX_TIMEOUT);
            
            if(ret<0 && errno==EINTR)
            {
                                
            }
            else
            {
                break;
            }
        }while(true);

        for(int i=0;i<ret;++i)
        {
            epoll_event& event=events[i];
            if(event.data.fd==m_tickleFds[0])
            {
                uint8_t dummy;
                while(read(m_tickleFds[0],&dummy,1)==1);
                continue;        
            }
            FdContext* fd_ctx=(FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            // 错误或者中断
            if(event.events & (EPOLLERR | EPOLLHUP))
            {
                event.events |= EPOLLIN | EPOLLOUT;
            }

            int real_events=NONE;
            if(event.events & EPOLLIN)
            {
                real_events |= READ;
            }

            if(event.events & EPOLLOUT)
            {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events)==NONE)
                continue;

            // 剩余的事件
            int left_events=(fd_ctx->events & ~real_events);
            int op=left_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
            event.events=EPOLLET | left_events;

            int ret2=epoll_ctl(m_epfd,op,fd_ctx->fd,&event);
            if(ret2)
            {
                ATPDXY_LOG_ERROR(g_logger)<<"epollctl("<<m_epfd<<", "
                    <<op<<","<<fd_ctx->fd<<","<<event.events<<"):"
                    <<ret2<<" ("<<errno<<") ("<<strerror(errno)<<")";
                continue;
            }

            if(real_events & READ)
            {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }

            if(real_events & WRITE)
            {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }
        
        Fiber::ptr cur=Fiber::GetThis();
        auto raw_ptr=cur.get();
        cur.reset();
        raw_ptr->swapOut();
    }
}

}
