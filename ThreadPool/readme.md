 

# 一、为什么要有线程池？

​    早期的计算机采用的是单指令单数据（SISD）架构，即一次只执行一条指令，处理一份数据。这种串行执行的方式是计算机的基本工作原理，然而随着计算机的发展，程序所承载的作业越来越复杂，传统的串行执行极大的拉长了作业执行的时间。

​    举个例子：制作一杯咖啡

​    1、取杯子；

​    2、磨咖啡豆；

​    3、烧水；

​    4、泡咖啡；

​    5、加糖和奶；

​    在传统的串行执行计算机中，上面的每个步骤都是一条指令，依次执行，不能在前一步没有执行完的情况下开始下一步。比如，你想在磨咖啡豆的过程中烧水是不允许的，必须磨完咖啡豆后才能执行烧水的过程。不难看出这样的效率是十分低下的，因此慢慢的发展出了多处理器系统、多核处理器和并行计算，计算机能够同时执行多个任务，从而实现并行执行。通过多处理器，可以在磨咖啡的同时烧水，这样可以提高我们的效率。

# 二、进程和线程

**进程（Process）：**

1. **定义**：进程是一个独立的执行单位，每个进程都有自己的独立内存空间，包括代码、数据、堆栈等。进程之间彼此独立，不共享内存，通常通过进程间通信来交换数据。
2. **资源分配**：每个进程拥有自己的资源，如文件描述符、打开的文件、网络连接等。进程需要操作系统分配资源，并拥有独立的地址空间。
3. **开销**：创建和销毁进程的开销较大，包括分配和回收内存、初始化和销毁进程数据结构等。
4. **稳定性**：由于进程之间相互隔离，一个进程的崩溃通常不会影响其他进程。因此，进程较为稳定，但也较为消耗资源。
5. **并发性**：进程可以并发执行，多个进程可以在不同的处理器上并行运行，从而提高系统的并发性。

**线程（Thread）：**

1. **定义**：线程是进程内的执行单元，多个线程共享同一进程的资源，包括内存空间和文件描述符。线程是轻量级的执行单位。
2. **资源共享**：线程之间共享进程的资源，它们可以访问相同的内存、文件和网络连接。这使得线程之间的通信更加高效，但也需要更小的开销。
3. **开销**：创建和销毁线程的开销较小，因为它们共享进程的资源，不需要分配独立的地址空间。
4. **稳定性**：由于线程共享进程的资源，一个线程的错误可能会影响整个进程，使得进程较不稳定。但线程的错误通常不会引起整个系统的崩溃。
5. **并发性**：线程允许多个线程在同一个进程内并发执行，它们可以协同工作，共同完成任务。

**总结**：

- 进程是独立的执行单元，线程是进程内的执行单元。
- 进程之间相互隔离，线程之间共享进程的资源。
- 创建和销毁进程的开销较大，创建和销毁线程的开销较小。
- 进程通常更稳定，但消耗更多资源。线程更轻量，但不如进程稳定。
- 进程可以并发执行，多个进程可以并行运行。线程在同一进程内并发执行。

​    现在我们常用的都是线程池，应为进程的切换和创建销毁会消耗更多的性能。

# 三、C++代码实现线程池

线程池需要下面四个核心成员：

​    1、任务队列，任务队列中存放需要线程执行的任务；

​    2、互斥锁，由于任务队列中是临界资源，被多个线程访问，需要互斥锁保证安全性；

​    3、条件变量，当任务队列不为空的时候或者需要停止线程池运行时唤醒线程；

​    4、工作线程，负责不断从任务队列中取出任务并执行。

​    5、线程池是否停止工作的标志；

下面是将要使用到的头文件：

```cpp
#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
```

下面是线程池的私有数据成员：

```cpp
std::vector<std::thread> workers_;          // 工作线程
std::queue<std::function<void()>>tasks_;    // 任务队列，存放匿名函数
std::mutex queue_mutex_;                    // 任务队列的互斥锁
std::condition_variable condition_;         // 条件变量，用来唤醒工作线程
bool stop_;                                 // 线程池是否正在工作
```

## 1、构造函数

接下来说一下线程池的构造函数，构造函数接受一个整型变量，这个整型代表线程池中存放线程的数目，并在构造函数中将线程以匿名函数的方式存放到vector容器内部。线程会不断检测任务队列中是否有需要执行的任务，如果没有就一直被阻塞，直到任务队列中有数据，被条件变量唤醒，取出任务并执行。

```cpp
ThreadPool(size_t size)
        :stop_(false)
    {
        for(size_t i=0;i<size;++i)
        {
            workers_.emplace_back([this]{
                for(;;)    // 效果等于while(true)循环
                {
                    std::function<void()> task;

                    {    // 这里的大括号是为了unique_lock出了大括号作用域后自动解锁
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock,[this]{
                            return (this->stop_)||!this->tasks_.empty();
                        });
                        if(this->stop_&&this->tasks_.empty())
                            return;
                        task=std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
```

> std::function<void()> task;

这句代码声明了一个函数指针： 指向一个不接受任何参数、返回值为void型的函数，task是函数指针的名称，将可调用对象分配给task，然后通过task()来执行函数。

> this->condition_.wait(lock,[this]{
>      return (this->stop_)||!this->tasks_.empty();
>  });

这段代码只有当括号内的代码结果为真时才会继续执行后面的代码，否则会一直阻塞在这个函数中。将上面的代码翻译过来：只有当线程池已经停止运行或者任务队列不为空时才会执行后面的代码，否则就阻塞在这里等待任务队列中被添加新的任务后被条件变量唤醒。 

> if(this->stop_&&this->tasks_.empty())
>      return; 

这里如果线程池已经停止运行了并且任务队列不为空的话就直接返回，停止线程的运行，加入任务队列不为空的条件限制是为了执行完任务队列中的所有待执行任务，避免有任务未执行而线程池中的线程直接停止运行。 

否则就将任务队列队首的任务通过move移动函数将任务赋值给task函数指针。移动操作是一种高效的资源转移操作，它将对象的资源（如内存、文件句柄等）从一个对象转移到另一个对象，而不进行复制。然后将这个任务从任务队列中移除。之后通过task()的形式执行函数，这就是每个线程的任务。

**总的来说：每个线程的任务就是不断监听任务队列，任务队列不为空就尝试获得锁，并取出任务执行。**

## 2、向任务队列中添加任务

```cpp
template <class F,class... Args>
auto enqueue(F&& f,Args&&... args)
    ->std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type=typename std::result_of<F(Args...)>::type;

    auto task=std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );

    std::future<return_type> res=task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool.");

        tasks_.emplace([task](){
            (*task)();
        });
    }
    condition_.notify_one();
    return res;
}
```

这个函数接受一个可调用的函数F和一系列参数，并返回一个std::future对象，用于异步获取可调用函数的执行结果或状态。

> auto enqueue(F&& f,Args&&... args)
>    ->std::future<typename std::result_of<F(Args...)>::type>

这里的auto是自动推导，常用方法如下面这样：

> auto& ret:m_vector; 

其中，m_vector是vector容器，如果存放的类型是int，那么经过auto推导会得到int& ret:m_vector; 

然而auto无法推到返回值，因此有了后面的代码:

> ->std::future<typename std::result_of<F(Args...)>::type>

这里给出了函数的返回值类型，result_of函数是C++11引入的类型萃取工具，用于推断一个可调用对象的返回类型，使得编译器能够在不需要显示指定返回类型的情况下确定可调用对象的返回类型，->用于指定函数的返回类型，通过上述操作，auto就可以推导出函数的返回值类型了。

> using return_type=typename std::result_of<F(Args...)>::type;

这句代码使用using来声明了返回值类型，用来定义返回值类型。

> auto task=std::make_shared<std::packaged_task<return_type()>>(
>      std::bind(std::forward<F>(f),std::forward<Args>(args)...)
>  );

 这里声明了一个类型为task的智能指针，拥有一个std::packaged_task对象，make_shared用来创建一个智能指针，确保对象不再被需要的时候被销毁。通过packaged_task对象封装可调用对象，然后用bind函数将可调用对象和和可变参数绑定，并通过forward完美转发保证值的类别不变。到这里，task指针已经拥有了任务的所有资源。

> [task](){
>
> ​	(*task)();
> }

这里代码是将任务封装成std::packaged_task可调用对象（匿名函数）并添加到线程池的任务队列中，以便稍后被工作线程异步执行。最后通过条件变量唤醒一个线程来执行这个任务。

## 3、析构函数

```cpp
~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_=true;
    }
    condition_.notify_all();
    for(std::thread& worker: workers_)
    {
        if(worker.joinable())
            worker.join();
    }
}
```

拥有锁后将线程池停止运行的标志设置为真，函数体的大括号也是为了在出了作用域后释放锁。之后唤醒所有的线程。然后检查线程是否满足joinable的条件，满足就调用join函数，使得主函数等待线程执行完毕。这是为了将任务队列中的所有任务全部执行完毕。避免漏掉任务队列中的任务。

#  四、小建议

最后一点小建议：我并不知道用visual studio或者vs code多线程的调试方法，但是尝试过gdb调试，大伙可以写一个测试的demo来测试这个线程池。在gdb中有多线程的调试方法：

> info threads

上面的指令可以查看当前程序所有的线程和线程id，然后通过下面的命令就可以跳转到具体的某个线程执行到了哪个代码，非常有用。

> thread id (这里的id是线程的id，使用info threads会给出)

# 五、源码 

```cpp
// ThreadPool.h

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool
{
private:
    std::vector<std::thread> workers_;          // 工作线程
    std::queue<std::function<void()>>tasks_;    // 任务队列，存放匿名函数
    std::mutex queue_mutex_;                    // 任务队列的互斥锁
    std::condition_variable condition_;         // 条件变量，用来唤醒工作线程
    bool stop_;                                 // 线程池是否正在工作
public:
    ThreadPool(size_t size)
        :stop_(false)
    {
        for(size_t i=0;i<size;++i)
        {
            workers_.emplace_back([this]{
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock,[this]{
                            return (this->stop_)||!this->tasks_.empty();
                        });
                        if(this->stop_&&this->tasks_.empty())
                            return;
                        task=std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template <class F,class... Args>
    auto enqueue(F&& f,Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type=typename std::result_of<F(Args...)>::type;

        auto task=std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );

        std::future<return_type> res=task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if(stop_)
                throw std::runtime_error("enqueue on stopped ThreadPool.");

            tasks_.emplace([task](){
                (*task)();
            });
        }
        condition_.notify_one();
        return res;
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_=true;
        }

        condition_.notify_all();
        for(std::thread& worker: workers_)
        {
            if(worker.joinable())
                worker.join();
        }
    }
};
```

```cpp
// TestDemo.cpp

#include <iostream>
#include <chrono>
#include "ThreadPool.h"

int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([i] {
            try {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i * i;
            } catch (const std::exception& e) {
                std::cerr << "Exception in task: " << e.what() << std::endl;
                return -1;  // or handle the exception in an appropriate way
            }
        }));
    }

    for (auto&& result : results) {
        int value = result.get();
        std::cout << "Result: " << value << std::endl;
    }

    return 0;
}
```