#pragma once

#include "iterator.h"

//ͷ�ļ�����ѵ��㷨������push_heap,pop_heap,sort_heap,make_heap
namespace mystl
{
/*
* ����в�������
* RandomIter������ָ��ѵĵ�һ��Ԫ��
* holeIndex��ǰ��Ҫ����������λ�ã���ʼ����Ԫ��λ��
* topIndex�ѵĶ���Ԫ������
* value��Ԫ�ص�ֵ
* 
* ���㵱ǰ�ն��ڵ�ĸ��׽ڵ������λ��
* �Ƚϵ�ǰ�ն�λ�ú͸��ڵ��ֵ��С�����׽ڵ����Ƶ�ǰ�ն����ն���������Ϊ��������
* ֱ������ն���ֵ���ٴ��ڸ��׽ڵ��ֵ���߿ն������˶ѵĶ���
* ��valueֵ������Ӧ������
*/
template<class RandomIter,class Distance,class T>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value)
{
	auto parent = (holeIndex - 1) / 2;
	while (holeIndex > topIndex && *(first + parent) < value)
	{
		*(first + holeIndex) = *(first + parent);
		holeIndex = parent;
		parent = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}

//���²����Ԫ������
template<class  RandomIter,class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*)
{
	mystl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
}

//���ⲿ�ṩ�Ľӿڣ�����Ԫ�ز��뵽���У���Ԫ���Ѿ����뵽��β����
template<class  RandomIter>
void push_heap(RandomIter first, RandomIter last)
{
	mystl::push_heap_d(first, last, distance_type(first);
}

template<class RandomIter,class Distance,class T,class Compared>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value, Compared comp)
{
	auto parent = (holeIndex - 1) / 2;
	while (holeIndex > topIndex && comp(*(first + parent), value))
	{
		*(first + holeIndex) = *(first + parent);
		holeIndex = parent;
		parent = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}

template<class RandomIter,class Compared,class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*, Compared comp)
{
	mystl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1), comp);
}

template<class RandomIter,class Compared>
void push_heap(RandomIter first, RandomIter last, Compared comp)
{
	mystl::push_heap_d(first, last, distance_type(first), comp);
}

/*
* pop_heap�����ѵĸ��ڵ�ȡ����
* adjust_heap����˵��
* first������ʵ���������ʾ�ѵ���Ԫ�ص�ַ
* holeIndex��Ҫ�����Ŀն�����λ��
* len��ʾ�ѵ���Ч����
* valueҪ������Ƴ���ֵ
* 
* �������ӽڵ�ֵ��������Ч��Χ�ڣ��������ݽ�value�����ƶ���ֱ���ҵ����ʵ�λ��
* ��ѭ����ȷ��rchildʼ��ָ��ϴ�ֵ
* ���ϴ�ֵ��ֵ���ն��ڵ㣬�ն�ֵ���Ʋ���������
* ����ҵ�Ӧ�ò����ֵ��Ȼ�����ݲ���
*/

template<class RandomIter ,class T,class Distance>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value)
{
	auto topIndex = holeIndex;
	auto rchild = 2 * holeIndex + 2;
	while (rchild < len)
	{
		if (*(first + rchild) < *(first + rchild - 1))
			--rchild;
		*(first + holeIndex) = *(first + rchild);
		holeIndex = rchild;
		rchild = 2 * (rchild + 1);
	}

	//û�����ӽڵ�
	if (rchild == len)
	{
		*(first + holeIndex) = *(first + (rchild - 1));
		holeIndex = rchild - 1;
	}
	mystl::push_heap_aux(first, holeIndex, topIndex, value);
}

//���׽ڵ��β�ڵ㻥��λ�ã�Ȼ�������
template<class RandomIter,class T,class Distance>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*)
{
	*result = *first;
	mystl::adjust_heap(first, static_cast<Distance>(0), last - first, value);
}

template<class RandomIter>
void pop_heap(RandomIter fist, RandomIter last)
{
	mystl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first));
}

//����compare�汾
template <class RandomIter, class T, class Distance, class Compared>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value,
	Compared comp)
{
	// �Ƚ�������(percolate down)����
	auto topIndex = holeIndex;
	auto rchild = 2 * holeIndex + 2;
	while (rchild < len)
	{
		if (comp(*(first + rchild), *(first + rchild - 1)))  --rchild;
		*(first + holeIndex) = *(first + rchild);
		holeIndex = rchild;
		rchild = 2 * (rchild + 1);
	}
	if (rchild == len)
	{
		*(first + holeIndex) = *(first + (rchild - 1));
		holeIndex = rchild - 1;
	}
	// ��ִ��һ������(percolate up)����
	mystl::push_heap_aux(first, holeIndex, topIndex, value, comp);
}

template <class RandomIter, class T, class Distance, class Compared>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result,
	T value, Distance*, Compared comp)
{
	*result = *first;  // �Ƚ�βָ���ó���ֵ����βָΪ������
	mystl::adjust_heap(first, static_cast<Distance>(0), last - first, value, comp);
}

template <class RandomIter, class Compared>
void pop_heap(RandomIter first, RandomIter last, Compared comp)
{
	mystl::pop_heap_aux(first, last - 1, last - 1, *(last - 1),
		distance_type(first), comp);
}

//����ִ��pop������ÿִ��һ�ζ������ֵ�ŵ�����ĩβ�����һ�β��ô���
template<class RandomIter>
void sort_heap(RandomIter first, RandomIter last)
{
	while (last - first > 1)
		mystl::pop_heap(first, last--);
}

template<class RandomIter,class Compared>
void sort_heap(RandomIter first, RandomIter last, Compared comp)
{
	while (last - first > 1)
		mystl::pop_heap(first, last--, comp);
}

//make_heap���������ڵ����ݰ��նѵ�˳������
template<class RandomIter,class Distance>
void make_heap_aux(RandomIter first, RandomIter last, Distance*)
{
	//lastָ�����һ��Ԫ�ص���һ��λ�ã����last-first<2����ֻ��0/1������
	if (last - first < 2)
		return;

	auto len = last - first;
	//������һ����Ҷ�ӽڵ������
	auto holeIndex = (len - 2) / 2;

	//ѭ������ÿһ����Ҷ�ӽڵ���Ϊ���ڵ㣬�����Ըýڵ�Ϊ�����Ӷѣ�ʹ������ѵ�����
	while (true)
	{
		mystl::adjust_heap(first, holeIndex, len, *(first + holeIndex));
		if (holeIndex == 0)
			return;
		//ֱ�������Ľڵ��Ǹ��ڵ��˳�ѭ���������-1����һ����Ҷ�ӽڵ���Ϊ���ڵ�������
		holeIndex--;
	}
}
template <class RandomIter>
void make_heap(RandomIter first, RandomIter last)
{
	mystl::make_heap_aux(first, last, distance_type(first));;
}

// ���ذ汾ʹ�ú������� comp ����Ƚϲ���
template <class RandomIter, class Distance, class Compared>
void make_heap_aux(RandomIter first, RandomIter last, Distance*, Compared comp)
{
	if (last - first < 2)
		return;
	auto len = last - first;
	auto holeIndex = (len - 2) / 2;
	while (true)
	{
		// ������ holeIndex Ϊ�׵�����
		mystl::adjust_heap(first, holeIndex, len, *(first + holeIndex), comp);
		if (holeIndex == 0)
			return;
		holeIndex--;
	}
}

template <class RandomIter, class Compared>
void make_heap(RandomIter first, RandomIter last, Compared comp)
{
	mystl::make_heap_aux(first, last, distance_type(first), comp);
}

}