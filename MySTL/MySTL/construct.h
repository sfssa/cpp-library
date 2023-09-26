#pragma once

//这个文件负责构造和析构函数

#include <new>
#include "type_traits.h"
#include "iterator.h"

/*
* _MSC_VER是Microsoft Visual C++的一个预定义宏，包含编译器版本，用来判断是否使用的Microsoft Visual C++编译器
* 
* #pragma warning(push)保存当前编译器警告状态的快照，方便回到修改之前的警告设置
*/
#ifdef _MSC_VER
#pragma warngin(push)
#pragma warning(disable:4100)
#endif 

namespace mystl
{

	//构造函数
	template<class Ty>
	void construct(Ty* ptr)
	{
		//将ptr指针转换成void类型并在其指向的内存块上调用模板参数的构造函数
		::new((void*)ptr) Ty();
	}

	template <class Ty1, class Ty2>
	void construct(Ty1* ptr, const Ty2& value)
	{
		::new ((void*)ptr) Ty1(value);
	}

	//允许传递任意参数
	template<class Ty, class...Args>
	void construct(Ty* ptr, Args&&...args)
	{
		//调用完美转发来保证参数的传递
		::new((void*)ptr) Ty(mystl::forward<Args>(args)...);
	}

	//析构函数

	//表示对象已经销毁，针对具有平凡的析构函数的对象
	template<class Ty>
	void destroy_one(Ty*, std::true_type)
	{

	}

	//表示对象需要检查指针的有效性，对象的析构函数不是平凡的
	template<class T>
	void destroy_one(Ty* pointer, std::false_type)
	{
		if (pointer)
			pointer->~Ty();
	}

	//下面两个函数也是针对迭代器所指对象是否具有平凡的析构函数
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