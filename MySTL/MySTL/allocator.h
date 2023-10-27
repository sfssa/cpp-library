#pragma once
#include "construct.h"
#include "utils.h"

/*
* allocator类模板，用于分配和管理内存以及构造和销毁函数
* 在创建新的对象时，先给对象分配内存，然后执行对象的构造函数
* 在销毁对象时，先调用对象的析构函数，然后释放对象所占用的内存
*/

namespace mystl
{

template<class T>
class allocator
{
public:
	typedef T			value_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef size_t		size_type;
	typedef ptrdiff_t	differenct_type;

	//分配一个未初始化的T类型对象内存，返回一个指向分配内存的指针
	static T* allocate();	
	//分配N个未初始化的T类型对象内存，返回指向分配内存的指针
	static T* allocate(size_type n);

	//释放指针指向的内存
	static void deallocate(T* ptr);
	//释放指针指向的n个对象的内存
	static void deallocate(T* ptr, size_type n);

	//在指定位置构造T类型对象
	static void construct(T* ptr);
	//在指定位置构造T类型对象，并用value进行拷贝
	static void construct(T* ptr, const T& value);
	//在指定内存构造T类型对象，使用value进行移动构造
	static void construct(T* ptr, T&& value);

	//以可变数量的参数构造一个T类型的读对象，用于支持不同构造函数签名
	template<class... Args>
	static void construct(T* ptr, Args&& ...args);
	
	//调用对象的析构函数
	static void destroy(T* ptr);
	//调用first-last范围的析构函数
	static void destroy(T* first, T* last);
};

template<class T>
T* allocator<T>::allocate()
{
	return static_cast<T*>(::operator new(sizeof(T)));
}

template<class T>
T* allocator<T>::allocate(size_type n)
{
	if (n == 0)
		return nullptr;
	return static_cast<T*>(::operator new(n * sizeof(T)));
}

template<class T>
void allocator<T>::deallocate(T* ptr)
{
	if (ptr == nullptr)
		return;
	::operator delete(ptr);
}

template<class T>
void allocator<T>::deallocate(T* ptr, size_type /*size*/)
{
	if (ptr == nullptr)
		return;
	::operator delete(ptr);
}

template<class T>
void allocator<T>::construct(T* ptr)
{
	mystl::construct(ptr);
}

template<class T>
void allocator<T>::construct(T* ptr, const T& value)
{
	mystl::construct(ptr, value);
}

template<class T>
void allocator<T>::construct(T* ptr, T&& value)
{
	mystl::construct(ptr, mystl::move(value));
}

template<class T>
template<class ...Args>
void allocator<T>::construct(T* ptr, Args&& ...args)
{
	mystl::construct(ptr, mystl::forward<Args>(args)...);
}

template<class T>
void allocator<T>::destroy(T* ptr)
{
	mystl::destroy(ptr);
}

template<class T>
void allocator<T>::destroy(T* first, T* last)
{
	mystl::destroy(first, last);
}
}