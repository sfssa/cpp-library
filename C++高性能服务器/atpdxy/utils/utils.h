#pragma once
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
namespace atpdxy
{

uint32_t getFiberId();
pid_t getThreadId();

}
