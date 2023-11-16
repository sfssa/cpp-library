#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>
#include "../config/config.h"
// #include "util.h"
// #include "macro.h"
// #include "env.h"
#include "log.h"

namespace atpdxy
{

// Level
const char* LogLevel::toString(LogLevel::Level level)
{
    switch(level)
    {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::levelFromString(const std::string& str)
{
#define XX(level, v) \
    if(str == #v) { \
        return LogLevel::level; \
    }
    XX(DEBUG,debug);
    XX(INFO,info);
    XX(WARN,warn);
    XX(ERROR,error);
    XX(FATAL,fatal);

    XX(DEBUG,DEBUG);
    XX(INFO,INFO);
    XX(WARN,WARN);
    XX(ERROR,ERROR);
    XX(FATAL,FATAL);
    return LogLevel::UNKNOW;
#undef XX
}

// LogEventWrap
LogEventWrap::LogEventWrap(LogEvent::ptr e)
    :m_event_(e)
{

}

LogEventWrap::~LogEventWrap()
{
    m_event_->getLogger()->log(m_event_->getLevel(),m_event_);
}

std::stringstream& LogEventWrap::getSS()
{
    return m_event_->getSS();
}

// LogEvent
void LogEvent::format(const char* fmt,...)
{
    va_list al;
    va_start(al,fmt);
    format(fmt,al);
    va_end(al);
}

void LogEvent::format(const char* fmt,va_list al)
{
    char* buf=nullptr;
    int len=vasprintf(&buf,fmt,al);
    if(len!=-1)
    {
        m_ss_<<std::string(buf,len);
        free(buf);
    }
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level
    ,const char* file,int32_t line,uint32_t elapse,uint32_t thread_id
    ,uint32_t fiber_id,uint64_t time,const std::string& thread_name)
    :m_file_(file),m_line_(line),m_elapse_(elapse),m_thread_id_(thread_id),
    m_fiber_id_(fiber_id),m_time_(time),m_thread_name_(thread_name),
    m_logger_(logger),m_level_(level)
{

}

LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t thread_id
        ,uint32_t fiber_id,uint64_t time)
    :m_file_(file),m_line_(line),m_elapse_(elapse),m_thread_id_(thread_id),m_fiber_id_(fiber_id),
    m_time_(time),m_logger_(logger),m_level_(level)
{

}

// LogAppender
void LogAppender::setFormatter(LogFormatter::ptr val)
{
    // MutexType::Lock lock(m_mutex_);
    m_formatter_=val;
    if(m_formatter_)
        m_hasFormatter_=true;
    else
        m_hasFormatter_=false;
}

// LogFormatter
LogFormatter::ptr LogAppender::getFormatter()
{
    // MutexType::Lock lock(m_mutex_);
    return m_formatter_;
}

class MessageFormatItem:public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getContent();
    }
};

class LevelFormatItem:public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<LogLevel::toString(level);
    }
};

class ElapseFormatItem:public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getElapse();
    }
};

class NameFormatItem:public LogFormatter::FormatItem
{
public:
    NameFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getLogger()->getName();
        // os<<logger->getName();
    }
};

class ThreadIdFormatItem:public LogFormatter::FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getThreadId();
    }
};

class FiberIdFormatItem:public LogFormatter::FormatItem
{
public:
    FiberIdFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getFiberId();
    }
};

class ThreadNameFormatItem:public LogFormatter::FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getThreadName();
    }
};

class DateTimeFormatItem:public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const std::string& format="%Y-%m-%d %H:%M:%S")
        :m_format_(format)
    {
        if(m_format_.empty())
            m_format_="%Y-%m-%d %H:%M:%S";
    }

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time=event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf,sizeof(buf),m_format_.c_str(),&tm);
        os<<buf;
    }
private:
    std::string m_format_;
};

class FileNameFormatItem:public LogFormatter::FormatItem
{
public:
    FileNameFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getFile();
    }
};

class LineFormatItem:public LogFormatter::FormatItem
{
public:
    LineFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<event->getLine();
    }
};

class NewLineFormatItem:public LogFormatter::FormatItem
{
public:
    NewLineFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<std::endl;
    }
};

class StringFormatItem:public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string& str)
        :m_string_(str)
    {

    }

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<m_string_;
    }
private:
    std::string m_string_;
};

class TabFormatItem:public LogFormatter::FormatItem
{
public:
    TabFormatItem(const std::string& str=""){}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os<<"\t";
    }
// private:
//     std::string m_string_;
};

std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto& i : m_items_)
        i->format(ss,logger,level,event);
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    for(auto& i : m_items_)
        i->format(ofs,logger,level,event);
    return ofs;
}

// %xxx %xxx{xxx} %%
void LogFormatter::init()
{
    // 格式项-格式项内容-是否有格式项内容
    std::vector<std::tuple<std::string,std::string,int>>vec;
    std::string nstr;
    for(size_t i=0;i<m_pattern_.size();++i)
    {
        // 正常的格式项，加入到nstr中
        if(m_pattern_[i]!='%')
        {
            nstr.append(1,m_pattern_[i]);
            continue;
        }

        // %%则将%加入到nstr
        if((i+1)<m_pattern_.size())
        {
            if(m_pattern_[i+1]=='%')
            {
                nstr.append(1,'%');
                continue;
            }
        }

        size_t n=i+1;         // 下一个要解析的位置
        int fmt_status=0;     // 解析格式项还是格式项内容
        size_t fmt_begin=0;   // 格式项内容起始位置
        std::string str;      // 普通字符串部分
        std::string fmt;      // 存储格式项的格式信息内容

        while(n<m_pattern_.size())
        {
            if(!fmt_status && (!isalpha(m_pattern_[n]) && m_pattern_[n]!='{' && m_pattern_[n]!='}'))
            {
                str=m_pattern_.substr(i+1,n-i-1);
                break;
            }

            if(fmt_status==0)
            {
                if(m_pattern_[n]=='{')
                {
                    str=m_pattern_.substr(i+1,n-i-1);
                    fmt_status=1;
                    fmt_begin=n;
                    ++n;
                    continue;
                }
            }
            else if(fmt_status==1)
            {
                if(m_pattern_[n]=='}')
                {
                    fmt=m_pattern_.substr(fmt_begin+1,n-fmt_begin-1);
                    fmt_status=0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n==m_pattern_.size())
            {
                if(str.empty())
                    str=m_pattern_.substr(i+1);
            }
        }

        if(fmt_status==0)
        {
            if(!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr,std::string(),0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n-1;
        }
        else if(fmt_status==1)
        {
            std::cout<<"pattern parse error: "<<m_pattern_<<" - "<<m_pattern_.substr(i)<<std::endl;
            m_error_=true;
            vec.push_back(std::make_tuple("<<pattern_error",fmt,0));
        }
    }
    if(!nstr.empty())
        vec.push_back(std::make_tuple(nstr,"",0));

    static std::map<std::string,std::function<FormatItem::ptr(const std::string& str)>>s_format_items=
    {
#define XX(str,C) \
    {#str,[](const std::string& fmt){return FormatItem::ptr (new C(fmt));}}
    
    XX(m,MessageFormatItem),    // 消息
    XX(p,LevelFormatItem),      // 日志级别
    XX(r,ElapseFormatItem),     // 累计毫秒数
    XX(c,NameFormatItem),       // 日志名称
    XX(t,ThreadIdFormatItem),   // 线程ID
    XX(n,NewLineFormatItem),    // 换行
    XX(d,DateTimeFormatItem),   // 时间
    XX(f,FileNameFormatItem),   // 文件名
    XX(l,LineFormatItem),       // 行号
    XX(T,TabFormatItem),        // Tab
    XX(F,FiberIdFormatItem),    // 协程ID
    XX(N,ThreadNameFormatItem), // 线程名称
#undef XX
    };

    // 将vector内解析的数据和map中进行匹配，可以成功匹配就插入到m_items_，否则就
    // 创建错误的信息并插入到m_items_
    for(auto& i : vec)
    {
        if(std::get<2>(i)==0)
            m_items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        else
        {
            auto it=s_format_items.find(std::get<0>(i));
            if(it==s_format_items.end())
            {
                m_items_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %"+std::get<0>(i)+">>")));
                m_error_=true;
            }
            else 
                m_items_.push_back(it->second(std::get<1>(i)));
        }
    }
}

// LoggerManager
LoggerManager::LoggerManager()
{
    // 默认日志器名字为root
    m_root_.reset(new Logger);
    m_root_->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers_[m_root_->m_name_]=m_root_;
    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    // 有返回日志器的智能指针
    // MutexType::Lock lock(m_mutex_);
    auto it=m_loggers_.find(name);
    if(it!=m_loggers_.end())
        return it->second;

    // 没有就新建一个名字为name的日志器，设置主日志器并返回
    Logger::ptr logger(new Logger(name));
    logger->m_root_=m_root_;
    m_loggers_[name]=logger;
    return logger;
}

// Logger
Logger::Logger(const std::string& name)
    :m_name_(name),m_level_(LogLevel::DEBUG)
{
    // 没有任何配置也会输出到控制台
    m_formatter_.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    // MutexType::Lock lock(m_mutex_);
    m_formatter_=val;

    for(auto& i : m_appenders_)
    {
        // MutexType::Lock lock(i->m_mutex_);
        if(!i->m_hasFormatter_)
            i->m_formatter_=m_formatter_;
    }
}

void Logger::setFormatter(const std::string& val)
{
    std::cout<<"---"<<val<<std::endl;
    atpdxy::LogFormatter::ptr new_val(new atpdxy::LogFormatter(val));
    if(new_val->isError())
    {
        std::cout<<"Logger setFormatter name="<<m_name_
            <<" value="<<val<<" invalid formatter"
            <<std::endl;
        return;
    }
    // m_formatter_=new_val;
    setFormatter(new_val);
}

std::string Logger::toYamlString()
{
    // MutexType::Lock lock(m_mutex_);
    YAML::Node node;
    node["name"]=m_name_;
    if(m_level_!=LogLevel::UNKNOW)
        node["level"]=LogLevel::toString(m_level_);
    if(m_formatter_)
        node["formatter"]=m_formatter_->getPattern();

    for(auto& i:m_appenders_)
        node["appenders"].push_back(YAML::Load(i->toYamlString()));

    std::stringstream ss;
    ss<<node;
    return ss.str();
    return nullptr;
}

LogFormatter::ptr Logger::getFormatter()
{
    // MutexType::Lock lock(m_mutex_);
    return m_formatter_;
}

void Logger::addAppender(LogAppender::ptr appender)
{
    // MutexType::Lock lock(m_mutex_);
    if(!appender->getFormatter())
    {
        // MutexType::Lock lock(appender->m_mutex_);
        appender->m_formatter_=m_formatter_;
    }
    m_appenders_.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    // MutexType::Lock lock(m_mutex_);
    for(auto it=m_appenders_.begin();it!=m_appenders_.end();++it)
    {
        if(*it==appender)
        {
            m_appenders_.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    // MutexType::Lock lock(m_mutex_);
    m_appenders_.clear();
}

void Logger::log(LogLevel::Level level,LogEvent::ptr event)
{
    if(level>=m_level_)
    {
        auto self=shared_from_this();
        // MutexType::Lock lock(m_mutex_);
        if(!m_appenders_.empty())
        {
            for(auto& i: m_appenders_)
                i->log(self,level,event);
        }
        else if(m_root_)
            m_root_->log(level,event);
    }
}

void Logger::debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG,event);
}

void Logger::info(LogEvent::ptr event)
{
    log(LogLevel::INFO,event);
}

void Logger::warn(LogEvent::ptr event)
{
    log(LogLevel::WARN,event);
}

void Logger::error(LogEvent::ptr event)
{
    log(LogLevel::ERROR,event);
}

void Logger::fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL,event);
}

// FileLogAppender
FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename_(filename)
{
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    if(level>=m_level_)
    {
        uint64_t now=event->getTime();
        if(now>=(m_last_time_+3))
        {
            reopen();
            m_last_time_=now;
        }
        // MutexType::Lock lock(m_mutex_);
        if(!m_formatter_->format(m_filestream_,logger,level,event))
            std::cout<<"error"<<std::endl;
    }
}

std::string FileLogAppender::toYamlString()
{
    // MutexType::Lock lock(m_mutex_);
    YAML::Node node;
    node["type"]="FileLogAppender";
    node["file"]=m_filename_;
    if(m_level_!=LogLevel::UNKNOW)
        node["level"]=LogLevel::toString(m_level_);
    if(m_hasFormatter_ && m_formatter_)
        node["formatter"]=m_formatter_->getPattern();
    std::stringstream ss;
    ss<<node;
    return ss.str();
    return nullptr;
}

bool FileLogAppender::reopen()
{
    // MutexType::Lock lock(m_mutex_);
    if(m_filestream_)
        m_filestream_.close();
    m_filestream_.open(m_filename_, std::ios::app);
    if(!m_filestream_)
        return false;
    return true;
    // return FSUtil::OpenForWrite(m_filestream_,m_filename_,std::ios::app);
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    if(level>=m_level_)
    {
        // MutexType::Lock lock(m_mutex_);
        m_formatter_->format(std::cout,logger,level,event);
    }
}

std::string StdoutLogAppender::toYamlString()
{
    // MutexType::Lock lock(m_mutex_);
    YAML::Node node;
    node["type"]="StdoutLogAppender";
    if(m_level_!=LogLevel::UNKNOW)
        node["level"]=LogLevel::toString(m_level_);
    if(m_hasFormatter_ && m_formatter_)
        node["formatter"]=m_formatter_->getPattern();
    std::stringstream ss;
    ss<<node;
    return ss.str();
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern_(pattern)
{
    init();
}

// 定义日志附加器的配置参数
// type:指定附加器的类型；level:日志记录级别；formatter:日志格式字符串;file:日志文件路径
struct LogAppenderDefine
{
    int type=0; // 1-file,2-stdout
    LogLevel::Level level=LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    // 比较两个LogAppenderDefine配置是否相等
    bool operator==(const LogAppenderDefine& other) const
    {
        return type==other.type 
            && level==other.level
            && formatter==other.formatter
            && file==other.file;
    }  
};

// 定义日志器的配置参数
// name:日志器名称;level:日志级别;formatter:日志格式字符串;appenders:日志附加器的配置信息
struct LogDefine
{
    std::string name;
    LogLevel::Level level=LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    // 比较两个LogDefine对象是否相等
    bool operator==(const LogDefine& other) const
    {
        return name==other.name
            && level==other.level
            && formatter==other.formatter
            && appenders==other.appenders;
    }

    // 比较两个LogDefine对象的名称字段
    bool operator<(const LogDefine& other) const
    {
        return name<other.name;
    }

    // 检查日志定义是否有效
    bool isValid() const
    {
        return !name.empty();
    }
};



// 特化的LexicalCast模板类，将字符串转换成日志输出器定义对象。是一个重要的配置转换器，用于从配置文件或
// 其他配置中读取字符串并将其转换成具体的数据结构
template <>
class LexicalCast<std::string,LogDefine>
{
public:
    LogDefine operator()(const std::string& v)
    {
        YAML::Node n=YAML::Load(v);
        LogDefine ld;
        if(!n["name"].IsDefined())
        {
            std::cout<<"log config error: name is null, "<<n<<std::endl;
            throw std::logic_error("log config name is null");
        }
        ld.name=n["name"].as<std::string>();
        ld.level=LogLevel::levelFromString(n["level"].IsDefined()?n["level"].as<std::string>():"");
        if(n["formatter"].IsDefined())
            ld.formatter=n["formatter"].as<std::string>();

        if(n["appenders"].IsDefined())
        {
            for(size_t x=0;x<n["appenders"].size();++x)
            {
                auto a=n["appenders"][x];
                if(!a["type"].IsDefined())
                {
                    std::cout<<"log config error: appender type is null, "<<a<<std::endl;
                    continue;
                }

                std::string type=a["type"].as<std::string>();
                LogAppenderDefine lad;
                if(type=="FileLogAppender")
                {
                    lad.type=1;
                    if(!a["file"].IsDefined())
                    {
                        std::cout<<"log config erro: fileappender file is nl, "<<a<<std::endl;
                        continue;
                    }
                    lad.file=a["file"].as<std::string>();
                    if(a["formatter"].IsDefined())
                    {
                        lad.formatter=a["formatter"].as<std::string>();
                    }
                }
                else if(type=="StdoutLogAppender")
                {
                    lad.type=2;
                    if(a["formatter"].IsDefined())
                        lad.formatter=a["formatter"].as<std::string>();
                }
                else
                {
                    std::cout<<"log config error: appender type is invalid, "<<a<<std::endl;
                    continue;
                }
                ld.appenders.push_back(lad);
            }   
        }
        return ld;
    }
};

// 实现LogDefine类型到string类型的转换，将LogDefine对象序列化成字符串的操作；
// 将LogDefine转换为YAML格式的字符串，将日志配置信息保存到文件或其他介质
template<>
class LexicalCast<LogDefine,std::string>
{
public:
    std::string operator()(const LogDefine& i)
    {
        YAML::Node n;
        n["name"]=i.name;
        if(i.level!=LogLevel::UNKNOW)
            n["level"]=LogLevel::toString(i.level);

        if(!i.formatter.empty())
            n["formatter"]=i.formatter;

        for(auto& a:i.appenders)
        {
            YAML::Node na;
            if(a.type==1)
            {
                na["type"]="FileLogAppender";
                na["file"]=a.file;
            }
            else if(a.type==2)
                na["type"]="StdoutLogAppender";
            if(a.level!=LogLevel::UNKNOW)
                na["level"]=LogLevel::toString(a.level);
            if(!a.formatter.empty())
                na["formatter"]=a.formatter;

            n["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss<<n;
        return ss.str();
    }
};

// 创建一个配置项g_log_defines，从配置系统查找名为logs的配置项，找到就赋值给g_log_defines；
// 否则使用空的set<LogDefine>作为默认值
atpdxy::ConfigVar<std::set<LogDefine>>::ptr g_log_defines=
    atpdxy::Config::lookUp("logs",std::set<LogDefine>(),"logs cofig");

// 在程序启动时执行，注册一个监听器，监听名为logs的配置项的变化
// 当logs配置发生变化时，lambda会执行
// lambda根据新旧配置项的差异，更新日志系统的配置，包括设置级别、格式器、添加/删除日志输出目标
struct LogIniter
{
    LogIniter()
    {
        // 更新配置
        g_log_defines->addListener([](const std::set<LogDefine>& old_value,
                    const std::set<LogDefine>& new_value){
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"on_logger_conf_changed";
            for(auto& i : new_value)
            {
                auto it=old_value.find(i);
                atpdxy::Logger::ptr logger;
                if(it==old_value.end())
                    logger=ATPDXY_LOG_NAME(i.name);
                else
                {
                    if(!(i==*it))
                        logger=ATPDXY_LOG_NAME(i.name);
                    else
                        continue;
                }
                
                logger->setLevel(i.level);
                if(!i.formatter.empty())
                {
                    logger->setFormatter(i.formatter);
                }

                logger->clearAppenders();
                for(auto& a:i.appenders)
                {
                    atpdxy::LogAppender::ptr ap;
                    if(a.type==1)
                        ap.reset(new FileLogAppender(a.file));
                    else if(a.type==2)
                    {
                        // if(!atpdxy::EnvMgr::getInstance()->has("d"));
                        //     ap.reset(new StdoutLogAppender);
                        // else
                        //     continue;
                        ap.reset(new StdoutLogAppender);
                    }

                    ap->setLevel(a.level);
                    if(!a.formatter.empty())
                    {
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(!fmt->isError())
                            ap->setFormatter(fmt);
                        else
                        {
                            std::cout<<"log.name="<<i.name<<" appender type="<<a.type
                                <<" formatter="<<a.formatter<<" is invalid"<<std::endl;
                        }
                    }
                    logger->addAppender(ap);
                }
            }

            // 删除logger
            for(auto& i:old_value)
            {
                auto it=new_value.find(i);
                if(it==new_value.end())
                {
                    auto logger=ATPDXY_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)0);
                    logger->clearAppenders();
                }
            }
        });
    }
};


static LogIniter __log_init;

std::string LoggerManager::toYamlString()
{
    // MutexType::Lock lock(m_mutex_);
    YAML::Node node;
    for(auto& i:m_loggers_)
        node.push_back(YAML::Load(i.second->toYamlString()));

    std::stringstream ss;
    ss<<node;
    return ss.str();
}
}
