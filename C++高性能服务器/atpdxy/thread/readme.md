# 线程模块

这个模块提供了一套简单的线程控制和同步原语，用于多线程环境中的编程。以下是该模块的主要组成部分和功能：

## 1. Semaphore（信号量）

`Semaphore` 类实现了信号量机制，用于线程之间的同步。主要包括等待和通知两个操作。

- `Semaphore(uint32_t count=0)`：构造函数，初始化信号量，可选择设置初始计数值。
- `~Semaphore()`：析构函数，释放信号量资源。
- `void wait()`：等待操作，如果计数值大于零，就将计数值减一；否则，线程阻塞等待。
- `void notify()`：通知操作，将信号量的计数值加一，唤醒一个等待的线程。

**作用：** 提供一种线程同步机制，用于控制多个线程之间的访问顺序。Semaphore 维护一个计数器，线程通过 `wait()` 操作阻塞自己，直到计数器不为零；通过 `notify()` 操作释放资源，增加计数器。

## 2. Mutex（互斥锁）

`Mutex` 类实现了互斥锁，用于线程之间的互斥操作。

- `Mutex()`：构造函数，初始化互斥锁。
- `~Mutex()`：析构函数，销毁互斥锁。
- `void lock()`：上锁操作，如果锁已经被其他线程占用，则当前线程阻塞等待。
- `void unlock()`：解锁操作，释放互斥锁。

**作用：** 提供一种互斥机制，确保同一时刻只有一个线程可以访问被保护的共享资源。Mutex 对象有两个状态：锁定和未锁定。线程在访问共享资源前需要先锁定 Mutex，使用完毕后再解锁。

## 3. NullMutex（空锁）

`NullMutex` 类是一个空锁，不执行任何实际的互斥操作。主要用于验证 logger 输出是否有重叠。

**作用：** 一个不执行实际互斥操作的空锁，用于验证 logger 输出是否有重叠。对于某些场景，可以使用 NullMutex 替代 Mutex 以减小锁开销。

## 4. RWMutex（读写锁）

`RWMutex` 类实现了读写锁，支持多个线程同时读，但只允许一个线程写。

- `ReadScopeLockImpl` 和 `WriteScopeLockImpl` 分别实现了读锁和写锁的 RAII 封装。
- `void rdlock()`：上读锁。
- `void wrlock()`：上写锁。
- `void unlock()`：解锁。

**作用：** 提供读写分离的锁，允许多个线程同时读取共享资源，但只允许一个线程写入。通过 `ReadScopeLockImpl` 和 `WriteScopeLockImpl` 实现 RAII 封装。

## 5. NullRWMutex（空读写锁）

`NullRWMutex` 类是一个空的读写锁，不执行任何实际的读写锁操作。

**作用：** 一个不执行实际读写锁操作的空读写锁，用于验证 logger 输出是否有重叠。类似于 NullMutex，用于减小锁开销。

## 6. Spinlock（自旋锁）

`Spinlock` 类实现了自旋锁，不会阻塞线程，而是忙等直到获取锁为止。

- `void lock()`：上锁。
- `void unlock()`：解锁。

**作用：** 提供一种不阻塞线程而是忙等的锁机制，适用于对共享资源的访问时间很短的情况。线程在无法获得锁时会循环等待。

## 7. CASLock（自旋锁）

`CASLock` 类实现了自旋锁，使用原子操作确保对共享变量的原子读写。

- `void lock()`：上锁。
- `void unlock()`：解锁。

**作用：** 使用 CAS（Compare and Swap）指令实现的自旋锁，确保对共享变量的原子读写。类似于 Spinlock，但使用了更底层的原子操作。

## 8. Thread（线程）

`Thread` 类封装了线程的创建、执行和管理。

- `Thread(std::function<void()> cb, const std::string& name)`：构造函数，创建线程对象并指定线程执行的函数和名称。

- `~Thread()`：析构函数，负责线程的资源管理。

- `void join()`：等待线程执行结束。

- `pid_t getId() const`：获取线程的 ID。

- `const std::string& getName()`：获取线程的名称。

- `static Thread* GetThis()`：获取当前线程对象。

- `static const std::string& GetName()`：获取当前线程的名称。

  `static void SetName(const std::string& name)`：设置当前线程的名称。

**作用：** 封装线程的创建、执行和管理。通过设置线程执行的函数和线程名称，提供了对线程的方便管理和操作，包括等待线程结束、获取线程 ID 和名称等功能。

## 使用示例：

```
#include "thread.h"

void threadFunc() {
    // 线程执行的具体操作
}

int main() {
    atpdxy::Thread::ptr thread = std::make_shared<atpdxy::Thread>(threadFunc, "example_thread");
    thread->join();  // 等待线程执行结束
    return 0;
}
```

## 补充

### thread_local介绍

`thread_local` 是 C++11 引入的关键字，用于声明线程局部存储（Thread-Local Storage，TLS）。它允许变量的值在每个线程中都有一份独立的拷贝，而不是所有线程共享同一份变量。这对于在多线程环境中保持每个线程独立状态非常有用。

以下是 `thread_local` 的一些主要特点：

1. **独立副本：** 在每个线程中都有一份独立的变量副本，每个线程对该变量的修改不会影响其他线程中的副本。
2. **生命周期：** 变量的生命周期与线程的生命周期相关。当线程创建时，会分配一个新的 `thread_local` 变量副本，当线程退出时，相关的副本会被销毁。
3. **线程特定数据：** `thread_local` 可以看作是实现线程特定数据（Thread-Specific Data，TSD）的一种方式。

示例方法：

```
#include <iostream>
#include <thread>

thread_local int thread_specific_variable = 0;

void thread_function() {
    // 每个线程都有自己的 thread_specific_variable
    thread_specific_variable++;
    std::cout << "Thread ID: " << std::this_thread::get_id()
              << ", Thread Specific Variable: " << thread_specific_variable << std::endl;
}

int main() {
    std::thread t1(thread_function);
    std::thread t2(thread_function);

    t1.join();
    t2.join();

    return 0;
}
```

