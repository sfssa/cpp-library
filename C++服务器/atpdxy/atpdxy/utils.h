#pragma once

#include <cinttypes>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "noncopyable.h"
#include "singleton.h"

namespace server
{

//获取线程的唯一ID
long get_threadid();

//获取协程的唯一ID
uint64_t get_fiberid();

//以vector<string&>的形式获得调用栈信息，可以指定获取的最大层数和省略最近的调用栈次数
//在调试和追踪时非常有用，用来获取函数的调用路径
void backtrace(std::vector<std::string>& out,int size=200,int skip=1);

//获取调用栈的信息并以string的形式返回，可以指定获取的最大层数和省略最近的调用栈层数
std::string backtrace_tostring(int size=200,int skip=2);

//获得当前时间的毫秒表示
uint64_t get_currentms();

//获得当前时间的微秒表示
uint64_t get_currentus();

}
