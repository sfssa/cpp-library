#pragma once

//����ļ����������������

#include <new>
#include "type_traits.h"
#include "iterator.h"

/*
* _MSC_VER��Microsoft Visual C++��һ��Ԥ����꣬�����������汾�������ж��Ƿ�ʹ�õ�Microsoft Visual C++������
* 
* #pragma warning(push)���浱ǰ����������״̬�Ŀ��գ�����ص��޸�֮ǰ�ľ�������
*/
#ifdef _MSC_VER
#pragma warngin(push)
#pragma warning(disable:4100)
#endif 

namespace mystl
{

	//���캯��
	template<class Ty>
	void construct(Ty* ptr)
	{
		//��ptrָ��ת����void���Ͳ�����ָ����ڴ���ϵ���ģ������Ĺ��캯��
		::new((void*)ptr) Ty();
	}

	template <class Ty1, class Ty2>
	void construct(Ty1* ptr, const Ty2& value)
	{
		::new ((void*)ptr) Ty1(value);
	}

	//�������������
	template<class Ty, class...Args>
	void construct(Ty* ptr, Args&&...args)
	{
		//��������ת������֤�����Ĵ���
		::new((void*)ptr) Ty(mystl::forward<Args>(args)...);
	}

	//��������

	//��ʾ�����Ѿ����٣���Ծ���ƽ�������������Ķ���
	template<class Ty>
	void destroy_one(Ty*, std::true_type)
	{

	}

	//��ʾ������Ҫ���ָ�����Ч�ԣ������������������ƽ����
	template<class T>
	void destroy_one(Ty* pointer, std::false_type)
	{
		if (pointer)
			pointer->~Ty();
	}

	//������������Ҳ����Ե�������ָ�����Ƿ����ƽ������������
	template<class ForwardIter>
	void destroy_cat(ForwardIter, ForwardIter, std::true_type)
	{

	}

	template<class ForwardIter>
	void destroy_cat(ForwardIter first, ForwardIter last, std::false_type)
	{
		for (; first != last; ++first)
			destroy(&*first);
	}

	template<class Ty>
	void destroy(Ty* pointer)
	{
		destroy_one(pointer, std::is_trivially_destructible<Ty>{});
	}

	template<class ForwardIter>
	void destroy(ForwardIter first, ForwardIter last)
	{
		destroy_cat(first, last, std::is_trivially_destructible<
			typename iterator_traits<ForwardIter>::value_type>{});
	}

}