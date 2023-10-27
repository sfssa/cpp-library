#pragma once

#include "iterator.h"

//头文件负责堆的算法。包括push_heap,pop_heap,sort_heap,make_heap
namespace mystl
{
/*
* 向堆中插入数据
* RandomIter迭代器指向堆的第一个元素
* holeIndex当前需要调整的索引位置，初始是新元素位置
* topIndex堆的顶部元素索引
* value新元素的值
* 
* 计算当前空洞节点的父亲节点的索引位置
* 比较当前空洞位置和父节点的值，小于则父亲节点下移当前空洞，空洞索引设置为父亲索引
* 直到满足空洞的值不再大于父亲节点的值或者空洞到达了堆的顶部
* 将value值赋给对应的索引
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

//将新插入的元素上移
template<class  RandomIter,class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*)
{
	mystl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
}

//对外部提供的接口，将新元素插入到堆中（新元素已经插入到最尾部）
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
* pop_heap，将堆的根节点取出来
* adjust_heap函数说明
* first随机访问迭代器，表示堆的首元素地址
* holeIndex需要调整的空洞索引位置
* len表示堆的有效长度
* value要插入或移除的值
* 
* 计算右子节点值，若在有效范围内，进行下溯将value向下移动，直到找到合适的位置
* 在循环中确保rchild始终指向较大值
* 将较大值赋值给空洞节点，空洞值下移并更新索引
* 最后找到应该插入的值，然后将数据插入
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

	//没有右子节点
	if (rchild == len)
	{
		*(first + holeIndex) = *(first + (rchild - 1));
		holeIndex = rchild - 1;
	}
	mystl::push_heap_aux(first, holeIndex, topIndex, value);
}

//将首节点和尾节点互换位置，然后调整堆
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

//重载compare版本
template <class RandomIter, class T, class Distance, class Compared>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value,
	Compared comp)
{
	// 先进行下溯(percolate down)过程
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
	// 再执行一次上溯(percolate up)过程
	mystl::push_heap_aux(first, holeIndex, topIndex, value, comp);
}

template <class RandomIter, class T, class Distance, class Compared>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result,
	T value, Distance*, Compared comp)
{
	*result = *first;  // 先将尾指设置成首值，即尾指为欲求结果
	mystl::adjust_heap(first, static_cast<Distance>(0), last - first, value, comp);
}

template <class RandomIter, class Compared>
void pop_heap(RandomIter first, RandomIter last, Compared comp)
{
	mystl::pop_heap_aux(first, last - 1, last - 1, *(last - 1),
		distance_type(first), comp);
}

//不断执行pop操作，每执行一次都将最大值放到容器末尾，最后一次不用处理
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

//make_heap，将容器内的数据按照堆的顺序排列
template<class RandomIter,class Distance>
void make_heap_aux(RandomIter first, RandomIter last, Distance*)
{
	//last指向最后一个元素的下一个位置，因此last-first<2代表只有0/1个数据
	if (last - first < 2)
		return;

	auto len = last - first;
	//获得最后一个非叶子节点的索引
	auto holeIndex = (len - 2) / 2;

	//循环，将每一个非叶子节点作为根节点，调整以该节点为根的子堆，使其满足堆的特性
	while (true)
	{
		mystl::adjust_heap(first, holeIndex, len, *(first + holeIndex));
		if (holeIndex == 0)
			return;
		//直到调整的节点是根节点退出循环，否则就-1将下一个非叶子节点作为根节点来调整
		holeIndex--;
	}
}
template <class RandomIter>
void make_heap(RandomIter first, RandomIter last)
{
	mystl::make_heap_aux(first, last, distance_type(first));;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Distance, class Compared>
void make_heap_aux(RandomIter first, RandomIter last, Distance*, Compared comp)
{
	if (last - first < 2)
		return;
	auto len = last - first;
	auto holeIndex = (len - 2) / 2;
	while (true)
	{
		// 重排以 holeIndex 为首的子树
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