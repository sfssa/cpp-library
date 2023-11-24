#pragma once
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <sys/time.h>
#include <string>

namespace atpdxy
{

uint32_t getFiberId();
pid_t getThreadId();
// 获取当前调用栈信息，将栈信息以字符串的形式保存在bt参数中
// size:调用栈的大小
// skip：从skip开始将调用栈信息添加到bt中
void BackTrace(std::vector<std::string>& bt,int size,int skip=1);
std::string BacktraceToString(int size=64,int skip=2,const std::string& prefix="");

uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

}
