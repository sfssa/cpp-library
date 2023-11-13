#include <iostream>
#include <thread>
#include "../atpdxy/log/log.h"
#include "../atpdxy/utils/utils.h"

int main(int argc,char** argv)
{
    atpdxy::Logger::ptr logger(new atpdxy::Logger);
    logger->addAppender(atpdxy::LogAppender::ptr(new atpdxy::StdoutLogAppender));
    // atpdxy::LogEvent::ptr event(new atpdxy::LogEvent(__FILE__,__LINE__,0,atpdxy::getThreadId(),2,time(0)));
    // logger->log(atpdxy::LogLevel::DEBUG,event);
    std::cout<<"hello log"<<std::endl;
    atpdxy::FileLogAppender::ptr file_appender(new atpdxy::FileLogAppender("./log.txt"));
    atpdxy::LogFormatter::ptr fmt(new atpdxy::LogFormatter("%d%T%p%T%m%m"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(atpdxy::LogLevel::ERROR);
    logger->addAppender(file_appender);
    ATPDXY_LOG_INFO(logger)<<"test macro";
    ATPDXY_LOG_ERROR(logger)<<"test macro error";
    ATPDXY_LOG_FMT_ERROR(logger,"test macro fmt error %s","aa");
    auto l=atpdxy::LoggerMgr::getInstance()->getLogger("xx");
    ATPDXY_LOG_INFO(l)<<"xxx";
    return 0;
}
