#pragma once

#include "algo_base.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "utils.h"

/*
* ��δ��ʼ�����ڴ�ռ����Ԫ�ع��죬�������ơ���䡢�ƶ��Ȳ���
*/
namespace mystl
{

//��[first,last)������ݸ��Ƶ���result��ʼ�Ŀռ䣬����trivial copy assignment��ֱ�Ӱ�λ����������Ҫ���ù��캯��
template<class InputIter,class ForwardIter>
ForwardIter
unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::true_type)
{
	return mystl::copy(first, last, result);
}

template<class InputIter,class ForwardIter>
ForwardIter
unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::false_type)
{
	auto cur = result;
	try
	{
		for (; first != last; ++first, ++cur)
			mystl::construct(&*cur, *first);
	}
	catch (...)
	{
		for (; result != cur; --cur)
			mystl::destroy(&*cur);
	}
	return cur;
}

//ѡ������Ч�ķ�ʽ���и���
template<class InputIter,class ForwardIter>
ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result)
{
	return mystl::unchecked_uninit_copy(first, last, result,
		std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::
		value_type>{});
	//�����{}������һ��std::is_trivially_copy_assignable<>����ʱ����
}

//��[first,first+n)�ϵ����ݸ��Ƶ�result��ʼ�Ŀռ��ϣ����ظ��ƽ�����λ��
template<class InputIter,class Size,class ForwardIter>
ForwardIter
unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::true_type)
{
	return mystl::copy_n(first, n, result).second;
}

template<class InputIter,class Size,class ForwardIter>
ForwardIter
unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::false_type)
{
	auto cur = result;
	try
	{
		for (; n > 0; --n, ++cur, ++first)
			mystl::construct(&*cur, *first);
	}
	catch (...)
	{
		for (; result != cur; --cur)
			mystl::destroy(&*cur);
	}
	return cur;
}

template<class InputIter,class Size,class ForwardIter>
ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result)
{
	return mystl::unchecked_uninit_copy_n(first, n, result,
		std::is_trivially_copy_assignable<
		typename iterator_traits<InputIter>::value_type>{});
}

//��[first,last)�����Ԫ��ֵ
template<class ForwardIter,class T>
void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::true_type)
{
	mystl::fill(first, last, value);
}

template<class ForwardIter,class T>
void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::false_type)
{
	auto cur = result;
	try
	{
		for (; cur != last; ++cur)
			mystl::construct(&*cur, value);
	}
	catch (...)
	{
		for (; first != cur; ++first)
			mystl::destroy(&*first);
	}
}
template<class ForwardIter,class T>
void uninitialized_fill(ForwardIter first, ForwardIter last, const T& value)
{
	mystl::unchecked_uninit_fill(first, last, value,
		std::is_trivially_copy_assignable<
		typename iterator_traits<ForwardIter>::value_type>{});
}

//��first��ʼ���n��Ԫ��
template<class ForwardIter,class Size,class T>
ForwardIter
unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::true_type)
{
	return mystl::fill_n(first, n, value);
}

template<class ForwardIter,class Size,class T>
ForwardIter
unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::false_type)
{
	auto cur = first;
	try
	{
		for (; n > 0; --n, ++cur)
			mystl::construct(&*cur, value);
	}
	catch (...)
	{
		for (; first != cur; ++first)
			mystl::destroy(&*first);
	}
	return cur;
}

template<class ForwardIter,class Size,class T>
ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value)
{
	return mystl::unchecked_uninit_fill_n(first, n, value,
		std::is_trivially_copy_assignable<
		typename iterator_traits<ForwardIter>::value_type>{});
}

//��[first,last)�ϵ������ƶ�����resultΪ��ʼ�Ŀռ�
template<class InputIter, class ForwardIter>
ForwardIter
unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::true_type)
{
	return mystl::move(first, last, result);
}

template<class InputIter,class ForwardIter>
ForwardIter 
unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type)
{
	ForwardIter cur = result;
	try
	{
		for (; first != last; ++first, ++cur)
			mystl::construct(&*cur, mystl::move(*first));
	}
	catch (...)
	{
		mystl::destroy(result, cur);
	}
	return cur;
}

template<class InputIter, class ForwardIter>
ForwardIter
unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type)
{
	return mystl::unchecked_uninit_move(first, last, result,
		std::is_trivially_move_assignable<
		typename iterator_traits<InputIter>::value_type>{});
}

//��[first,first+n)�������ƶ�����resultΪ��ʼ�Ŀռ�
template<class InputIter,class Size,class ForwardIter>
ForwardIter
unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type)
{
	return mystl::move(first, first + n, result);
}

template<class InputIter, class Size, class ForwardIter>
ForwardIter
unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type)
{
	auto cur = result;
	try
	{
		for (; n > 0; --n, ++first, ++cur)
			mystl::construct(&*cur, mystl::move(*first));
	}
	catch (...)
	{
		for (; result != cur; ++result)
			mystl::destroy(&*result);
		throw;
	}
	return cur;
}

template<class InputIter, class Size, class ForwardIter>
ForwardIter
unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result)
{
	return mystl::unchecked_uninit_move_n(first, n, result,
		std::is_trivially_move_assignable<
		typename iterator_traits<InputIter>::value_type>{});
}
}