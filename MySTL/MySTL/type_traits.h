#pragma once

#include<type_traits>

namespace mystl
{

/*
* 两个参数，T代表常量的类型；v代表常量的值
* 定义一个静态常量成员，这个成员可以在编译期使用(constexpr)
*/
template<class T,T value>
struct m_integral_constant
{
	static constexpr T value = value;
};

//编译期的true和false值
template<bool flag>
using m_bool_constant = m_integral_constant<bool, flag > ;
typedef m_bool_constant<true> m_true_type;
typedef m_bool_constant<false> m_false_type;

//声明pair结构体
template<class T1,class T2>
struct pair;

//判断一个类型是否是pair（泛化），默认不是，表示值为false
template<class T>
struct is_pair :mystl::m_false_type {};

//特化，当T是mystl::pair<T1,T2>实例是，表示值为true
template<class T1,class T2>
struct is_pair<mystl::pair<T1, T2>> :mystl::m_true_type {};

}
