#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <stdio.h>
#include <stdarg.h>
#include <map>
// #include "../utils/utils.h"
// #include "../design/singleton.h"
// #include "../thread/thread.h"

// 使用流式方式将日志级别为level的日志写入到logger
#define ATPDXY_LOG_LEVEL(logger,level) \
    if(logger->getLevel()<=level) \
        atpdxy::LogEventWrap(atpdxy::LogEvent::ptr(new atpdxy::LogEvent(logger,level, \
            __FILE__,__LINE__,0,atpdxy::getThreadId(),atpdxy::getFiberId(),time(0), \
            atpdxy::Thread::getName()))).getSS()

// 使用流方式将日志级别debug的日志写入logger
#define ATPDXY_LOG_DEBUG(logger) ATPDXY_LOG_LEVEL(logger,atpdxy::LogLevel::DEBUG)

// 使用流方式将日志级别info的日志写入logger
#define ATPDXY_LOG_INFO(logger) ATPDXY_LOG_LEVEL(logger,atpdxy::LogLevel::INFO)

// 使用流方式将日志级别warn的日志写入logger
#define ATPDXY_LOG_WARN(logger) ATPDXY_LOG_LEVEL(logger,atpdxy::LogLevel::WARN)

// 使用流方式将日志级别error的日志写入logger
#define ATPDXY_LOG_ERROR(logger) ATPDXY_LOG_LEVEL(logger,atpdxy::LogLevel::ERROR)

// 使用流方式将日志级别fatal的日志写入logger
#define ATPDXY_LOG_FATAL(logger) ATPDXY_LOG_LEVEL(logger,atpdxy::LogLevel::FATAL)

// 使用格式化方式将日志级别为level日志写入logger
#define ATPDXY_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel()<=level) \
        atpdxy::LogEventWrap(atpdxy::LogEvent::ptr(new atpdxy::LogEvent(logger,level, \
            __FILE__,__LINE__,0,sylar::getThreadId(),atpdxy::getFiberId(),time(0), \
            atpdxy::Thread::getName()))).getEvent()->format(fmt,__VA_ARGS__)

// 使用格式化方式将日志级别为debug的日志写入logger
#define ATPDXY_LOG_FMT_DEBUG(logger,fmt,...) ATPDXY_LOG_FMT_LEVEL(logger,atpdxy::LogLevel::DEBUG,fmt,__VA_ARGS__)

// 使用格式化方式将日志级别为info的日志写入logger
#define ATPDXY_LOG_FMT_INFO(logger,fmt,...) ATPDXY_LOG_FMT_LEVEL(logger,atpdxy::LogLevel::INFO,fmt,__VA_ARGS__)

// 使用格式化方式将日志级别为warn的日志写入logger
#define ATPDXY_LOG_FMT_WARN(logger,fmt,...) ATPDXY_LOG_FMT_LEVEL(logger,atpdxy::LogLevel::WARN,fmt,__VA_ARGS__)

// 使用格式化方式将日志级别为error的日志写入logger
#define ATPDXY_LOG_FMT_ERROR(logger,fmt,...) ATPDXY_LOG_FMT_LEVEL(logger,atpdxy::LogLevel::ERROR,fmt,__VA_ARGS__)

// 使用格式化方式将日志级别为fatal的日志写入logger
#define ATPDXY_LOG_FMT_FATAL(logger,fmt,...) ATPDXY_LOG_FMT_LEVEL(logger,atpdxy::LogLevel::FATAL,fmt,__VA_ARGS__)

// 获取主日志器
#define ATPDXY_LOG_ROOT() atpdxy::LoggerMgr::getInstance().getRoot()

// 获取名为name的日志器
#define ATPDXY_LOG_NAME(name) atpdxy::LoggerMgr::getInstance().getLogger(name
namespace atpdxy
{

class Logger;
class LoggeManager;

// 日志级别
class LogLevel
{
public:
    enum Level{
        UNKNOW=0,   // 未知级别
        DEBUG=1,    // DEBUG级别
        INFO=2,     // INFO级别
        WARN=3,     // WARN级别
        ERROR=4,    // ERROR级别
        FATAL=5,    // FATAL级别
    };

    // 将日志级别转换成文本输出
    static const char* toString(LogLevel::Level level);

    // 将文本转换成日志级别
    static LogLevel::Level levelFromString(const std::string& str);
};

// 日志事件
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,
        const char* file,int32_t line,uint32_t elapse,uint32_t thread_id,
        uint32_t fiber_id,uint64_t time,const std::string& thread_name);

    LogEvent(const char* file,int32_t line,uint32_t elapse,uint32_t thread_id
        ,uint32_t fiber_id,uint64_t time);
    // 返回文件名
    const char* getFile() const 
    {
        return m_file_;
    }

    // 返回行号
    int32_t getLine() const
    {
        return m_line_;
    }

    // 返回耗时
    uint32_t getElapse() const
    {
        return m_elapse_;
    }

    // 返回线程ID
    uint32_t getThreadId() const
    {
        return m_thread_id_;
    }

    // 返回协程ID
    uint32_t getFiberId() const
    {
        return m_fiber_id_;
    }

    // 返回时间戳
    uint64_t getTime() const
    {
        return m_time_;
    }

    // 返回线程名称
    const std::string& getThreadName() const
    {
        return m_thread_name_;
    }

    // 返回日志内容
    std::string getContent() const
    {
        return m_ss_.str();
    }

    // 返回日志器
    std::shared_ptr<Logger> getLogger() const
    {
        return m_logger_;
    }

    // 返回日志级别
    LogLevel::Level getLevel() const
    {
        return m_level_;
    }

    // 返回日志内容字符串流
    std::stringstream& getSS() 
    {
        return m_ss_;
    }

    // 格式化写入日志内容-接受可变参数
    void format(const char* fmt,...);

    // 格式化写入日志内容-将可变参数传递给其他函数
    void format(const char* fmt,va_list al);

private:
    const char* m_file_=nullptr;        // 文件名
    int32_t m_line_=0;                  // 行号
    uint32_t m_elapse_=0;               // 程序启动到现在的毫秒数
    uint32_t m_thread_id_=0;            // 线程ID
    uint32_t m_fiber_id_=0;             // 协程ID
    uint64_t m_time_=0;                 // 时间戳
    std::string m_thread_name_;         // 线程名称
    std::stringstream m_ss_;            // 日志内容流
    std::shared_ptr<Logger> m_logger_;  // 日志器
    LogLevel::Level m_level_;           // 日志等级
};

class LogEventWrap
{
public:
    // 接收一个日志事件的构造函数
    LogEventWrap(LogEvent::ptr e);

    ~LogEventWrap();

    // 获取日志事件的智能指针
    LogEvent::ptr getEvent() const
    {
        return m_event_;
    }

    // 获取日志内容流
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event_;
};

// 日志格式器
class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /*    
    %m 消息
    %p 日志级别
    %r 累计毫秒数
    %c 日志名称
    %t 线程id
    %n 换行
    %d 时间
    %f 文件名
    %l 行号
    %T 制表符
    %F 协程id
    %N 线程名称 

    默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    */
    // 接受一个输出的格式模板
    LogFormatter(const std::string& pattern);

    // 日志器-日志级别-日志事件
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);

    // 输出流-日志器-日志级别-日志事件
    std::ostream& format(std::ostream& ofs,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);

    // 初始化解析日志模板
    void init();

    // 是否出现错误
    bool isError() const 
    {
        return m_error_;
    }

    // 返回日志模板
    const std::string getPattern() const
    {
        return m_pattern_;
    }
public:
    class FormatItem
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem(){}

        virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)=0;
    };
private:
    std::string m_pattern_;                 // 日志模板
    std::vector<FormatItem::ptr> m_items_;  // 日志格式解析后的格式
    bool m_error_=false;                    // 是否出现了错误
};

// 日志输出目标
class LogAppender
{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    // typedef Spinlock MutexType;

    virtual ~LogAppender(){}

    // 日志器-日志级别-日志事件
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)=0;

    // 将日志输出目标的配置转换成YAML String
    virtual std::string toYamlString()=0;

    // 更改日志格式器
    void setFormatter(LogFormatter::ptr val);

    // 获取日志格式器
    LogFormatter::ptr getFormatter();

    // 获取日志级别
    LogLevel::Level getLevel() const
    {
        return m_level_;
    }

    // 设置日志级别
    void setLevel(LogLevel::Level val)
    {
        m_level_=val;
    }
protected:
    LogLevel::Level m_level_=LogLevel::DEBUG;    // 日志级别
    bool m_hasFormatter_=false;                  // 是否有自己的日志格式器
    // MutexType m_mutex_;                          // 互斥锁
    LogFormatter::ptr m_formatter_;              // 日志格式器
};

// 日志器
class Logger:public std::enable_shared_from_this<Logger>
{
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    // typedef Spinlock MutexType;

    // 接收一个字符串作为日志器名称
    Logger(const std::string& name="root");

    // 写日志，日志级别-日志事件
    void log(LogLevel::Level level,LogEvent::ptr event);

    // 写debug级别的日志
    void debug(LogEvent::ptr event);

    // 写info级别的日志
    void info(LogEvent::ptr event);

    // 写warn级别的日志
    void warn(LogEvent::ptr event);

    // 写error级别的日志
    void error(LogEvent::ptr event);

    // 写fatal级别的日志
    void fatal(LogEvent::ptr event);

    // 添加日志目标
    void addAppender(LogAppender::ptr appender);

    // 删除日志目标
    void delAppender(LogAppender::ptr appender);

    // 清空日志目标
    void clearAppenders();

    // 返回日志级别
    LogLevel::Level getLevel() const
    {
        return m_level_;
    }

    // 设置日志级别
    void setLevel(LogLevel::Level level)
    {
        m_level_=level;
    }

    // 返回日志名称
    const std::string& getName() const
    {
        return m_name_;
    }

    // 设置日志格式器
    void setFormatter(LogFormatter::ptr val);

    // 设置日志格式模板
    void setFormatter(const std::string& val);

    // 获取日志格式器
    LogFormatter::ptr getFormatter();

    // 将日志器的配置转换成YAML String类型
    std::string toYamlString();
private:
    std::string m_name_;                        // 日志器名称
    LogLevel::Level m_level_;                   // 日志级别
    // MutexType m_mutex_;                         // 互斥锁
    std::list<LogAppender::ptr> m_appenders_;   // 日志目标集合
    LogFormatter::ptr m_formatter_;             // 日志格式器
    Logger::ptr m_root_;                        // 主日志器
};

// 输出到控制台的Appender
class StdoutLogAppender:public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override;
    std::string toYamlString() override;
};

// 输出到文件的Appender
class FileLogAppender:public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override;
    std::string toYamlString() override;

    // 重新打开日志文件
    bool reopen();
private:
    std::string m_filename_;        // 文件名
    std::ofstream m_filestream_;    // 文件流
    uint64_t m_last_time_=0;        // 最后一次打开文件
};

class LoggerManager
{
public:
    // typedef Spinlock MutexType;
    LoggerManager();

    Logger::ptr getLogger(const std::string& name);

    void init() {}

    // 返回主日志器
    Logger::ptr getRoot() const
    {
        return m_root_;
    }

    // 将日志器配置转换成YAML String
    std::string toYamlString();
private:
    // MutexType m_mutex_;                             // 互斥锁
    std::map<std::string,Logger::ptr> m_loggers_;   // 日志器容器
    Logger::ptr m_root_;                            // 主日志器
};

// typedef atpdxy::Singleton<LoggerManager> LoggerMgr;
}

