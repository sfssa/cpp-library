#pragma once

#include <cstddef>
#include "type_traits.h"

namespace mystl
{

/*
* 输入迭代器：支持对容器的遍历访问以及对元素的读取
* 输出迭代器：支持对元素的遍历访问以及对元素的写入
* 向前迭代器：向前遍历访问元素以及对元素的读取
* 双向迭代器：支持向前或向后遍历访问元素以及对元素读取
* 随机访问迭代器：支持O(1)时间复杂度对元素随机位置访问以及对元素的读取
*/
struct input_iterator_tag{};
struct output_iterator_tag {};
struct forward_iterator_tag:public input_iterator_tag{};
struct bidirectional_iterator_tag:public forward_iterator_tag{};
struct random_access_iterator_tag:public bidirectional_iterator_tag{};

/*
* ptrdiff_t：通过相减得到两个指针之间的差值
* 
* Category	：迭代器类型
* T			：值类型
* Pointer	：指针
* Reference ：引用
* Distance	：迭代器之间的距离
*/
template<class Category,class T,class Distance=ptrdiff_t,class Pointer=T*,class Reference=T&>
struct iterator
{
	typedef Category		iterator_category;
	typedef T				value_type;
	typedef Pointer			pointer;
	typedef Reference		reference;
	typedef Distance		difference_type;
};

template<class T>
struct has_iterator_category
{
//typename U::iterator_category *= 0检查是否可以有效使用iterator_category，没有会导致SFINAE而不是编译错误
private:
	struct two
	{
		char a;
		char b;
	};

	//没有iterator_category就匹配第一个版本test，有就匹配第二个版本test，从而判断是都有iterator_category
	template<class U> 
	static two test(...);
	
	template<class U>
	static char test(typename U::iterator_category* = 0);

public:
	static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

//针对基本数据类型，不需要额外的信息
template<class Iterator,bool>
struct iterator_traits_impl{};

//为指定的Iterator提供特性信息
template<class Iterator>
struct iterator_traits_impl<Iterator, true>
{
	typedef typename Iterator::iterator_category	iterator_category;
	typedef typename Iterator::value_type			value_type;
	typedef typename Iterator::pointer				pointer;
	typedef typename Iterator::reference			reference;
	typedef typename Iterator::difference_type		difference_type;
};

/*
* is_convertible<from,to>检查是否可以从from隐式转换到to
* 
* 第一个版本针对基本数据类型
* 第二个版本只有在迭代器 Iterator 具有 iterator_category 成员并且该成员
* 可转换为 input_iterator_tag 或output_iterator_tag 时才会被实例化。
* 
* 这段代码的目的是检查迭代器 Iterator 是否为输入迭代器或输出迭代器，
* 如果是，就将 iterator_traits_helper 派生自 iterator_traits_impl，
* 从而为该迭代器提供合适的迭代器特性信息。否则，如果迭代器不是输入迭代
* 器或输出迭代器，就不会为其提供特性信息。
*/

template<class Iterator,bool>
struct iterator_traits_helper{};

template <class Iterator>
struct iterator_traits_helper<Iterator,true>
	:public iterator_traits_impl<Iterator,
	std::is_convertible<typename Iterator::iterator_catrgory,input_iterator_tag>::value||
	std::is_convertible<typename Iterator::iterator_category,output_iterator_tag>::value>
{};

//萃取迭代器特性
template<class Iterator>
struct iterator_traits
	:public iterator_traits_helper<Iterator,has_iterator_category<Iterator>::value>
{};

//针对基本数据类型
template<class T>
struct iterator_traits<T*>
{
	typedef		random_access_iterator_tag   iterator_category;
	typedef		T							 value_type;
	typedef		T*							 pointer;
	typedef		T&							 reference;
	typedef		ptrdiff_t					 difference_type;
};

template<class T>
struct iterator_traits<const T*>
{
	typedef		random_access_iterator_tag   iterator_category;
	typedef		T							 value_type;
	typedef		T* pointer;
	typedef		T& reference;
	typedef		ptrdiff_t					 difference_type;
};

template<class T,class U,bool=has_iterator_category<iterator_traits<T>>::value>
struct has_iterator_category_of
	:public m_bool_constant<std::is_convertible<
	typename iterator_traits<T>::iterator_category,U>::value>
{};

/*
* 检查迭代器 T 的类别是否与给定的类别 U 匹配。如果匹配，它将继承 m_true_type（表示 true），
* 否则继承 m_false_type（表示 false）
*/
template<class T,class U>
struct has_iterator_category_of<T, U, false> :public m_false_type{};

//确定迭代器类型
template<class Iter>
struct is_input_iterator:public has_iterator_category_of<Iter,input_iterator_tag>{};

template <class Iter>
struct is_output_iterator:public has_iterator_category_of<Iter,output_iterator_tag>{};

template<class Iter>
struct is_forward_iterator:public has_iterator_category_of<Iter,forward_iterator_tag>{};

template<class Iter>
struct is_bidirectional_iterator:public has_iterator_category_of<Iter,bidirectional_iterator_tag>{};

template<class Iter>
struct is_random_iterator:public has_iterator_category_of<Iter,random_access_iterator_tag>{};

//检查迭代器是否为任何一种特定类别的迭代器
template <class Iterator>
struct is_iterator :
	public m_bool_constanc<is_input_iterator<Iterator>>::value ||
	is_output_iterator<Iterator>::value >
{};

//萃取迭代器的category
template<class Iterator>
typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{
	typedef typename iterator_traits<Iterator>::iterator_category category;
	return category();	//参考仿函数
}

//萃取distance_type
template<class Iterator>
typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&)
{
	/*
	* 返回指针
	* 将指针从 0 转换为差异类型的指针。这不执行实际的转换，但告诉编译器差异类型是什么。
	* 告诉编译器值类型是什么。这个函数通常用于泛型算法中，以便在编译时确定迭代器的值类型。
	*/
	return static_cast<typename iterator_traits<Iterator>::differenct_type*>(0);
}

//萃取value_type
template<class Iterator>
typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

//计算迭代器距离
template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag)
{
	typename iterator_traits<InputIterator>::difference_type n = 0;
	while (first != last)
	{
		++first;
		++n;
	}
	return n;
}
//指针距离的random迭代器版本
template<class RandomIter>
typename iterator_traits<RandomIter>::difference_type
distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
{
	return last - first;
}

//调用之前的函数
template<class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
	return distance_dispatch(first, last, iterator_category(first));
}

//将迭代器向前移动
template<class InputIterator,class Distance>
void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
{
	while (n--)
		++i;
}

//双向迭代器实现向前移动操作
template<class InputIterator,class Distance>
void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
{
	if (n >= 0)
		while (n--)	++i;
	else
		while (n++)	--i;//负数为真，只有n=0时为假
}

//random迭代器版本	
template<class RandomIterator,class Distance>
void advance_dispatch(Random& i, Distance n, random_access_iterator_tag)
{
	i += n;
}

template<class InputIterator,class Distance>
void advance(InputIterator& i, Distance n)
{
	advance_dispatch(i, n, iterator_category());
}

//反向迭代器
template<class Iterator>
class reverse_iterator
{
private:
	Iterator current;
public:
	typedef typename iterator_traits<Iterator>::iterator_category   iterator_category;
	typedef typename iterator_traits<Iterator>::value_type			value_type;
	typedef typename iterator_traits<Iterator>::difference_type		difference_type;
	typedef typename iterator_traits<Iterator>::pointer				pointer;
	typedef typename iterator_traits<Iterator>::reference			reference;
	/*
	* iterator_type为了让使用reverse_iterator的代码能够轻松获取迭代器类型，以便进行声明和类型匹配，
	* reverse_iterator对象可以通过reverse_iterator::iterator_type来访问Iterator类型
	* 
	* self是与当前reverse_iterator类相同的类型，使得能够在类的方法中引用当前类的实例，或者在需要返回
	* reverse_iterator对象时将self作为返回类型
	*/
	typedef Iterator												iterator_type;
	typedef reverse_iterator<Iterator>								self;
public:
	reverse_iterator();
	explicit reverse_iterator(iterator_type i) :current(i) {}
	reverse_iterator(const self& rhs) :current(rhs.current) {}

	//获得正向迭代器
	iterator_type base() const
	{
		return current;
	}

	//重载*操作符，如果对--有疑问，画图
	/* __ __ __ __ __ __
	* |__|__|__|__|__|__|按照正向从左往右的顺序，此时正向迭代器指向的空间是从左往右第三个
	* 		| 
	* 但是反向迭代器需要反过来，从右向左的话迭代器应该指向从左往右第二个，
	* 这里用正向迭代器模拟反向操作，因此减一
	*/
	reference operator*() const
	{
		auto tmp = current;
		return *(--tmp);
	}

	pointer operator->() const
	{
		return &(operator*());
	}

	//前进--->后退
	self& operator++()
	{
		--current;
		return *this;
	}

	//后置++
	self operator(int)
	{
		self tmp = *this;
		--current;
		return tmp;
	}

	//后退--->前进
	self& operator--()
	{
		++current;
		return *this;
	}

	self operator--(int)
	{
		self tmp = *this;
		++current;
		return tmp;
	}

	self& operator+=(difference_type n)
	{
		current -= n;
		return *this;
	}

	self operator+(difference_type n) const
	{
		return self(current - n);
	}

	self& operator-=(difference_type n)
	{
		current += n;
		return *this;
	}

	self operator-(difference_type n) const
	{
		return self(current + n);
	}

	/*
	* 这里注意，+会触发上面的重载，如果当前反向迭代器已经指向了从左向右数第i个元素，
	* 并且传入的n>i，那么就会越界
	*/
	reference operator[](difference_type n) const
	{
		return *(*this + n);
	}

	/*
	* reverse_iterator是一个模板类，可以用于不同类型的迭代器，每个不同的类型对应不同的迭代器，因此
	* 使用模板来适应不同的迭代器类型
	* 
	* 上面的函数中没有使用模板是由于类中已经定义了迭代器的类型信息，不需要使用额外的模板参数
	*/
	//重载operator-
	template<class Iterator>
	typename reverse_iterator<Iterator>::difference_type
	operator-(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return rhs.base() - lhs.base();
	}

	//重载比较操作符
	template<class Iterator>
	bool operator==(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return lhs.base() == rhs.base();
	}

	template<class Iterator>
	bool operator<(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return rhs.base() < lhs.base();
	}

	template<class Iterator>
	bool operator!=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return !(lhs == rhs);
	}

	template<class Iterator>
	bool operator>(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return rhs < lhs;
	}

	template<class Iterator>
	bool operator<=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return !(rhs < lhs);
	}

	template<class Iterator>
	bool operator>=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return !(lhs < rhs);
	}
};
}