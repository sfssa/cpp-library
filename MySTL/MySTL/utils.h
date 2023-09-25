#pragma once

/*
* ����ļ��ṩͨ�ù��ߺ�����pair�����ṹ��������ֵ���á�����ת������������
* 
* move-ʵ����ֵ���ã�����ֵǿ��ת��Ϊ��ֵ����
* 
* forward-ʵ������ת�����������Լ�ԭʼ��ֵ����ֵ���ô��ݸ���������������ֵ�����ֵ����ֵ��
* 
* swap-�������������ֵ��ͨ�ú���
* 
* pair-�洢����ֵ�����ݽṹ������first��second
* 
* T&&����ֵ�������ͣ�֧���ƶ����壬���ھ���������Ĳ������Ա�ʾ��ֵ���ã������Ա�ʾ��ֵ����
*/
#include <cstddef>
#include "type_traits.h"

namespace mystl
{

/*
* typename std::remove_reference<T>::type&&��ʾ��������ֵ��һ����ֵ����
* remove_reference���������ģ��T���Ƴ����õõ��Ƴ����ú������
* move(T&& arg)move��������һ����ֵ���õĲ���
* noexcept�ؼ��ִ��������׳��쳣����ŵ��������������쳣
* �����ڲ�����ֵ����argǿ��ת������ֵ���ò�����
*/
template<class T>
typename std::remove_reference<T>::type&& move(T&& arg) noexcept
{
	return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

/*
* std::remove_reference<T>::type& ��������T����������þͰ�����ȥ��������һ��û�����õ�����
* ��������arg����ֵ����
* forward�����Ǹ��ݴ�����������ͣ�������Ӧ����ֵ���û�����ֵ����
* ����ֵ����ת������ֵ����
*/
template<class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept
{
	return static_cast<T&&>(arg);
}
/*
* ������ϸ˵һ�£����������ú������۵������ϣ��������Ĳ�����һ����ֵ�����贫�ݵĲ�����int
* ��ֵ���ã���T����int&����ô�������int& &&�����������۵�����int&����ʾ��ֵ����
* ��ֵ���ã���T����int����ô�������int &&����ʾ��ֵ����
*/

//����ֵ����ת��Ϊ��ֵ���ã����T����ֵ��ô�ʹ�������
template<class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
{
	static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
	return static_cast<T&&>(arg);
}

//swap����
template<class Tp>
void swap(Tp& lhs, Tp& rhs)
{
	auto tmp(mystl::move(lhs));
	lhs = mystl::move(rhs);
	rhs = mystl::move(tmp);
}


//�������������first1-last1֮������ݺ���first2��ʼ��Ԫ�ؽ��н���
template<class ForwardIter1,class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
{
	//�����void���Ǳ���ģ���Ϊ���������������ܲ����ġ�δʹ�ñ���������
	for (; first1 != last1; ++first1, (void)++first2)
		mystl::swap(*first1, *first2);
	return first2;
}

//���ȳ���������ݽ��н���
template<class Tp, size_t N>
void swap(Tp(&a)[N], Tp(&b)[N])
{
	mystl::swap_range(a, a + N, b);
}

//����pair
template<class Ty1,class Ty2>
struct pair
{
	typedef Ty1 first_type;
	typedef Ty2 second_type;

	first_type first;
	second_type second;

	/*
	* ģ���๹�캯��������һ��pair���Ͷ���
	* 
	* std::is_default_constructible<Other1>::value����Ƿ���Ե���Ĭ�Ϲ��죬
	* ���Ծͽ�value��ֵΪtrue����֮value=false
	* 
	* std::enable_if���������һ������������Ϊ�棬��ô�ͽ�type_info����Ϊ�ڶ���������void��
	* 
	* typename = ����һ��û�����ֵ�ģ�����
	* 
	* ���enable_if����Ϊ�棬��ôtype_info=void����������ģ�壻�����ֹ����ģ��
	* 
	* constexprָʾ�������ڱ���ʱ������ʽ��ֵ��ִ�к������Ա�������ʱ֮ǰȷ��������
	* �����๹�캯��ʱ���Ա�֤�ڱ���ʱ���ù��캯���������������
	*/
	template<class Other1=Ty1,class Other2=Ty2,
		typename = typename std::enable_if<
		std::is_default_constructible<Other1>::value &&
		std::is_default_constructible<Other2>::value,void>::type_info>
	constexpr pair()
		:first(), second()//����Ĭ�Ϲ��캯��
	{}

	template<class U1=Ty1,class U2=Ty2,
		typename std::enable_if<
		std::is_copy_constructible<U1>::value&&		//�Ƿ��п�������
		std::is_copy_constructible<U2>::value&&
		std::is_convertible<const U1&,Ty1>::value&&	//�Ƿ������ʽת��
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

	//���ز�����
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
	return rhs < lhs;	//�����еĺ���
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

//����pair
template<class Ty1,class Ty2>
pair<Ty1, Ty2>make_pair(Ty1&& first, Ty2&& second)
{
	return pair<Ty1, Ty2>(mystl::forward<Ty1>(first), mystl::forward<Ty2>(second));
}

}