#pragma once
#include "construct.h"
#include "utils.h"

/*
* allocator��ģ�壬���ڷ���͹����ڴ��Լ���������ٺ���
* �ڴ����µĶ���ʱ���ȸ���������ڴ棬Ȼ��ִ�ж���Ĺ��캯��
* �����ٶ���ʱ���ȵ��ö��������������Ȼ���ͷŶ�����ռ�õ��ڴ�
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

	//����һ��δ��ʼ����T���Ͷ����ڴ棬����һ��ָ������ڴ��ָ��
	static T* allocate();	
	//����N��δ��ʼ����T���Ͷ����ڴ棬����ָ������ڴ��ָ��
	static T* allocate(size_type n);

	//�ͷ�ָ��ָ����ڴ�
	static void deallocate(T* ptr);
	//�ͷ�ָ��ָ���n��������ڴ�
	static void deallocate(T* ptr, size_type n);

	//��ָ��λ�ù���T���Ͷ���
	static void construct(T* ptr);
	//��ָ��λ�ù���T���Ͷ��󣬲���value���п���
	static void construct(T* ptr, const T& value);
	//��ָ���ڴ湹��T���Ͷ���ʹ��value�����ƶ�����
	static void construct(T* ptr, T&& value);

	//�Կɱ������Ĳ�������һ��T���͵Ķ���������֧�ֲ�ͬ���캯��ǩ��
	template<class... Args>
	static void construct(T* ptr, Args&& ...args);
	
	//���ö������������
	static void destroy(T* ptr);
	//����first-last��Χ����������
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