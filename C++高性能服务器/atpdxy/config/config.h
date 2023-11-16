/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-16 22:14:51
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/config/config.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <list>
#include "../log/log.h"
#include "../utils/utils.h"

namespace atpdxy
{

class ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    ConfigVarBase(const std::string& name,const std::string& description="")
        :m_name(name),m_description(description)
    {
        // 保证key都是小写没有大写
        std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
    }

    virtual ~ConfigVarBase(){}

    const std::string& getName() const {return m_name;}

    const std::string& getDescription() const {return m_description;}

    // 将值转换为字符串
    virtual std::string toString()=0;

    // 从字符串初始化值
    virtual bool fromString(const std::string& val)=0;

    // 返回配置参数值的类型名称
    virtual std::string getTypeName() const=0;
protected:
    std::string m_name;         // 配置参数的名称
    std::string m_description;  // 配置参数的描述
};

// 从F转换成T
template <class F,class T>
class LexicalCast
{
public:
    T operator()(const F& v)
    {
        return boost::lexical_cast<T>(v);
    }
private:

};

// vector偏特化
template <class T>
class LexicalCast<std::string,std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i)
        {
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::vector<T>,std::string>
{
public:
    std::string operator() (const std::vector<T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// list偏特化
template <class T>
class LexicalCast<std::string,std::list<T>>
{
public:
    std::list<T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i)
        {
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::list<T>,std::string>
{
public:
    std::string operator() (const std::list<T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// set特化版本
template <class T>
class LexicalCast<std::string,std::set<T>>
{
public:
    std::set<T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i)
        {
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::set<T>,std::string>
{
public:
    std::string operator() (const std::set<T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// unordered_set特化版本
template <class T>
class LexicalCast<std::string,std::unordered_set<T>>
{
public:
    std::unordered_set<T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i)
        {
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>,std::string>
{
public:
    std::string operator() (const std::unordered_set<T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// map特化版本
template <class T>
class LexicalCast<std::string,std::map<std::string,T>>
{
public:
    std::map<std::string,T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::map<std::string,T> vec;
        std::stringstream ss;
        for(auto it=node.begin();it!=node.end();++it)
        {
            ss.str("");
            ss<<it->second;
            vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::map<std::string,T>,std::string>
{
public:
    std::string operator() (const std::map<std::string,T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i.second)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// unordered_map特化版本
template <class T>
class LexicalCast<std::string,std::unordered_map<std::string,T>>
{
public:
    std::unordered_map<std::string,T> operator()(const std::string& v)
    {
        YAML::Node node=YAML::Load(v);
        typename std::unordered_map<std::string,T> vec;
        std::stringstream ss;
        for(auto it=node.begin();it!=node.end();++it)
        {
            ss.str("");
            ss<<it->second;
            vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string,T>,std::string>
{
public:
    std::string operator() (const std::unordered_map<std::string,T>& v)
    {
        YAML::Node node;
        for(auto& i: v)
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i.second)));

        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// 后两个类是为了转换成string和从string转换成我们要的
// FromStr T operator() (const std::string&)
// ToStr std::string operator() (const T&)
template <class T,class FromStr=LexicalCast<std::string,T>
        ,class ToStr=LexicalCast<T,std::string>>
class ConfigVar:public ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    // 当配置发生变化的回调函数
    typedef std::function<void (const T& old_value,const T& new_value)> on_change_cb;
    
    ConfigVar(const std::string& name,const T& default_value,const std::string& description="")
        :ConfigVarBase(name,description),m_val(default_value)
    {

    }

    const T getValue() const {return m_val;}

    void setValue(const T& val) 
    {
        if(val==m_val)
        {
            return;
        }
        for(auto& i : m_cbs)
            i.second(m_val,val);// 执行回调函数
        m_val=val;
    }

    std::string getTypeName() const override {return typeid(T).name();}

    std::string toString() override
    {
        try{
            // return boost::lexical_cast<std::string>(m_val);
            // 创造一个ToStr()实例，然后通过operator()调用这个实例(仿函数)
            return ToStr()(m_val);
        }catch(std::exception& e){
            ATPDXY_LOG_ERROR(ATPDXY_LOG_ROOT())<<"ConfigVar::toString exception"
                <<e.what()<<" convert: "<<typeid(m_val).name()<<" to string.";
        }
        return "";
    }

    bool fromString(const std::string& val) override
    {
        try{
            // m_val=boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        }catch(std::exception& e){
            ATPDXY_LOG_ERROR(ATPDXY_LOG_ROOT())<<"ConfigVar::toString exception"
                <<e.what()<<" convert: string to "<<typeid(m_val).name();
        }
        return false;
    }

    // void addListener(uint64_t key,on_change_cb cb)
    // {
    //     m_cbs[key]=cb;
    // }
    uint64_t addListener(on_change_cb cb)
    {
        static uint64_t s_fun_id=0;
        ++s_fun_id;
        m_cbs[s_fun_id]=cb;
        return s_fun_id;
    }

    void delListener(uint64_t key)
    {
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key)
    {
        auto it=m_cbs.find(key);
        return it==m_cbs.end()?nullptr:it->second;
    }

    void clearListener()
    {
        m_cbs.clear();
    }
private:
    T m_val;
    // 变更回调函数组，函数指针没有比较函数，因此采用map来解决，通过key找到函数指针来删除
    std::map<uint64_t,on_change_cb> m_cbs;
};

class Config
{
public: 
    // 配置项的名称和配置项的智能指针
    typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;

    /*
        这个函数首先调用了 lookUp<T>(name) 函数，尝试查找是否已经存在该配置项。
        如果存在，直接返回，否则，它会检查 name 是否合法（是否包含除了字母、数字、
        点和下划线之外的字符）。如果合法，它会创建一个新的 ConfigVar<T> 对象，
        并将其存储到 s_datas（配置项的容器）中。
    */
    template<class T>
    static typename ConfigVar<T>::ptr lookUp(const std::string& name,const T& default_value,
        const std::string& description="")
    {
        // auto tmp=lookUp<T>(name);
        auto it=getDatas().find(name);
        if(it!=getDatas().end())
        {
            // 将it->second转换成ConfigVar，it->second是一个基类指针，T可能与实际数据类型不符合
            // it->second是一个ConfigVarBase类型的智能指针，但期望它是一个ConfigVar类型的对象，
            // 通过动态转换，如果可以成功转换，证明是一个需要的对象，否则存储的对象不是ConfigVar，
            // 返回空指针
            auto tmp=std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(tmp)
            {
                ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"lookUp name="<<name<<" exists";
                return tmp;
            }
            else
            {
                ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"lookUp name="<<name<<" exists but type not "
                    <<typeid(T).name()<<" real_type="<<it->second->getTypeName()
                    <<" "<<it->second->toString();
                return nullptr;
            }
        }
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678")!=std::string::npos)
        {
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"lookUp name invalid "<<name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
        getDatas()[name]=v;
        return v;
    }

    /*
        这个函数是用于查找已经存在的配置项的。它根据 name 在 s_datas 中查找，如果找到了，
        就返回相应类型的 ConfigVar 智能指针。如果找不到，返回 nullptr。
    */
    template <class T>
    static typename ConfigVar<T>::ptr lookUp(const std::string& name)
    {
        auto it=getDatas().find(name);
        if(it==getDatas().end())
            return nullptr;
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void loadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr lookupBase(const std::string& name);
private:
    static ConfigVarMap& getDatas()
    {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

}


