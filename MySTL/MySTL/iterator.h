#pragma once

#include <cstddef>
#include "type_traits.h"

namespace mystl
{

/*
* �����������֧�ֶ������ı��������Լ���Ԫ�صĶ�ȡ
* �����������֧�ֶ�Ԫ�صı��������Լ���Ԫ�ص�д��
* ��ǰ����������ǰ��������Ԫ���Լ���Ԫ�صĶ�ȡ
* ˫���������֧����ǰ������������Ԫ���Լ���Ԫ�ض�ȡ
* ������ʵ�������֧��O(1)ʱ�临�Ӷȶ�Ԫ�����λ�÷����Լ���Ԫ�صĶ�ȡ
*/
struct input_iterator_tag{};
struct output_iterator_tag {};
struct forward_iterator_tag:public input_iterator_tag{};
struct bidirectional_iterator_tag:public forward_iterator_tag{};
struct random_access_iterator_tag:public bidirectional_iterator_tag{};

/*
* ptrdiff_t��ͨ������õ�����ָ��֮��Ĳ�ֵ
* 
* Category	������������
* T			��ֵ����
* Pointer	��ָ��
* Reference ������
* Distance	��������֮��ľ���
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
//typename U::iterator_category *= 0����Ƿ������Чʹ��iterator_category��û�лᵼ��SFINAE�����Ǳ������
private:
	struct two
	{
		char a;
		char b;
	};

	//û��iterator_category��ƥ���һ���汾test���о�ƥ��ڶ����汾test���Ӷ��ж��Ƕ���iterator_category
	template<class U> 
	static two test(...);
	
	template<class U>
	static char test(typename U::iterator_category* = 0);

public:
	static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

//��Ի����������ͣ�����Ҫ�������Ϣ
template<class Iterator,bool>
struct iterator_traits_impl{};

//Ϊָ����Iterator�ṩ������Ϣ
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
* is_convertible<from,to>����Ƿ���Դ�from��ʽת����to
* 
* ��һ���汾��Ի�����������
* �ڶ����汾ֻ���ڵ����� Iterator ���� iterator_category ��Ա���Ҹó�Ա
* ��ת��Ϊ input_iterator_tag ��output_iterator_tag ʱ�Żᱻʵ������
* 
* ��δ����Ŀ���Ǽ������� Iterator �Ƿ�Ϊ����������������������
* ����ǣ��ͽ� iterator_traits_helper ������ iterator_traits_impl��
* �Ӷ�Ϊ�õ������ṩ���ʵĵ�����������Ϣ��������������������������
* ����������������Ͳ���Ϊ���ṩ������Ϣ��
*/

template<class Iterator,bool>
struct iterator_traits_helper{};

template <class Iterator>
struct iterator_traits_helper<Iterator,true>
	:public iterator_traits_impl<Iterator,
	std::is_convertible<typename Iterator::iterator_catrgory,input_iterator_tag>::value||
	std::is_convertible<typename Iterator::iterator_category,output_iterator_tag>::value>
{};

//��ȡ����������
template<class Iterator>
struct iterator_traits
	:public iterator_traits_helper<Iterator,has_iterator_category<Iterator>::value>
{};

//��Ի�����������
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
* �������� T ������Ƿ����������� U ƥ�䡣���ƥ�䣬�����̳� m_true_type����ʾ true����
* ����̳� m_false_type����ʾ false��
*/
template<class T,class U>
struct has_iterator_category_of<T, U, false> :public m_false_type{};

//ȷ������������
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

//���������Ƿ�Ϊ�κ�һ���ض����ĵ�����
template <class Iterator>
struct is_iterator :
	public m_bool_constanc<is_input_iterator<Iterator>>::value ||
	is_output_iterator<Iterator>::value >
{};

//��ȡ��������category
template<class Iterator>
typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{
	typedef typename iterator_traits<Iterator>::iterator_category category;
	return category();	//�ο��º���
}

//��ȡdistance_type
template<class Iterator>
typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&)
{
	/*
	* ����ָ��
	* ��ָ��� 0 ת��Ϊ�������͵�ָ�롣�ⲻִ��ʵ�ʵ�ת���������߱���������������ʲô��
	* ���߱�����ֵ������ʲô���������ͨ�����ڷ����㷨�У��Ա��ڱ���ʱȷ����������ֵ���͡�
	*/
	return static_cast<typename iterator_traits<Iterator>::differenct_type*>(0);
}

//��ȡvalue_type
template<class Iterator>
typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

//�������������
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
//ָ������random�������汾
template<class RandomIter>
typename iterator_traits<RandomIter>::difference_type
distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
{
	return last - first;
}

//����֮ǰ�ĺ���
template<class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
	return distance_dispatch(first, last, iterator_category(first));
}

//����������ǰ�ƶ�
template<class InputIterator,class Distance>
void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
{
	while (n--)
		++i;
}

//˫�������ʵ����ǰ�ƶ�����
template<class InputIterator,class Distance>
void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
{
	if (n >= 0)
		while (n--)	++i;
	else
		while (n++)	--i;//����Ϊ�棬ֻ��n=0ʱΪ��
}

//random�������汾	
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

//���������
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
	* iterator_typeΪ����ʹ��reverse_iterator�Ĵ����ܹ����ɻ�ȡ���������ͣ��Ա��������������ƥ�䣬
	* reverse_iterator�������ͨ��reverse_iterator::iterator_type������Iterator����
	* 
	* self���뵱ǰreverse_iterator����ͬ�����ͣ�ʹ���ܹ�����ķ��������õ�ǰ���ʵ������������Ҫ����
	* reverse_iterator����ʱ��self��Ϊ��������
	*/
	typedef Iterator												iterator_type;
	typedef reverse_iterator<Iterator>								self;
public:
	reverse_iterator();
	explicit reverse_iterator(iterator_type i) :current(i) {}
	reverse_iterator(const self& rhs) :current(rhs.current) {}

	//������������
	iterator_type base() const
	{
		return current;
	}

	//����*�������������--�����ʣ���ͼ
	/* __ __ __ __ __ __
	* |__|__|__|__|__|__|��������������ҵ�˳�򣬴�ʱ���������ָ��Ŀռ��Ǵ������ҵ�����
	* 		| 
	* ���Ƿ����������Ҫ����������������Ļ�������Ӧ��ָ��������ҵڶ�����
	* ���������������ģ�ⷴ���������˼�һ
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

	//ǰ��--->����
	self& operator++()
	{
		--current;
		return *this;
	}

	//����++
	self operator(int)
	{
		self tmp = *this;
		--current;
		return tmp;
	}

	//����--->ǰ��
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
	* ����ע�⣬+�ᴥ����������أ������ǰ����������Ѿ�ָ���˴�����������i��Ԫ�أ�
	* ���Ҵ����n>i����ô�ͻ�Խ��
	*/
	reference operator[](difference_type n) const
	{
		return *(*this + n);
	}

	/*
	* reverse_iterator��һ��ģ���࣬�������ڲ�ͬ���͵ĵ�������ÿ����ͬ�����Ͷ�Ӧ��ͬ�ĵ����������
	* ʹ��ģ������Ӧ��ͬ�ĵ���������
	* 
	* ����ĺ�����û��ʹ��ģ�������������Ѿ������˵�������������Ϣ������Ҫʹ�ö����ģ�����
	*/
	//����operator-
	template<class Iterator>
	typename reverse_iterator<Iterator>::difference_type
	operator-(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs)
	{
		return rhs.base() - lhs.base();
	}

	//���رȽϲ�����
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