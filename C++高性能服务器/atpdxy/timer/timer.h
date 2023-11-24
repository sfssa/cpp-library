#pragma once
#include <memory>
#include <set>
#include <vector>
#include "../thread/thread.h"

namespace atpdxy{

class TimerManager;

class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerManager; // 必须通过管理器创建定时器
public:
    typedef std::shared_ptr<Timer> ptr;

    // 取消定时器
    bool cancel();

    // 刷新设置定时器的执行时间
    bool refresh();

    // 重置定时器的时间间隔
    bool reset(uint64_t ms,bool from_now);
private:
    // 毫秒-回调函数-是否循环执行-定时器对应的管理器
    Timer(uint64_t ms,std::function<void()> cb,
          bool recurring,
          TimerManager* manager);

    Timer(uint64_t next);
private:
    bool m_recurring=false;             // 是否循环定时器
    uint64_t m_ms;                      // 循环执行周期
    uint64_t m_next=0;                  // 精确的执行时间
    std::function<void()> m_cb;         // 回调函数
    TimerManager* m_manager=nullptr;    // 管理器指针
private:
    // 协调比较两个定时器的位置
    struct Comparator{
        bool operator() (const Timer::ptr& lhs,const Timer::ptr& rhs) const;
    };
};

class TimerManager{
friend class Timer;
public:
    typedef RWMutex RWMutexType;
    TimerManager();
    virtual ~TimerManager();
    // 添加定时器
    Timer::ptr addTimer(uint64_t ms,std::function<void()> cb,
                        bool recurring=false);
    
    // 添加条件定时器
    Timer::ptr addConditionTimer(uint64_t ms,std::function<void()> cb,
                                 std::weak_ptr<void> weak_cond,
                                 bool recurring=false);
    // 获取距离下一次触发的时间
    uint64_t getNextTime();

    // 获取所有的超时的任务
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
protected:
    // 当有新的定时器插入到定时器首部执行此函数
    virtual void onTimerInsertedAtFront()=0;

    // 将定时器添加到管理器中
    void addTimer(Timer::ptr val,RWMutexType::WriteLock& lock);
private:
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    // 存放定时器的容器
    std::set<Timer::ptr,Timer::Comparator> m_timers;
    // 是否插入了新的队头
    bool m_tickled=false;
    // 上一次执行的时间，判断是否修改了系统时间
    uint64_t m_previouseTimer=0;
};
}
