/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 18:14:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 00:04:35
 * @FilePath: /cpp-library/C++高性能服务器/atpdxy/config/config.cpp
 * @Description:    
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include "config.h"

namespace atpdxy {

// Config::ConfigVarMap Config::s_datas;

// 递归遍历YAML节点，将节点和名称存储到output中
static void listAllMember(const std::string& prefix,const YAML::Node& node,std::list<std::pair<std::string,const YAML::Node>>& output)
{
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678")
        !=std::string::npos)
    {
        ATPDXY_LOG_ERROR(ATPDXY_LOG_ROOT())<<"Config invalid name: "<<prefix<<" : "<<node;
        return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap())
    {
        for(auto it=node.begin();it!=node.end();++it)
        {
            listAllMember(prefix.empty()?it->first.Scalar():prefix+"."+it->first.Scalar(),it->second,output);
        }
    }
}

// 
void Config::loadFromYaml(const YAML::Node& root)
{
    std::list<std::pair<std::string,const YAML::Node>> all_nodes;
    listAllMember("",root,all_nodes); 
    for(auto& i:all_nodes)
    {
        std::string key=i.first;
        if(key.empty())
            continue;
            
        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        ConfigVarBase::ptr var=lookupBase(key);
        if(var)
        {
            if(i.second.IsScalar())
            {
                // 标量值，将值转换并存入对应的配置项中
                var->fromString(i.second.Scalar());
            }
            else
            {
                // 将整个节点转换为字符串并存储
                std::stringstream ss;
                ss<<i.second;
                var->fromString(ss.str());
            }
        }
    }
}

// 查找当前命名的项
ConfigVarBase::ptr Config::lookupBase(const std::string& name)
{
    RWMutexType::ReadLock lock(GetMutex());
    auto it =getDatas().find(name);
    return it==getDatas().end()?nullptr:it->second;
}

void Config::Visit(std::function<void( ConfigVarBase::ptr)> cb)
{
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m=getDatas();
    for(auto it=m.begin();it!=m.end();++it)
        cb(it->second);
}
}
