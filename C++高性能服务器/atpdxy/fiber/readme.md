# `Fiber` 类详解

在协程的实现中，存在一个主协程（主线程协程），用于管理和调度其他子协程，主协程是在线程启动时创建的，和线程绑定，负责调度其他协程的执行。

```
class Fiber
{

public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State
    {
		INIT, // 初始化状态
        HOLD, // 阻塞状态
        EXEC, // 执行状态
        TERM, // 执行完毕状态
        READY,// 就绪状态
        EXCEPT// 出错状态
    };
private:

    // 默认构造函数，用于创建主协程
    Fiber();
public:

    // 带有回调函数和栈大小的构造函数，用于创建子协程
    Fiber(std::function<void()> cb, size_t stacksize = 0);

    // 析构函数，用于协程的资源释放
    ~Fiber();

    // 重置协程的执行函数并重置状态为INIT/TERM
    void reset(std::function<void()> cb);

    // 切换到当前协程执行
    void swapIn();

    // 把当前协程切换到后台
    void swapOut();

    // 设置当前线程的协程
    static void SetThis(Fiber* f);

    // 返回当前执行点的协程
    static Fiber::ptr GetThis();

    // 协程切换到后台并设置为Ready状态
    static void YieldToReady();

    // 协程切换后台并设置为Hold状态
    static void YieldToHold();

    // 获取总协程数
    static uint64_t TotalFibers();

    // 获取当前协程的ID
    static uint64_t GetFiberId();

    // 主协程执行函数
    static void MainFunc();
};
```

这个类是一个协程实现，其中包括了协程的创建、切换、状态管理等功能。主要成员函数包括：

- 构造函数和析构函数用于协程的初始化和资源释放。
- `reset` 方法用于重置协程的执行函数和状态。
- `swapIn` 和 `swapOut` 方法用于协程的切换。
- `SetThis` 和 `GetThis` 方法用于设置和获取当前线程的协程。
- `YieldToReady` 和 `YieldToHold` 方法用于将协程切换到后台，并设置为 Ready 或 Hold 状态。
- `TotalFibers` 方法用于获取总协程数。
- `GetFiberId` 方法用于获取当前协程的ID。
- `MainFunc` 方法是协程的执行函数，其中包含了协程的具体逻辑。

# ucontext函数说明

## getcontext

`getcontext` 是一个 POSIX 标准的系统调用，用于获取当前上下文（context）的状态，并将其保存在提供的 `ucontext_t` 结构体中。这个函数通常用于实现用户级线程（协程）的上下文切换。

以下是 `getcontext` 函数的主要特征和作用：

```
int getcontext(ucontext_t *ucp);
```

- **参数：**
  - `ucp`：一个指向 `ucontext_t` 结构体的指针，用于存储当前上下文的状态。
- **返回值：**
  - 如果成功，返回 0；如果失败，返回 -1。
- **作用：**
  - 获取当前执行线程（或协程）的上下文状态，包括寄存器、堆栈指针等信息，并将其保存在提供的 `ucontext_t` 结构体中。
- **注意事项：**
  - `getcontext` 函数获取的上下文是当前执行点的状态，通常在创建协程时首次调用，将该上下文保存起来，以备后续恢复协程执行点。

这个函数通常与 `makecontext` 和 `swapcontext` 一起使用，用于实现用户级线程（协程）的上下文切换。在你的代码中，它被用于初始化协程的上下文，以便在后续的 `swapcontext` 中进行协程的切换。

## ucontext_t结构体

1. **`m_ctx.uc_link=nullptr;`：**
   - `uc_link` 是 `ucontext_t` 结构体中的一个字段，表示协程运行结束后将要切换到的上下文。在这里，将 `uc_link` 设置为 `nullptr`，表示协程运行结束后不切换到其他上下文，即协程运行完毕后结束。
2. **`m_ctx.uc_stack.ss_sp=m_stack;`：**
   - `uc_stack` 是 `ucontext_t` 结构体中的另一个字段，它是一个 `stack_t` 结构体，表示协程的堆栈信息。
   - `ss_sp` 表示堆栈的起始地址，这里设置为 `m_stack`，即协程对象中保存的堆栈指针。
3. **`m_ctx.uc_stack.ss_size=m_stacksize;`：**
   - `ss_size` 表示堆栈的大小，这里设置为 `m_stacksize`，即协程对象中保存的堆栈大小。
4. **`makecontext(&m_ctx, &Fiber::MainFunc, 0);`：**
   - `makecontext` 函数用于将一个新的上下文与执行函数关联起来，并指定参数个数。
   - 第一个参数是 `ucontext_t` 结构体，即当前协程的上下文。
   - 第二个参数是协程执行的函数指针，这里指定为 `&Fiber::MainFunc`，即协程的执行函数。
   - 第三个参数是执行函数的参数个数，这里设置为 0，表示协程的执行函数不接受任何参数。

这段代码的主要作用是初始化协程的上下文，包括堆栈信息和执行函数。当协程第一次被创建时，这些设置将决定协程的运行环境。随后，协程的状态和执行点可以通过 `swapcontext` 等函数进行调度和切换。

## swapcontext

`swapcontext` 是一个系统调用，用于切换两个不同执行上下文之间的控制流。在协程的实现中，通常用于实现协程的切换。这个函数在 `<ucontext.h>` 头文件中声明。

具体而言，`swapcontext` 用于保存当前执行上下文，并切换到另一个执行上下文。其原型如下：

```
int swapcontext(ucontext_t *restrict oucp, const ucontext_t *restrict ucp);
```

- `oucp`：指向保存当前执行上下文的结构体 `ucontext_t` 的指针。
- `ucp`：指向将要切换到的执行上下文的结构体 `ucontext_t` 的指针。

该函数会保存当前执行上下文到 `oucp` 指向的结构体，并切换到 `ucp` 指向的结构体所描述的执行上下文。这样，控制流就从调用 `swapcontext` 的地方切换到了新的执行上下文。

在协程切换的场景中，通常有两个协程，一个是当前协程（调用 `swapcontext` 的协程），另一个是目标协程（即将切换到的协程）。`swapcontext` 的调用会将当前协程的执行状态保存到指定的上下文，然后切换到目标协程的上下文，使得目标协程开始执行。

需要注意的是，对于在多线程环境中使用 `swapcontext` 的情况，可能会遇到一些限制和问题。在使用时应仔细了解文档和可能的线程安全性问题。

# 执行流程

测试代码：

```
atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

void run_in_fiber()
{
    ATPDXY_LOG_INFO(g_logger)<<"run_in_fiber begin";
    atpdxy::Fiber::YieldToHold();
    ATPDXY_LOG_INFO(g_logger)<<"run_in_fiber end";
    atpdxy::Fiber::YieldToHold();
}

void test_fiber()
{
    ATPDXY_LOG_INFO(g_logger)<<"main begin -1";
    {
        atpdxy::Fiber::GetThis();
        ATPDXY_LOG_INFO(g_logger)<<"main begin";
        atpdxy::Fiber::ptr fiber(new atpdxy::Fiber(run_in_fiber));
        fiber->swapIn();
        ATPDXY_LOG_INFO(g_logger)<<"main after swapIn";
        fiber->swapIn();
        ATPDXY_LOG_INFO(g_logger)<<"main after end";
        fiber->swapIn();
    }
    ATPDXY_LOG_INFO(g_logger)<<"main after end2";
}
int main()
{
    atpdxy::Thread::SetName("main");
    std::vector<atpdxy::Thread::ptr> thrs;
    for(int i=0;i<3;++i)
    {
        thrs.push_back(atpdxy::Thread::ptr(new atpdxy::Thread(&test_fiber,"name_"+std::to_string(i))));
    }

    for(auto i: thrs)
        i->join();

    return 0;
}
```

这段测试代码创建了三个线程，每个线程首先通过 `atpdxy::Fiber::GetThis()` 获得主协程，然后在主协程中创建了一个新的子协程（`fiber` 对象），并执行了 `run_in_fiber` 函数。在 `test_fiber` 函数中，通过 `fiber->swapIn()` 实现了协程的切入和切出。

具体的流程如下：

1. 每个线程在初始阶段通过 `atpdxy::Fiber::GetThis()` 获取主协程对象。
2. 在主协程中，通过创建 `fiber` 对象，并在 `fiber` 中执行 `run_in_fiber` 函数。
3. `fiber->swapIn()` 实现了协程的切入，执行 `run_in_fiber` 函数。
4. 在 `run_in_fiber` 函数中，执行完一部分逻辑后通过 `atpdxy::Fiber::YieldToHold()` 实现了协程的主动切出。
5. 回到主协程后，再次调用 `fiber->swapIn()` 实现了协程的切入，继续执行 `run_in_fiber` 函数的剩余部分。
6. 重复上述步骤，实现了协程的交替执行。

在协程的设计中，`swapIn` 和 `swapOut` 的过程是通过操作上下文（context）实现的。`swapIn` 将当前协程的上下文保存起来，然后切换到另一个协程的上下文，从而实现协程的切入。而 `swapOut` 就是将当前协程的上下文保存起来，然后再切换回主协程或其他协程的上下文，实现协程的切出。

当一个协程执行了 `swapOut` 后，它的上下文被保存起来，然后通过 `swapcontext` 函数切换到其他协程（可能是主协程，也可能是其他子协程）。这样，其他协程的上下文就开始执行，实现了协程的交替执行。

