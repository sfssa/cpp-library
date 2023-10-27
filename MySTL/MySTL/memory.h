#pragma once

#include <cstddef>
#include <cstdlib>
#include <climits>

#include "algo_base.h"
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"

/*
* 这个文件提供动态内存管理和智能指针的功能
*/
namespace mystl
{
//返回对象的地址
template<class Tp>
constexpr Tp* address_of(Tp& value) noexcept
{
	return &value;
}

/*
* 尝试分配缓冲区
* 两个参数是请求内存的大小和模板类型指针
* 如果超过了系统可分配最大内存，就将缓冲区设置为最大可分配内存
* 如果len>0，那么就开辟空间，成功则返回分配的指针和大小；失败将len减半后尝试分配
* 如果循环结束时依然无法分配内存，则返回空指针和0大小内存
* static_cast<T*>(0) 是 C++ 中的类型转换操作符 static_cast 的使用，它的目的是将整数
* 常量 0 转换为类型 T* 的指针。这通常用于获取一个指向特定类型的空指针
*/
template<class T>
pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*)
{
	if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
		len = INT_MAX / sizeof(T);
	while (len > 0)
	{
		T* tmp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
		if (tmp)
			return pair<T*, ptrdiff_t>(tmp, len);
		len /= 2;
	}
	return pair<T*, ptrdiff_t>(nullptr, 0);
}

template<class T>
pair<T*, ptrdiff_t>get_temporary_buffer(ptrdiff_t len)
{
	return get_buffer_helper(len, static_cast<T*>(0));
}

template<class T>
pair<T*, ptrdiff_t>get_temporary_buffer(ptrdiff_t len, T*)
{
	return get_buffer_helper(len, static_cast<T*>(0));
}

template<class T>
void release_temporary_buffer(T* ptr)
{
	free(ptr);
}

//临时缓冲区的申请和释放
template<class ForwardIterator,class T>
class temporary_buffer
{
private:
	ptrdiff_t	original_len;	//缓冲区申请大小
	ptrdiff_t	len;			//缓冲区实际大小
	T*			buffer;			//指向缓冲区的指针

public:
	temporary_buffer(ForwardIterator first, ForwardIterator last);

	~temporary_buffer()
	{
		mystl::destroy(buffer, buffer + len);
		free(buffer);
	}

	ptrdiff_t size() const noexcept
	{
		return len;
	}

	ptrdiff_t requested_size() const noexcept
	{
		return original_len;
	}

	T* begin() noexcept
	{
		return buffer;
	}

	T* end() noexcept
	{
		return buffer + len;
	}

private:
	void allocate_buffer();

	void initialize_buffer(const T&, std::true_type) {}

	void initialize_buffer(const T& value, std::false_type)
	{
		mystl::uninitialized_fill_n(buffer, len, value);
	}

	temporary_buffer(const temporary_buffer&);

	//声明重载=运算符但不实现，编译器会生成默认的
	void operator=(const temporary_buffer&);
};


template<class ForwardIterator, class T>
temporary_buffer<ForwardIterator, T>::temporary_buffer(ForwardIterator first, ForwardIterator last)
{
	try
	{
		len = mystl::distance(first, last);
		allocate_buffer();
		if (len > 0)
			initialize_buffer(*first, std::is_trivially_default_constructible<T>());
	}
	catch (...)
	{
		free(buffer);
		buffer = nullptr;
		len = 0;
	}
}

template<class ForwardIterator,class T>
void temporary_buffer<ForwardIterator, T>::allocate_buffer()
{
	original_len = len;
	if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
		len = INT_MAX / sizeof(T);
	while (len > 0)
	{
		buffer = static_cast<T*>(malloc(len * sizeof(T)));
		if (buffer)
			break;
		len /= 2;
	}
}

//aotu_ptr模板类
template<class T>
class auto_ptr
{
public:
	typedef T elem_type;		//类型别名

private:
	T* m_ptr;					//动态分配内存的指针

public:
	//构造函数，传入一个指向动态分配内存的指针
	explicit auto_ptr(T* P = nullptr) :m_ptr(p) {}

	//复制构造函数，从另一个对象接管资源
	autp_ptr(auto_ptr& rhs) :m_ptr(rhs.release()) {}

	//模板构造，允许从不同类型对象中接管资源
	template<class U>
	auto_ptr(auto_ptr<U>& rhs) : m_ptr(rhs.release()) {}

	//从另一个对象获取资源
	auto_ptr& operator=(auto_ptr& rhs)
	{
		if (this != &rhs)
		{
			delete m_ptr;				//释放当前对象指针的空间，将另一个对象的空间赋值给当前对象
			m_ptr = rhs.release();		//接管资源
		}
		return *this;
	}

	template<class U>
	auto_ptr& operator=(auto_ptr<U>& rhs)
	{
		if (this->get() != rhs.get())
		{
			delete m_ptr;
			m_ptr = rhs.release();
		}
		return *this;
	}

	//析构，释放指针资源
	~auto_ptr()
	{
		delete m_ptr;
	}

public:
	//返回指针指向的对象的引用
	T& operator*() const
	{
		return *m_ptr;
	}

	//返回指针本身
	T* operator->() const
	{
		return m_ptr;
	}

	//获得当前对象所持有的指针
	T* get() const
	{
		return m_ptr;
	}

	//释放对象对指针的管理，返回指针本身
	T* release()
	{
		T* tmp = m_ptr;
		m_ptr = nullptr;
		return tmp;
	}

	//释放当前管理的内存，接管新的内存指针
	void reset(T* p = nullptr)
	{
		if (m_ptr != p)
		{
			delete m_ptr;
			m_ptr = p;
		}
	}
};
}