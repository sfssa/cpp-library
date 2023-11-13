#include <iostream>
#include "../atpdxy/log/log.h"

int main(int argc,char** argv)
{
    atpdxy::Logger::ptr logger(new atpdxy::Logger);
    logger->addAppender(atpdxy::LogAppender::ptr(new atpdxy::StdoutLogAppender));
    atpdxy::LogEvent::ptr event(new atpdxy::LogEvent(__FILE__,__LINE__,0,1,2,time(0)));
    logger->log(atpdxy::LogLevel::DEBUG,event);
    std::cout<<"hello log"<<std::endl;
    return 0;
}
