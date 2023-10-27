#pragma once

//本文件包含基本算法

#include "iterator.h"
#include "utils.h"
#include <cstring>

namespace mystl
{

/*
* C++中标准库提供了std::max和std::min函数用来比较取最大值和最小值
* 如果在代码中定义了max和min宏，就取消对其的定义，避免和标准库冲突
*/
#ifdef max
#pragma message("#dundefing marco max")
#undef max
#endif 

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif 

//求最大值和最小值
template<class T>
const T& max(const T& lhs, const T& rhs)
{
	return lhs < rhs ? rhs : lhs;
}

template<class T,class Compare>
const T& max(const T& lhs, const T& rhs, Compare comp)
{
	return comp(lhs, rhs) ? rhs : lhs;
}

template<class T>
const T& min(const T& lhs, const T& rhs)
{
	return rhs < lhs ? rhs : lhs;
}

template<class T,class Compare>
const T& min(const T& lhs, const T& rhs,Compare comp)
{
	return comp(lhs, rhs) ? rhs : lhs;
}

//将迭代器指向的对象调换
template<class FIter1,class FIter2>
void iter_swap(FIter1 lhs, FIter2 rhs)
{
	mystl::swap(*lhs, *rhs);
}

//拷贝,将[first,last)拷贝到以result开始的地址
template<class InputIter,class OutputIter>
OutputIter unchecked_copy_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag)
{
	for (; first != last; ++first, ++result)
	{
		*result = *first;
	}
	return result;
}

template<class RandomIter,class OutputIter>
OutputIter unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result, mystl::random_access_iterator_tag)
{
	for (auto n = last - first; n > 0; --n, ++first, ++result)
	{
		*result = *first;
	}
	return result;
}

template<class InputIter,class OutputIter>
OutputIter unchecked_copy_cat(InputIter first, InputIter last, OutputIter result)
{
	return unchecked_copy_cat(first, last, result, iterator_category(first));
}

template <class Tp,class Up>
typename std::enable_if<
	//判断输入和输出元素类型是否匹配并且支持平凡复制赋值操作
	std::is_same<typename std::remove_const<Tp>::type,Up>::value &&
	std::is_trivially_copy_assignable<Up>::value,
	Up*>::type
	unchecked_copy_cat(Tp* first, Tp* last, Up* result)
{
	//memmove 函数用于在内存中复制一块数据，即使源地址和目标地址有重叠也能够安全地执行。
	const auto n = static_cast<size_t>(last - first);
	if (n != 0)
		std::memmove(result, first, n * sizeof(Up));
	return result + n;
}

template <class InputIter,class OutputIter>
OutputIter copy(InputIter first, InputIter last, OutputIter result)
{
	return unchecked_copy_cat(first, last, result);
}

//将[first,last)内容复制到[result-(last-first),result)中
template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2 
unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
	BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
{
	while (first != last)
	{
		*--result = *--last;
	}
	return result;
}

template <class RandomIter1,class BidirectionalIter2>
BidirectionalIter2
unchecked_copy_backward_cat(RandomIter1 first, RandomIter1 last,
	BidirectionalIter2 result, mystl::random_access_iterator_tag)
{
	for (auto n = last - first; n > 0; --n)
	{
		*--result = *--last;
	}
	return result;
}

template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2
unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
	BidirectionalIter2 result)
{
	return unchecked_copy_backward_cat(first, last, result, iterator_category(first));
}

//为tribially_copy_assignable类型提供特化版本
template<class Tp,class Up>
typename std::enable_if<
	std::is_same<typename std::remove_const<Tp>::type,Up>::value &&
	std::is_trivially_copy_assignable<Up>::value,Up*>::type
	unchecked_copy_backward(Tp* first, Tp* last, Up* result)
{
	const auto n = static_cast<size_t>(last - first);
	if (n != 0)
	{
		result -= n;
		std::memmove(result, first, n * sizeof(Up));
	}
	return result;
}

template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2 
copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
{
	return unchecked_copy_backward(first, last, result);
}
/*
* 上面的unchecked_copy_backward是一个辅助函数，处理不同类型的迭代器，通过iterator_category
* 来确定迭代器类型，然后调用对应版本来执行具体的复制 
* 
* copy_backward是一个公开的接口，没有显式处理不同迭代器类型，提供简单的接口来让用户直接调用，
* 不必关心具体的迭代器类型
*/
//将[fist,last)满足一元操作unary_pred的元素拷贝到result起始地址上
template<class InputIter,class OutputIter,class UnaryPredicate>
OutputIter
copy_if(InputIter first, InputIter last, OutputIter result, UnaryPredicate unary_pred)
{
	for (; first != last; ++first)
	{
		if (unary_pred(*first))
			*result++ = *first;
	}
	return result;
}

//将[first,first+n)的元素拷贝到[result,result+n)上，返回值指向拷贝源和目的的尾部
template<class InputIter,class Size,class OutputIter>
mystl::pair<InputIter,OutputIter>
unchecked_copy_n(InputIter first, Size n, OutputIter result, mystl::input_iterator_tag)
{
	for (; n > 0; --n, ++first, ++result)
	{
		*result = *first;
	}
	return mystl::pair<InputIter, OutputIter>(first, result);
}

template<class RandomIter,class Size,class OutputIter>
mystl::pair<RandomIter,OutputIter>
unchecked_copy_n(RandomIter first, Size n, OutputIter result, mystl::random_access_iterator_tag)
{
	auto last = first + n;
	return mystl::pair<RandomIter, OutputIter>(last, mystl::copy(first, last, result));
}

template<class InputIter, class Size, class OutputIter>
mystl::pair<InputIter,OutputIter>
copy_n(InputIter first, Size n, OutputIter result)
{
	return unchecked_copy_n(first, n, result, iterator_category(first));
}

//将[first,last)区间内的元素移动到[result,result+(last-first))内
template<class InputIter,class OutputIter>
OutputIter unchecked_move_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag)
{
	for (; first != last; ++first, ++result)
	{
		*result = mystl::move(*first);
	}
	return result;
}

template<class RandomIter,class OutputIter>
OutputIter unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result, mystl::random_access_iterator_tag)
{
	for (auto n = last - first; n > 0; ++first, ++result)
	{
		*result = mystl::move(*first);
	}
	return result;
}

template<class InputIter,class OutputIter>
OutputIter unchecked_move(InputIter first, InputIter last, OutputIter result)
{
	return unchecked_move_cat(first, last, result, iterator_category(first));
}

template<class Tp,class Up>
typename std::enable_if<
	std::is_same<typename std::remove_const<Tp>::type,Up>::value &&
	std::is_trivially_move_assignable<Up>::value,Up*>::type
	unchecked_move(Tp* first, Tp* last, Up* result)
{
	const size_t n = static_cast<size_t>(last - first);
	if (n != 0)
		std::memmove(result, first, n * sizeof(Up));
	return result + n;
}

template<class InputIter,class OutputIter>
OutputIter move(InputIter first, InputIter last, OutputIter result)
{
	return unchecked_move(first, last, result);
}

//将[first,last)区间内的元素移动到[result-(last-first))中
template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2 unchecked_move_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
	BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
{
	while (first != last)
		*--result = mystl::move(*--last);
	return result;
}

template<class RandomIter1,class RandomIter2>
RandomIter2 unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last,
	RandomIter2 result, mystl::random_access_iterator_tag)
{
	for (auto n = last - first; n > 0; --n)
		*--result = mystl::move(*--last);
	return result;
}

template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2 unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
	BidirectionalIter2 result)
{
	return unchecked_move_backward_cat(first, last, result, iterator_category(first));
}

template<class Tp,class Up>
typename std::enable_if<
	std::is_same<typename std::remove_const<Tp>::type,Up>::value &&
	std::is_trivially_move_assignable<Up>::value,Up*>::type
	unchecked_move_backward(Tp* first, Tp* last, Up* result)
{
	const size_t n = static_cast<size_t>(last - first);
	if (n != 0)
	{
		result -= n;
		std::memmove(result, first, n * sizeof(Up));
	}
	return result;
}

template<class BidirectionalIter1,class BidirectionalIter2>
BidirectionalIter2 move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
	BidirectionalIter2 result)
{
	return unchecked_move_backward(first, last, result);
}

//比较第一序列在[first,last)上和第二序列是否相等
template<class InputIter1,class InputIter2>
bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
	for (; first1 != last1; ++first1, ++first2)
	{
		if (*first1 != *first2)
			return false;
	}
	return true;
}

template<class InputIter1,class InputIter2,class Compared>
bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp)
{
	for (; first1 != last1; ++first1, ++first2)
	{
		if (!comp(*first1, *first2))
			return false;
	}
	return true;
}
//从first开始填充n个值
template<class OutputIter,class Size,class T>
OutputIter unchecked_fill_n(OutputIter first, Size n, const T& value)
{
	for (; n > 0; --n, ++first)
		*first = value;
	return first;
}

/*
 * 为一字节类型提供特化版本
 * 这是由于memset函数是按照字节来填充的，当数据类型是1字节大小时，用memset效率更高
 * 而其他非1字节的类型不能用memset，比如int是4字节，memset(起始地址,值,字节数)，他会把每个字节的数值换为参数中的值
 * 比如类型是int,一个int有四个字节，参数中的值为1，那么调用函数后的每个int数据是这样:00000001 00000001 00000001 00000001
*/
template<class Tp,class Size,class Up>
typename std::enable_if<
	std::is_integral<Tp>::value && sizeof(Tp)==1 &&
	!std::is_same<Tp,bool>::value &&
	std::is_integral<Up>::value && sizeof(Up)==1,
	Tp*>::type unchecked_fill_n(Tp* first, Size n, Up value)
{
	if (n > 0)
		std::memset(first, (unsigned char)value, (size_t)(n));
	return first + n;
}


template<class OutputIter,class Size,class T>
OutputIter fill_n(OutputIter first, Size n, const T& value)
{
	return unchecked_fill_n(first, n, value);
}

//为[first,last)区间内填充新值
template<class ForwardIter, class T>
void fill_cat(ForwardIter first, ForwardIter last, const T& value,mystl::forward_iterator_tag)
{
	for (; first != last; ++first)
	{
		*first = value;
	}
}

template<class RandomIter,class T>
void fill_cat(RandomIter first, RandomIter last, const T& value, mystl::random_access_iterator_tag)
{
	fill_n(first, last - first, value);
}

template<class ForwardIter, class T>
void fill(ForwardIter first, ForwardIter last, const T& value)
{
	fill_cat(first, last, value, iterator_category(first));
}

/*
* 以字典序排列对两个序列进行比较
*	1、第一序列元素较小，返回true；
*   2、到达last1而未到达last2返回true；
*	3、到达last2而未到达last1返回false；
*	4、同时到达last1和last2返回false；
*/
template<class InputIter1,class InputIter2>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
	InputIter2 first2, InputIter2 last2)
{
	for (; first1 != last1 && first2 != last2; ++first1, ++first2)
	{
		if (*first1 < *first2)
			return true;
		if (*first2 < *first1)
			return false;
	}
	return first1 == last1 && first2 != last2;
}

template<class InputIter1,class InputIter2,class Compred>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
	InputIter2 first2, InputIter2 last2,Compred comp)
{
	for (; first1 != last1 && first2 != last2; ++first1, ++first2)
	{
		if (comp(*first1, *first2))
			return true;
		if (comp(*first2, *first1))
			return false;
	}
	return first1 == last1 && first2 != last2;
}

//针对const unsigned char*版本
bool lexicographical_compare(const unsigned char* first1, const unsigned char* last1,
	const unsigned char* first2, const unsigned char* last2)
{
	const auto len1 = last1 - first1;
	const auto len2 = last2 - first2;
	//比较相同的部分
	const auto result = std::memcmp(first1, first2, mystl::min(len1, len2));
	//长度大的比较大
	return result != 0 ? result < 0 : len1 < len2;

}

//平行比较两个序列，找到第一处失配的元素，返回一堆迭代器，指向两个序列中失配的元素
template<class InputIter1,class InputIter2>
mystl::pair<InputIter1, InputIter2>
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
	while (first1 != last1 && *first1 == *first2)
	{
		++first1;
		++first2;
	}
	return mystl::pair<InputIter1, InputIter2>(first1, first2);
}

template<class InputIter1,class InputIter2,class Compred>
mystl::pair<InputIter1,InputIter2>
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compred comp)
{
	while (first1 != last1 && comp(*first1, *first2))
	{
		++first1;
		++first2;
	}
	return mystl::pair<InputIter1, InputIter2>(first1, first2);
}
}