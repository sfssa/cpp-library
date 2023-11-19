#include "./fiber.h"
#include <atomic>
#include "../macro.h"
#include "../config/config.h"
#include "../log/log.h"

namespace atpdxy
{
// 日志对象
static Logger::ptr g_logger=ATPDXY_LOG_NAME("system");

// 为每个协程分配唯一ID
static std::atomic<uint64_t> s_fiber_id {0};

// 跟踪协程的总数
static std::atomic<uint64_t> s_fiber_count {0};

// 线程局部变量，跟踪正在执行的协程
static thread_local Fiber* t_fiber=nullptr;

// 线程局部变量，存储当前线程的主协程，也就是程序的入口点，分则整个程序的初始化和管理
static thread_local Fiber::ptr t_threadFiber=nullptr;

// 配置协程的栈大小为1M
static ConfigVar<uint32_t>::ptr g_fiber_stack_size=
    Config::lookUp<uint32_t>("fiber.stack_size",1024*1024,"fiber stack size");

// 简单的堆栈分配器类，用于分配和释放内存
class MallocStackAllocator
{
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }

    static void Dealloc(void* vp,size_t size)
    {
        return free(vp);
    }
};

using StackAllocator=MallocStackAllocator;

Fiber::Fiber()
{
    m_state=EXEC;
    SetThis(this);

    // 保存上下文
    if(getcontext(&m_ctx))
    {
        ATPDXY_ASSERT2(false,"getcontext");
    }

    // 协程总数增加
    ++s_fiber_count;
    ATPDXY_LOG_DEBUG(g_logger)<<"Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb,size_t stacksize)
    :m_id(++s_fiber_id),m_cb(cb)
{
    // 协程总数+1
    ++s_fiber_count;

    // 设置协程栈大小
    m_stacksize=stacksize?stacksize:g_fiber_stack_size->getValue();
    
    // 开辟协程栈空间
    m_stack=StackAllocator::Alloc(m_stacksize);

    // 保存协程上下文
    if(getcontext(&m_ctx))
    {
        ATPDXY_ASSERT2(false,"getcontext");
    }
    
    // 设置协程上下文
    m_ctx.uc_link=nullptr;

    // 设置栈指针
    m_ctx.uc_stack.ss_sp=m_stack;

    // 设置栈大小
    m_ctx.uc_stack.ss_size=m_stacksize;

    // 设置协程执行函数
    makecontext(&m_ctx,&Fiber::MainFunc,0);

    ATPDXY_LOG_DEBUG(g_logger)<<"Fiber::Fiber id= "<<m_id;
}

/*
    如果有堆栈，那么协程是一个正常创建的子协程对象，协程状态必须是TERM/INIT/EXCEPT，确保协程状态的合法性
    
    如果没有堆栈，表示这是主协程对象（没有堆栈，使用当前线程的堆栈），需要确保回调函数为空，因为主协程没有
    执行函数，确保主协程是EXEC状态，如果当前线程的协程就是要析构的协程对象，就将当前线程设置为空
*/
Fiber::~Fiber()
{
    // 协程总数-1
    --s_fiber_count;

    // 协程有堆栈
    if(m_stack)
    {
        // 协程状态必须是结束、初始化或异常
        ATPDXY_ASSERT(m_state==TERM ||
                      m_state==INIT ||
                      m_state==EXCEPT);
        // 释放堆栈空间
        StackAllocator::Dealloc(m_stack,m_stacksize);
    }
    else 
    {
        // 协程没有堆栈，那么回调函数必须为空；协程必须是执行状态
        ATPDXY_ASSERT(!m_cb);
        ATPDXY_ASSERT(m_state==EXEC);

        // 获取当前协程，如果当前协程是要析构的协程，将协程设置为nullptr
        Fiber* cur=t_fiber;
        if(cur==this)
            SetThis(nullptr);
    }
    ATPDXY_LOG_DEBUG(g_logger)<<"Fiber::~Fiber id= "<<m_id;
}

uint64_t Fiber::GetFiberId()
{
    // 获取线程中正在执行的协程的ID
    if(t_fiber)
        return t_fiber->getId();
    return 0;
}

void Fiber::reset(std::function<void()> cb)
{
    ATPDXY_ASSERT(m_stack);
    // 是子协程
    ATPDXY_ASSERT(m_state==TERM ||
                  m_state==INIT ||
                  m_state==EXCEPT);

    m_cb=cb;
    // 获取上下文信息
    if(getcontext(&m_ctx))
        ATPDXY_ASSERT2(false,"getcontext");

    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_sp=m_stack;
    m_ctx.uc_stack.ss_size=m_stacksize;
    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state=INIT;
}

void Fiber::swapIn()
{
    // 将this对象设置为正在执行协程
    SetThis(this);

    // 设置协程状态为运行
    ATPDXY_ASSERT(m_state!=EXEC);
    m_state=EXEC;
    // 交换上下文
    if(swapcontext(&t_threadFiber->m_ctx,&m_ctx))
    {
        ATPDXY_ASSERT2(false,"swapcontext");
    }
}


void Fiber::swapOut()
{
    // 将主协程设置为正在执行协程
    SetThis(t_threadFiber.get());

    // 切换上下文
    if(swapcontext(&m_ctx,&t_threadFiber->m_ctx))
    {
        ATPDXY_ASSERT2(false,"swapcontext"); 
    }
}

// 将f设置为正在执行协程
void Fiber::SetThis(Fiber* f)
{
    t_fiber=f;
}


// 获取当前线程中正在执行的协程对象，如果已经初始化协程，将对象返回，否则新建一个主协程并返回 
Fiber::ptr Fiber::GetThis()
{
    // 已经有一个正在执行的协程
    if(t_fiber)
        return t_fiber->shared_from_this();

    // 第一次调用GetThis函数，创建新的主协程
    Fiber::ptr main_fiber(new Fiber);
    // 确保当前线程的协程指针设置为主协程
    // ATPDXY_ASSERT(t_fiber==main_fiber.get());
    // 这里我查到的解释是检查t_fiber是否为空，为空就将main_fiber的裸指针赋值给t_fiber
    if (!t_fiber)
        t_fiber = main_fiber.get();
    // 设置主协程
    t_threadFiber=main_fiber;
    // 返回新建的主协程
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady()
{
    // 正在执行协程转换为READY，然后将主协程设置为正在执行协程
    Fiber::ptr cur=GetThis();
    cur->m_state=READY;
    cur->swapOut();
}

void Fiber::YieldToHold()
{
    // 正在执行协程转换为HOLD，然后将主协程设置为正在执行协程
    Fiber::ptr cur=GetThis();
    cur->m_state=HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}

void Fiber::MainFunc()
{
    // 获得正在执行协程
    Fiber::ptr cur=GetThis();
    ATPDXY_ASSERT(cur);
    try
    {
        // 执行函数，执行完毕后设置为空并改变协程状态
        cur->m_cb();
        cur->m_cb=nullptr;
        cur->m_state=TERM;
    }
    catch(std::exception& ex)
    {
        cur->m_state=EXCEPT;
        ATPDXY_LOG_ERROR(g_logger)<<"Fiber Except: "<<ex.what();
    }
    catch(...)
    {
        cur->m_state=EXCEPT;
        ATPDXY_LOG_ERROR(g_logger)<<"Fiber Except: ";
    }

    // 将当前协程对象的智能指针转换为裸指针
    auto raw_ptr=cur.get(); 
    // 释放当前协程的智能指针，可能导致对象的销毁
    cur.reset();
    // 将正在执行的协程切换出去
    raw_ptr->swapOut();
    ATPDXY_ASSERT2(false,"never reach");
}

}
