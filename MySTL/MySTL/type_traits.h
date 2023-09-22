#pragma once

#include<type_traits>

namespace mystl
{

/*
* ����������T�����������ͣ�v��������ֵ
* ����һ����̬������Ա�������Ա�����ڱ�����ʹ��(constexpr)
*/
template<class T,T value>
struct m_integral_constant
{
	static constexpr T value = value;
};

//�����ڵ�true��falseֵ
template<bool flag>
using m_bool_constant = m_integral_constant<bool, flag > ;
typedef m_bool_constant<true> m_true_type;
typedef m_bool_constant<false> m_false_type;

//����pair�ṹ��
template<class T1,class T2>
struct pair;

//�ж�һ�������Ƿ���pair����������Ĭ�ϲ��ǣ���ʾֵΪfalse
template<class T>
struct is_pair :mystl::m_false_type {};

//�ػ�����T��mystl::pair<T1,T2>ʵ���ǣ���ʾֵΪtrue
template<class T1,class T2>
struct is_pair<mystl::pair<T1, T2>> :mystl::m_true_type {};

}
