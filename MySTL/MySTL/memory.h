#pragma once

#include <cstddef>
#include <cstdlib>
#include <climits>

#include "algo_base.h"
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"

/*
* ����ļ��ṩ��̬�ڴ���������ָ��Ĺ���
*/
namespace mystl
{
//���ض���ĵ�ַ
template<class Tp>
constexpr Tp* address_of(Tp& value) noexcept
{
	return &value;
}

/*
* ���Է��仺����
* ���������������ڴ�Ĵ�С��ģ������ָ��
* ���������ϵͳ�ɷ�������ڴ棬�ͽ�����������Ϊ���ɷ����ڴ�
* ���len>0����ô�Ϳ��ٿռ䣬�ɹ��򷵻ط����ָ��ʹ�С��ʧ�ܽ�len������Է���
* ���ѭ������ʱ��Ȼ�޷������ڴ棬�򷵻ؿ�ָ���0��С�ڴ�
* static_cast<T*>(0) �� C++ �е�����ת�������� static_cast ��ʹ�ã�����Ŀ���ǽ�����
* ���� 0 ת��Ϊ���� T* ��ָ�롣��ͨ�����ڻ�ȡһ��ָ���ض����͵Ŀ�ָ��
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

//��ʱ��������������ͷ�
template<class ForwardIterator,class T>
class temporary_buffer
{
private:
	ptrdiff_t	original_len;	//�����������С
	ptrdiff_t	len;			//������ʵ�ʴ�С
	T*			buffer;			//ָ�򻺳�����ָ��

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

	//��������=���������ʵ�֣�������������Ĭ�ϵ�
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

//aotu_ptrģ����
template<class T>
class auto_ptr
{
public:
	typedef T elem_type;		//���ͱ���

private:
	T* m_ptr;					//��̬�����ڴ��ָ��

public:
	//���캯��������һ��ָ��̬�����ڴ��ָ��
	explicit auto_ptr(T* P = nullptr) :m_ptr(p) {}

	//���ƹ��캯��������һ������ӹ���Դ
	autp_ptr(auto_ptr& rhs) :m_ptr(rhs.release()) {}

	//ģ�幹�죬����Ӳ�ͬ���Ͷ����нӹ���Դ
	template<class U>
	auto_ptr(auto_ptr<U>& rhs) : m_ptr(rhs.release()) {}

	//����һ�������ȡ��Դ
	auto_ptr& operator=(auto_ptr& rhs)
	{
		if (this != &rhs)
		{
			delete m_ptr;				//�ͷŵ�ǰ����ָ��Ŀռ䣬����һ������Ŀռ丳ֵ����ǰ����
			m_ptr = rhs.release();		//�ӹ���Դ
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

	//�������ͷ�ָ����Դ
	~auto_ptr()
	{
		delete m_ptr;
	}

public:
	//����ָ��ָ��Ķ��������
	T& operator*() const
	{
		return *m_ptr;
	}

	//����ָ�뱾��
	T* operator->() const
	{
		return m_ptr;
	}

	//��õ�ǰ���������е�ָ��
	T* get() const
	{
		return m_ptr;
	}

	//�ͷŶ����ָ��Ĺ�������ָ�뱾��
	T* release()
	{
		T* tmp = m_ptr;
		m_ptr = nullptr;
		return tmp;
	}

	//�ͷŵ�ǰ������ڴ棬�ӹ��µ��ڴ�ָ��
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