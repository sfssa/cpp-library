/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-21 18:44:56
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-21 19:24:21
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/iomanager/iomanager.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "./iomanager.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
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
    ATPDXY_ASSERT(ret);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    // 关联的文件描述符可读并采用边沿触发
    event.events=EPOLLIN | EPOLLET;
    event.data.fd=m_tickleFds[0];
    // F_SETFL-设置文件属性，将pipe[0]设置为非阻塞
    ret=fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);
    ATPDXY_ASSERT(ret);

    // 将m_tickerFds[0]添加到epoll事件中，有数据可读时会触发二poll事件
    ret=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
    ATPDXY_ASSERT(ret);
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
    // 这个扩充将原来的FdContext都重置了？？？？
    // 扩充后依然小于fd怎么办？？？？

    // 扩充的空间不小于原来的空间
    if(size<=m_fdContexts.size())
        return;
    m_fdContexts.resize(size);
    for(size_t i=0;i<m_fdContexts.size();++i)
    {
        if(m_fdContexts[i])
        {
            m_fdContexts[i]=new FdContext;
            m_fdContexts[i]->m_fd=i;
        }

    }
}

// 添加事件：socket句柄-事件类型-时间回调函数
int IOManager::addEvent(int fd,Event event,std::function<void()> cb)
{
    // 取出上下文
    FdContext* fd_ctx=nullptr;
    RWMutexType::Lock lock(m_mutex);
    if(m_fdContexts.size()>fd)
    {
        fd_ctx=m_fdContexts[fd];       
        lock.unlock();
    }
    else 
    {
        lock.unlock();
        RWMutexType::Lock lock2(m_mutex);
        // while(m_fdContexts<=size)
        //     contextResize(m_fdContexts.size()*1.5);
        contextResize(m_fdContexts.size()*1.5);
        fd_ctx=m_fdContexts[fd];
    }

    // 设置状态
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(fd_ctx->m_events & event)
    {
        ATPDXY_LOG_ERROR(g_logger)<<"ddd";
    }
}

// 删除事件：socket句柄-事件类型-不会触发事件
bool IOManager::delEvent(int fd,Event event)
{

}

// 取消事件：socket句柄-事件类型-如果事件存在则触发事件
bool IOManager::cancelEvent(int fd,Event event)
{

}

// 取消所有事件
bool IOManager::cancelAll(int fd)
{

}
// 获取当前IOmanager
IOManager* IOManager::GetThis()
{

}

}
