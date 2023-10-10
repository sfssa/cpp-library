#pragma once

#include <string>
#include <stdint.h>
#include <cstdio>
#include <ctime>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>

#include "config.h"
#include "thread.h"
#include "util.h"

namespace server
{

class logger;

//日志事件
class log_event
{
public:
    log_event();

    typedef std::shared_ptr<log_event> ptr;
    
    const char* get_file() const {return my_file_;}

    int32_t gel_line() const {return my_line_;}

    uint32_t get_elapse() const {return my_elapse_;}

    uint32_t get_threadid() const {return my_thread_id_;}

    uint32_t get_fiberid() const {return my_fiber_id_;}

    uint64_t get_time() const {return my_time_;}

    const std::string& get_content() const {return my_content_;}
    
private:
    const char*     my_file_         =   nullptr;   //文件名
    int32_t         my_line_         =   0;         //行号
    uint32_t        my_elapse_       =   0;         //程序开始到现在的毫秒数
    uint32_t        my_thread_id_    =   0;         //线程ID
    uint32_t        my_fiber_id_     =   0;         //协程ID
    std::string     my_content_;                    //内容
    uint64_t        my_time_;                       //时间戳
};

//日志级别
class log_level
{
public:
    enum LEVEL
    {
        UNKNOW= 0,
        DEBUG = 1,
        INFO  = 2,
        WARN  = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* to_string(log_level::LEVEL level);
};

//日志格式
class log_format
{
public:
    typedef std::shared_ptr<log_format> ptr;

    log_format(const std::string& pvattern);

    std::string format(std::shared_ptr<logger> logger,log_level::LEVEL level,log_event::ptr event);

public:
    class format_item
    {
    public:

        typedef std::shared_ptr<format_item> ptr;

        virtual ~format_item(){}

        virtual std::string format(std::ostream& os,std::shared_ptr<logger> logger,log_level::LEVEL level,log_event::ptr event)=0;
    };

    void init();
private:
    std::string my_pattern_;
    std::vector<format_item::ptr>my_items_;
};

//日志输出
class log_appender
{
public:
  
    typedef std::shared_ptr<log_appender> ptr;

    virtual ~log_appender(){}

    virtual void log(std::shared_ptr<logger> logger,log_level::LEVEL level,log_event::ptr event)=0;

    void set_format(log_format::ptr val){my_format_=val;}

    log_format::ptr get_format() const {return my_format_;}
protected:
    log_level::LEVEL my_level_;
    log_format::ptr my_format_;
};

//日志器
class logger
{
public:
    typedef std::shared_ptr<logger> ptr;

    logger(const std::string& name="root");
    
    void log(log_level::LEVEL level,log_event::ptr event);

    void debug(log_event::ptr event);

    void info(log_event::ptr event);

    void warn(log_event::ptr event);

    void error(log_event::ptr event);

    void fatal(log_event::ptr event);

    void add_appender(log_appender::ptr appender);

    void del_appender(log_appender::ptr appender);

    log_level::LEVEL get_level() const {return my_level_;}

    void set_level(log_level::LEVEL level) {my_level_=level;}

    const std::string& get_name() const {return my_name_;}
private:
    std::string my_name_;                       //日志名称
    log_level::LEVEL my_level_;                 //日志级别
    std::list<log_appender> my_appenders_;      //appender集合
};

//输出到控制台的appender
class stdout_log_appender:public log_appender
{
public:
    typedef std::shared_ptr<stdout_log_appender> ptr;
    //void log(log_level::LEVEL,log_event::ptr event) override;

};

//输出到文件的appender
class file_log_appender: public log_appender
{
public:
    typedef std::shared_ptr<stdout_log_appender> ptr;

    file_log_appender(const std::string& filename);

    //void log(log_level::LEVEL,log_event::ptr event) override;

    bool reopen();
private:
    std::string my_filename_;
    std::ofstream my_file_stream_;
    
};
}
