<!--
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-21 14:41:21
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-21 15:40:05
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/scheduler/readme.md
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
-->
# Scheduler 类

## 简介

`atpdxy::Scheduler` 类是一个基于协程的调度器，用于在多线程环境中调度协程的执行。它包含了一个线程池，每个线程可以执行一个或多个协程，实现了协程的调度和切换。

## 功能

- **协程调度：** 将协程或回调函数安排到执行队列中，由调度器进行调度执行。
- **线程池支持：** 内部包含一个线程池，支持在多个线程中执行协程。
- **调度器状态管理：** 支持启动、停止调度器，以及在停止时等待协程执行完毕。
- **线程切换：** 提供了线程切换的功能，可以将协程从一个线程切换到另一个线程上执行。
- **调度器信息输出：** 提供了输出调度器状态信息的接口，方便查看调度器的运行状态。

## 类成员

### 构造函数

```
Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
```

- 参数：
  - `threads`：线程池的大小。
  - `use_caller`：是否使用调用线程作为调度器线程之一。
  - `name`：调度器的名称。

### 成员函数

- `void start()`: 启动调度器。
- `void stop()`: 停止调度器，等待协程执行完毕。
- `template <class FiberOrCb> void schedule(FiberOrCb fc, int thread = -1)`: 将协程或回调函数安排到执行队列中。
- `void switchTo(int thread = -1)`: 在不同线程之间切换协程的执行。
- `std::ostream& dump(std::ostream& os)`: 输出调度器的基本信息和线程ID数组。

### 静态函数

- `static Scheduler* GetThis()`: 获取当前线程的协程调度器。
- `static Fiber* GetMainFiber()`: 获取当前线程的协程调度器主协程。

### 内部类

- `SchedulerSwitcher`: 在不同调度器之间进行切换的辅助类。

## 测试代码和流程

### 代码

```
#include "../atpdxy/atpdxy.h"

static atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

void test_fiber()
{
    static int s_count=5;
    ATPDXY_LOG_INFO(g_logger)<<"test in fiber s_count="<<s_count;
    sleep(1);
    if(--s_count>=0) // 指定要执行的线程
        atpdxy::Scheduler::GetThis()->schedule(&test_fiber,atpdxy::getThreadId());
}

int main()
{
    ATPDXY_LOG_INFO(g_logger)<<"main";
    atpdxy::Scheduler sc(3,false,"test");
    sc.start();
    sleep(2);
    ATPDXY_LOG_INFO(g_logger)<<"schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    ATPDXY_LOG_INFO(g_logger)<<"over";
    return 0;
}
```

### 程序流程图：
![logger1](https://github.com/sfssa/cpp-library/blob/master/C%2B%2B%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8/static/scheduler.jpg)