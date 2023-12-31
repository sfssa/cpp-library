/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-13 22:52:46
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-19 00:24:37
 * @FilePath: /cpp-library/C++高性能服务器/test/test_config.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include <iostream>
#include "./atpdxy/log/log.h"
#include "./atpdxy/config/config.h"
#include <yaml-cpp/yaml.h>

#if 0
atpdxy::ConfigVar<int>::ptr g_int_value_config =
    atpdxy::Config::lookUp("system.port", (int)8080, "system port");

atpdxy::ConfigVar<float>::ptr g_int_valuex_config =
    atpdxy::Config::lookUp("system.port", (float)8080, "system port");

atpdxy::ConfigVar<float>::ptr g_float_value_config =
    atpdxy::Config::lookUp("system.value", (float)10.2f, "system value");

atpdxy::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
    atpdxy::Config::lookUp("system.int_vec", std::vector<int>{1,2}, "system int vec");

atpdxy::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    atpdxy::Config::lookUp("system.int_list", std::list<int>{1,2}, "system int list");

atpdxy::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    atpdxy::Config::lookUp("system.int_set", std::set<int>{1,2}, "system int set");

atpdxy::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    atpdxy::Config::lookUp("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

atpdxy::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    atpdxy::Config::lookUp("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

atpdxy::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    atpdxy::Config::lookUp("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");

void print_yaml(const YAML::Node& node,int level)
{
    if(node.IsScalar())
    {
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<std::string(level*4,' ')
            <<node.Scalar()<<" - "<<node.Type()<<" - "<<level;
    }
    else if(node.IsNull())
    {
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<std::string(level*4,' ')
            <<"NULL - "<<node.Type()<<" - "<<level;
    }
    else if(node.IsMap())
    {
        for(auto it =node.begin();it!=node.end();++it)
        {
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<std::string(level*4,' ')
                <<it->first<<" - "<<it->second.Type()<<" - "<<level;
            print_yaml(it->second,level+1);            
        }
    }
    else if(node.IsSequence())
    {
        for(size_t i=0;i<node.size();++i)
        {
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<std::string(level*4,' ')
                <<i<<" - "<<node[i].Type()<<" - "<<level;
            print_yaml(node[i],level+1);
        }
    }
}

void test_yaml()
{
    YAML::Node root=YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/log.yml");
    print_yaml(root,0);
}

void test_config()
{
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_int_value_config->getValue();
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_float_value_config->toString();

    // auto v=g_int_vec_value_config->getValue();
    // for(auto& i :v)
    //     ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before int_vec: "<<i;
    // YAML::Node root =YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/test.yml");
    // atpdxy::Config::loadFromYaml(root);
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<root.Scalar();

    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_int_value_config->getValue();
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_float_value_config->toString();

    // v=g_int_vec_value_config->getValue();
    // for(auto& i:v)
    //     ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"after int_vec: "<<i;
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_int_value_config->getValue();
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_float_value_config->toString();
#define XX(g_var,name,prefix) \
    { \
        auto& v=g_var->getValue(); \
        for(auto& i : v){ \
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<#prefix " "#name ": "<<i; \
        } \
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<#prefix " " #name " yaml: "<<g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    XX(g_int_vec_value_config,int_vec,before);
    XX(g_int_list_value_config,int_list,before);
    XX(g_int_set_value_config,int_set,before);
    XX(g_int_uset_value_config,int_uset,before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root =YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/log.yml");
    atpdxy::Config::loadFromYaml(root);

    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"after: "<<g_int_value_config->getValue();
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"after: "<<g_float_value_config->toString();

    XX(g_int_vec_value_config,int_vec,after);
    XX(g_int_list_value_config,int_list,after);
    XX(g_int_set_value_config,int_set,after);
    XX(g_int_uset_value_config,int_uset,after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}
#endif

class Person {
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
};

namespace atpdxy
{

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

atpdxy::ConfigVar<Person>::ptr g_person =
    atpdxy::Config::lookUp("class.person", Person(), "system person");

atpdxy::ConfigVar<std::map<std::string, Person> >::ptr g_person_map =
    atpdxy::Config::lookUp("class.map", std::map<std::string, Person>(), "system person");

atpdxy::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map =
    atpdxy::Config::lookUp("class.vec_map", std::map<std::string, std::vector<Person> >(), "system person");

void test_class()
{
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before:"<<g_person->getValue().toString()<<" - "<<g_person->toString();
    // YAML::Node root=YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/test.yml");
    // atpdxy::Config::loadFromYaml(root);
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"after:"<<g_person->getValue().toString()<<" - "<<g_person->toString();
#define XX_PM(g_var, prefix) \
    { \
        auto m = g_person_map->getValue(); \
        for(auto& i : m) { \
            ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) <<  prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) <<  prefix << ": size=" << m.size(); \
    }

    g_person->addListener([](const Person& old_value,const Person& new_value){
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"old_value:"<<old_value.toString()
            <<" new_value:"<<new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) << "before: " << g_person_vec_map->toString();
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_person_map->toString();
    YAML::Node root=YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/log.yml");
    atpdxy::Config::loadFromYaml(root);
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"before: "<<g_person_map->toString();
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
    
}

void test_log()
{
    static atpdxy::Logger::ptr system_log=ATPDXY_LOG_NAME("system");
    ATPDXY_LOG_INFO(system_log)<<"hello system"<<std::endl;
    std::cout<<atpdxy::LoggerMgr::getInstance()->toYamlString()<<std::endl;
    YAML::Node root=YAML::LoadFile("/home/pzx/GitHub/cpp-library/C++高性能服务器/bin/conf/log.yml");
    atpdxy::Config::loadFromYaml(root);
    std::cout<<"=========================="<<std::endl;
    std::cout<<atpdxy::LoggerMgr::getInstance()->toYamlString()<<std::endl;
    std::cout<<root<<std::endl;
    ATPDXY_LOG_INFO(system_log)<<"hello system"<<std::endl;
}

int main()
{
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<g_int_value_config->getValue();
    // ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<g_float_value_config->toString();
    // test_yaml();
    // test_config();
    // test_class();
    test_log();
    atpdxy::Config::Visit([](atpdxy::ConfigVarBase::ptr var){
        ATPDXY_LOG_INFO(ATPDXY_LOG_ROOT())<<"name="<<var->getName()
            <<" description:"<<var->getDescription()
            <<" typename="<<var->getTypeName()
            <<" value="<<var->toString();
    });
    return 0;
}
