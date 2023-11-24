#include "./timer.h"
#include "../utils/utils.h"

namespace atpdxy{

bool Timer::Comparator::operator() (const Timer::ptr& lhs,const Timer::ptr& rhs) const{
    if(!lhs && !rhs){
        return false;
    }
    if(!lhs){
        return true;
    }
    if(!rhs){
        return false;
    }

    if(lhs->m_next<rhs->m_next){
        return true;
    }
    if(rhs->m_next<lhs->m_next){
        return false;
    }

    // 比较两者地址
    return lhs.get()<rhs.get();
}

Timer::Timer(uint64_t ms,std::function<void()> cb
            ,bool recurring,TimerManager* manager)
    :m_recurring(recurring)
    ,m_ms(ms)
    ,m_cb(cb)
    ,m_manager(manager){
    m_next=atpdxy::GetCurrentMS()+m_ms;
}

Timer::Timer(uint64_t next)
    :m_next(next){
    
}

Timer::ptr TimerManager::addTimer(uint64_t ms,
                                  std::function<void()> cb,
                                  bool recurring){
    Timer::ptr timer(new Timer(ms,cb,recurring,this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer,lock);
    return timer;
}

bool Timer::cancel(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(m_cb){
        m_cb=nullptr;
        // 返回指向当前Timer的智能指针
        auto it=m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb){
        return false;
    }
    auto it=m_manager->m_timers.find(shared_from_this());
    if(it==m_manager->m_timers.end()){
        return false;
    }
    // 删除定时器
    m_manager->m_timers.erase(it);
    // 获取当前毫秒值+自己的执行周期，获得下一次执行的具体时间
    m_next=atpdxy::GetCurrentMS()+m_ms;
    // 将当前定时器插入到容器中
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

// from_now是否需要从当前时间开始计算
bool Timer::reset(uint64_t ms,bool from_now){
    if(ms==m_ms && !from_now){
        return true;
    }

    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb){
        return false;
    }

    auto it=m_manager->m_timers.find(shared_from_this());
    if(it==m_manager->m_timers.end()){
        return false;
    }

    // 删除定时器
    m_manager->m_timers.erase(it);
    uint64_t start=0;
    if(from_now){
        start=atpdxy::GetCurrentMS();
    }else{
        start=m_next-m_ms;
    }

    m_ms=ms;
    m_next=start+m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}

static void OnTimer(std::weak_ptr<void> weak_cond,std::function<void()> cb){
    // 获取shared_ptr，判断执行前对象是否还存在，对象依然存在就执行回调函数
    std::shared_ptr<void> tmp=weak_cond.lock();
    if(tmp){
        cb();
    }
}

TimerManager::TimerManager(){
    m_previouseTimer=atpdxy::GetCurrentMS();
}

TimerManager::~TimerManager(){

}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms,
                                           std::function<void()> cb,
                                           std::weak_ptr<void> weak_cond,
                                           bool recurring){
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
}

// 获取下一次定时器执行回调函数的时间差
uint64_t TimerManager::getNextTime(){
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled=false;
    if(m_timers.empty()){
        return ~0ull;
    }

    const Timer::ptr& next=*m_timers.begin();
    uint64_t now_ms=atpdxy::GetCurrentMS();
    // 如果当前时间大于下次执行时间，时间差为0；否则返回下次执行时间-当前时间
    if(now_ms>=next->m_next){
        return 0;
    }else{
        return next->m_next-now_ms;
    }
}

// 显示所有到时的定时器
void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs){
    uint64_t now_ms=atpdxy::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()){
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    bool rollover=detectClockRollover(now_ms);
    // 第一个定时器的时间都大于当前时间，没有到时的定时器，没有调整系统时间
    if(!rollover && ((*m_timers.begin())->m_next>now_ms)){
        return;
    }

    Timer::ptr now_timer(new Timer(now_ms));
    // 调整了系统时间返回容器结尾，否则返回第一个大于now_timer的容器
    auto it=rollover?m_timers.end():m_timers.lower_bound(now_timer);
    while(it!=m_timers.end() && (*it)->m_next==now_ms){
        ++it;
    }
    expired.insert(expired.begin(),m_timers.begin(),it);
    m_timers.erase(m_timers.begin(),it);
    cbs.reserve(expired.size());
    for(auto& timer:expired){
        // cbs.push_back(timer->m_cb);
        cbs.emplace_back(timer->m_cb);
        if(timer->m_recurring){
            timer->m_next=now_ms+timer->m_ms;
            m_timers.insert(timer);
        }else{
            timer->m_cb=nullptr;
        }

    }
}

void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLock& lock){
    // insert返回pair<iterator,bool>，第一个是迭代器，第二个是是否成功插入
    auto it=m_timers.insert(val).first;
    // 队头插入新的定时器并且没有被处理过
    bool at_front=(it==m_timers.begin()) && !m_tickled;
    if(at_front){
        m_tickled=true;
    }
    lock.unlock();
    // 保证新插入的定时器在最短的时间内被处理
    if(at_front){
        onTimerInsertedAtFront();
    }
}

// 判断系统是否调整了时间
bool TimerManager::detectClockRollover(uint64_t now_ms){
    bool rollover=false;
    // 当前时间小于上次执行时间，并且超过了1H，表示调整了系统的时间
    if(now_ms<m_previouseTimer && 
       now_ms<(m_previouseTimer-60*60*1000)){
        rollover=true;
    }
    // 更新时间
    m_previouseTimer=now_ms;
    return rollover;
}

}
