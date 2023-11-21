#pragma once

#include "../scheduler/scheduler.h"

namespace atpdxy
{
class IOManager:public Scheduler
{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef Mutex RWMutexType;
    
    //  IO事件
    enum Event
    {
        NONE    =   0x0,    // 无事件
        READ    =   0x1,    // 读事件
        WRITE   =   0x2     // 写事件
    };

private:
    // 
    struct FdContext
    {
        typedef Mutex MutexType;

        struct EventContext
        {
            Scheduler* scheduler;       // 事件执行的调度器
            Fiber::ptr fiber;           // 事件的协程
            std::function<void()> cb;   // 事件的回调函数
        };

        int m_fd=0;                     // 事件关联的句柄
        EventContext read;              // 读事件
        EventContext write;             // 写事件
        Event m_events=NONE;            // 已经注册的事件
        MutexType mutex;                // 互斥锁
    };
public:
    // 线程数量-是否将调用线程包含-调度器的名称
    IOManager(size_t threads=1,bool use_caller=true,const std::string& name="");

    // 析构函数
    ~IOManager();

    // 添加事件：socket句柄-事件类型-时间回调函数
    int addEvent(int fd,Event event,std::function<void()> cb=nullptr);

    // 删除事件：socket句柄-事件类型-不会触发事件
    bool delEvent(int fd,Event event);

    // 取消事件：socket句柄-事件类型-如果事件存在则触发事件
    bool cancelEvent(int fd,Event event);

    // 取消所有事件
    bool cancelAll(int fd);

    // 获取当前IOmanager
    static IOManager* GetThis();
protected:
    // 唤醒线程执行
    void tickle() override;

    // 停止调度器执行
    bool stopping() override;

    // 没有任务时执行的协程
    void idle() override;

    // 重置事件大小
    void contextResize(size_t size);
private:
    // epoll文件句柄
    int m_epfd=0;
    // pipe文件句柄
    int m_tickleFds[2];
    // 当前等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount;
    // 互斥锁
    RWMutexType m_mutex;
    // socket事件上下文的容器
    std::vector<FdContext*> m_fdContexts;
};

}
