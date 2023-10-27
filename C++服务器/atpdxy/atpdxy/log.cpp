#include "log.h"

namespace server
{

const char* log_level::to_string(log_level::LEVEL level)
{
    switch(level)
    {
#define XX(name) \
    case log_level::name: \
        return #name;   \
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

logger::logger(const std::string& name)
    :my_name_(name)
{

}
 
void logger::log(log_level::LEVEL level,log_event::ptr event)
{
    if(level>=my_level_)
    {
        for(auto& i:my_appenders_)
        {
            i->log(level,event);
        }
    }
}
 
void logger::debug(log_event::ptr event)
{
    debug(log_level::DEBUG,event);
}
 
void logger::info(log_event::ptr event)
{
    info(log_level::INFO,event);
}
 
void logger::warn(log_event::ptr event)
{
    warn(log_level::WARN,event);
}
 
void logger::error(log_event::ptr event)
{
    error(log_level::ERROR,event);
}
 
void logger::fatal(log_event::ptr event)
{
    FATAL(log_level::FATAL,event);
}

void add_appender(log_appender::ptr appender)
{
    my_appenders_.push_back(appender);
}

void del_appender(log_appender::ptr appender)
{
    for(auto it=my_appenders_.begin();it!=my_appenders_.end();++it)
    {
        if(*it==appender)
        {
            my_appenders_.erase(it);
            break;
        }
    }
}

file_log_appender::file_log_appender(const std::string& filename)
    :my_filename_(filename)
{

}

void file_log_appender::log(std::shared_ptr<logger> logger, log_level::LEVEL level,log_event::ptr event) 
{
    if(level>=my_level_)
        my_file_stream_<<my_format_->format(logger,level,event);
}

bool file_log_appender::reopen()
{
    if(my_file_stream_)
        my_file_stream_.close();
    my_file_stream_.open(my_filename_);
    return !!my_file_stream_;
}
void stdout_log_appender::log(std::shared_ptr<logger> logger,log_level::LEVEL level,log_event::ptr event)
{
    if(level>=my_level_)
    {
        std::cout<<my_format_->format(logger, level,event);
    }
}

log_format::log_format(const std::string& pattern)
    :my_pattern_(pattern)
{
    
}

void log_format::init()
{
    std::vector<std::tuple<std::string,std::string,int>> vec;
    std::string nstr;
    for(size_t i=0;i<my_pattern_.size();++i)
    {
        if(my_pattern_[i]!='%')
        {
            nstr.append(1,my_pattern_[i]);
            continue;
        }

        if((i+1)<my_pattern_.size())
        {
            if(my_pattern_[i+1]=='%')
            {
                nstr.append(1,'%');
                continue;
            }
        }

        size_t n=i+1;
        int fmt_status=0;
        size_t fmt_begin=0;

        std::string str;
        std::string fmt;

        while(n<my_pattern_.size())
        {
            if(isspace(my_pattern_[n]))
                break;
            if(fmt_status==0)
            {
                if(my_pattern_[n]=='{')
                {
                    str=my_pattern_.substr(i+1,n-i-1);
                    fmt_status=1;           //解析格式
                    fmt_begin=n;
                    ++n;
                    continue;
                }
            }
            if(fmt_status==1)
            {
                if(my_pattern_[n]=='}')
                {
                    fmt=my_pattern_.substr(fmt_begin+1,n-fmt_begin-1);
                    fmt_status=2;
                    break;
                }
            }
        }
        if(fmt_status==0)
        {
            if(!nstr.empty())
                vec.push_back(std::make_tuple(nstr,"",0));
            str=my_pattern_.substr(i+1,n-i-1);
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n;
        }
        else if(fmt_status==1)
        {
            std::cout<<"pattern parse error: "<<my_pattern_<<"-"<<my_pattern_.substr(i)<<std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>",fmt,1));
        }
        else if(fmt_status==2)
        {
            if(!nstr.empty())               
                vec.push_back(std::make_tuple(nstr,"",0));
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n;
        }
    }
    if(!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr,"",0));
    }
    //$m--消息；%p-level；%c--启动后的时间；%t--线程ID；%n--回车换行；%d--时间；%f--文件名；%l--行号
    

}

class message_format_item:public log_format::format_item
{
public:
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event) override
    {
        os<<event->get_content();
    }

};

class level_format_item:public log_format::format_item
{
public:
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event) override
    {
        os<<log_level::to_string(level);
    }
};

class elapse_format_item:public log_format::format_item
{
public:
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event)
    {
        os<<event->get_elapse();
    }
};

class elapse_format_item:public log_format::format_item
{                 
public:           
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event)
    {             
        os<<event->get_name();
                        
    }    
};                                                                                               

class thread_format_item:public log_format::format_item
{                   
public:  
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event)
    {             
        os<<event->get_threadid();               
    }        
};                                                                                              

class fiber_format_item:public log_format::format_item
{                 
public:
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event)
    {           
        os<<event->get_fiberid();                                 
    }          
};                               

class datetime_format_item:public log_format::format_item
{                 
public:
    void format(std::ostream& os,logger::ptr logger,log_level::LEVEL level,log_event::ptr event)
    {           
        os<<event->get_time();                      
    }      
};                                                                                            

std::string log_format::format(std::shared_ptr<logger> logger,log_level::LEVEL level,log_event:: ptr event)
{
    std::stringstream ss;
    for(auto& i:my_items_)
    {
        i->format(ss,level,event);
    }
    return ss.str();
}
}


