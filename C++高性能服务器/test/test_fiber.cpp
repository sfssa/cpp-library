#include "../atpdxy/atpdxy.h"

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
