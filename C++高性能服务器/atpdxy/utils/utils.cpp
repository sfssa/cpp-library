#include "utils.h"

namespace atpdxy
{
pid_t getThreadId()
{
    return syscall(SYS_gettid);
}

uint32_t getFiberId()
{
    return 0;
}

}
