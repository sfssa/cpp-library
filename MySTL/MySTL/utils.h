#pragma once

/*
* 这个文件提供通用工具函数和pair函数结构，包括右值引用、完美转发、交换函数
* 
* move-实现右值引用，将左值强制转换为右值引用
* 
* forward-实现完美转发，将参数以及原始左值或右值引用传递给其他函数，保留值类别（左值或右值）
* 
* swap-交换两个对象的值的通用函数
* 
* pair-存储两个值的数据结构，包括first，second
* 
* T&&是右值引用类型，支持移动语义，但在具体的上下文不仅可以表示右值引用，还可以表示左值引用
*/
#include <cstddef>
#include "type_traits.h"

namespace mystl
{

/*
* typename std::remove_reference<T>::type&&表示函数返回值是一个右值引用
* remove_reference函数负责从模板T中移除引用得到移除引用后的类型
* move(T&& arg)move函数接受一个右值引用的参数
* noexcept关键字代表函数不抛出异常，承诺这个函数不引发异常
* 函数内部将右值引用arg强制转换成右值引用并返回
*/
template<class T>
typename std::remove_reference<T>::type&& move(T&& arg) noexcept
{
	return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

/*
* std::remove_reference<T>::type& 接受类型T，如果有引用就把引用去掉，返回一个没有引用的类型
* 函数返回arg的右值引用
* forward核心是根据传入参数的类型，返回相应的左值引用或者右值引用
* 将左值引用转换成右值引用
*/
template<class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept
{
	return static_cast<T&&>(arg);
}
/*
* 这里详细说一下，在万能引用和引用折叠基础上，如果传入的参数是一个左值，假设传递的参数是int
* 左值引用：将T换成int&，那么结果就是int& &&，经过引用折叠就是int&，表示左值引用
* 右值引用：将T换成int，那么结果就是int &&，表示右值引用
*/

//将右值引用转换为右值引用，如果T是左值那么就触发断言
template<class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
{
	static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
	return static_cast<T&&>(arg);
}

//swap函数
template<class Tp>
void swap(Tp& lhs, Tp& rhs)
{
	auto tmp(mystl::move(lhs));
	lhs = mystl::move(rhs);
	rhs = mystl::move(tmp);
}


//这个函数用来将first1-last1之间的内容和以first2起始的元素进行交换
template<class ForwardIter1,class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
{
	//这里的void不是必须的，是为了消除编译器可能产生的“未使用变量”警告
	for (; first1 != last1; ++first1, (void)++first2)
		mystl::swap(*first1, *first2);
	return first2;
}

//将等长数组的内容进行交换
template<class Tp, size_t N>
void swap(Tp(&a)[N], Tp(&b)[N])
{
	mystl::swap_range(a, a + N, b);
}

//定义pair
template<class Ty1,class Ty2>
struct pair
{
	typedef Ty1 first_type;
	typedef Ty2 second_type;

	first_type first;
	second_type second;

	/*
	* 模板类构造函数，构造一个pair类型对象
	* 
	* std::is_default_constructible<Other1>::value检查是否可以调用默认构造，
	* 可以就将value赋值为true；反之value=false
	* 
	* std::enable_if函数如果第一个参数的条件为真，那么就将type_info设置为第二个参数（void）
	* 
	* typename = 声明一个没有名字的模板参数
	* 
	* 如果enable_if条件为真，那么type_info=void，允许启用模板；否则禁止启用模板
	* 
	* constexpr指示编译器在编译时计算表达式的值或执行函数，以便在运行时之前确定其结果，
	* 修饰类构造函数时可以保证在编译时调用构造函数，可以提高性能
	*/
	template<class Other1=Ty1,class Other2=Ty2,
		typename = typename std::enable_if<
		std::is_default_constructible<Other1>::value &&
		std::is_default_constructible<Other2>::value,void>::type_info>
	constexpr pair()
		:first(), second()//都有默认构造函数
	{}

	template<class U1=Ty1,class U2=Ty2,
		typename std::enable_if<
		std::is_copy_constructible<U1>::value&&		//是否有拷贝构造
		std::is_copy_constructible<U2>::value&&
		std::is_convertible<const U1&,Ty1>::value&&	//是否可以隐式转换
		std::is_convertible<const U2&,Ty2>::value,int>::type=0>
	constexpr pair(const Ty1& a, const Ty2& b)
		:first(a), second(b)
	{}
	
	template<class U1 = Ty1, class U2 = Ty2,
		typename std::enable_if <
		std::is_copy_constructible<U1>::value&&
		std::is_copy_constructible<U2>::value &&
		(!std::is_convertible<const U1&, Ty1>::value ||
		 !std::is_convertible<const U2&, Ty2>::value), int>::type = 0>
	explicit constexpr pair(const Ty1& a, const Ty2& b)
		:first(a), second(b)
	{}

	pair(const pair& lhs) = default;
	pair(pair&& rhs) = default;

	template<class Other1, class Other2,
		typename std::enable_if<
		std::is_constructible<Ty1, Other1>::value&&
		std::is_constructible<Ty2, Other2>::value&&
		std::is_convertible<Other1&&, Ty1>::value&&
		std::is_convertible<Other2&&, Ty2>::value, int>::type = 0>
	constexpr pair(Other1&& a,Other2&& b)
		:first(mystl::forward<Other1>(a)),
		second(mystl::forward<Other2>(b))
	{}

	template<class Other,class Other2>
	typename std::enable_if<
		std::is_constructible<Ty1, Other1>::value&&
		std::is_constructible<Ty2, Other2>::value &&
		(!std::is_convertible<Other, Ty1>::value ||
		 !std::is_convertible<Other2, Ty2>::value), int>::type = 0 >
		explicit constexpr pair(Other1 && a, Other2 && b)
		:first(mystl::forward<Other1>(a)),second(mystl::forward<Other2>(b))
	{}

	template<class Other1,class Other2>
	typename std::enable_if<
		std::is_constructible<Ty1, const Other1&>::value&&
		std::is_constructible<Ty2, const Other2&>::value&&
		std::is_convertible<const Other1&, Ty1>::value&&
		std::is_convertible<const Other2&, Ty2>::value, int>::type = 0>
		constexpr pair(const pair<Other1,Other2>& other)
		:first(other.first),second(other.second)
	{}

	template<class Other1,class Other2>
	typename std::enable_if<
		std::is_constructible<Ty1,const Other1&>::value &&
		std::is_constructible<Ty2,const Other2&>::value &&
		(!std::is_convertible<const Other1&,Ty1>::value||
		 !std::is_convertible<const Other2&,Ty2>::value),int>::type=0>
		explicit constexpr pair(const pair<Other1,Other2>& other)
		:first(other.first),second(other.second)
	{}

	template<class Other1,class Other2,
		typename std::enable_if<
		std::is_constructible<Ty1,Other1>::value &&
		std::is_constructible<Ty2,Other2>::value &&
		std::is_convertible<Other1,Ty1>::value &&
		std::is_convertible<Other2,Ty2>::value,int>::type=0>
	constexpr pair(pair<Other1, Other2>&& other)
		:first(mystl::forward<Other1>(other.first)),
		second(mystl::forward<Other2>(other.second))
	{}

	template<class Other1, class Other2,
		typename std::enable_if<
		std::is_constructible<Ty1,Other1>::value &&
		std::is_constructible<Ty2,Other2>::value &&
		(!std::is_convertible<Other1,Ty1>::value ||
		 !std::is_convertible<Other2,Ty2>::value),int>::type=0>
	explicit constexpr pair(pair<Other1,Other2>&& other)
		:first(mystl::forward<Other1>(other.first)),
		second(mystl::forward<Other2>(other.second))
	{}

	//重载操作符
	pair& operator=(const pair& lhs)
	{
		if (this != &lhs)
		{
			first = lhs.first;
			second = lhs.second;
		}
		return *this;
	}

	pair& operator=(pair&& rhs)
	{
		if (this != &rhs)
		{
			first = mystl::move(rhs.first);
			second = mystl::move(rhs.second);
		}
		return *this;
	}

	template<class Other1,class Other2>
	pair& operator=(pair<Other1, Other2>&& other)
	{
		first = mystl::forward<Other1>(other.first);
		second = mystl::forward<Other2>(other.second);
		return *this;
	}

	template<class Other1,class Other2>
	pair& operator=(const pair<Other1, Other2>& other)
	{
		first = other.first;
		second = other.second;
		return *this;
	}

	~pair() = default;

	void swap(pair& other)
	{
		if (this != &other)
		{
			mystl::swap(first, other.first);
			mystl::swap(second, other.second);
		}
	}
};

template<class Ty1,class Ty2>
bool operator==(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return lhs.first == rhs.first && lhs.second == rhs.second;
}

template<class Ty1, class Ty2>
bool operator<(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
}

template<class Ty1, class Ty2>
bool operator!=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return !(lhs == rhs);
}

template<class Ty1, class Ty2>
bool operator>(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return rhs < lhs;	//用已有的函数
}

template<class Ty1, class Ty2>
bool operator<=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return !(rhs<lhs)
}

template<class Ty1, class Ty2>
bool operator>=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
	return !(lhs < rhs)
}

template<class Ty1,class Ty2>
void swap(pair<Ty1, Ty2>& lhs, pair<Ty1, Ty2>& rhs)
{
	lhs.swap(rhs);
}

//构造pair
template<class Ty1,class Ty2>
pair<Ty1, Ty2>make_pair(Ty1&& first, Ty2&& second)
{
	return pair<Ty1, Ty2>(mystl::forward<Ty1>(first), mystl::forward<Ty2>(second));
}

}