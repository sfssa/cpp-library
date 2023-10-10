# 复杂度

| 算法     | 平均时间复杂度 | 最差时间复杂度 | 空间复杂度 | 数据对象稳定性     |
| -------- | -------------- | -------------- | ---------- | ------------------ |
| 冒泡排序 | O(n2)          | O(n2)          | O(1)       | 稳定               |
| 选择排序 | O(n2)          | O(n2)          | O(1)       | 数组稳定，链表稳定 |
| 插入排序 | O(n2)          | O(n2)          | O(1)       | 稳定               |
| 快速排序 | O(n*lon2n)     | O(n2)          | O(1)       | 不稳定             |
| 堆排序   |                |                |            |                    |
| 归并排序 |                |                |            |                    |
| 希尔排序 |                |                |            |                    |
| 计数排序 |                |                |            |                    |
| 桶排序   |                |                |            |                    |
|          |                |                |            |                    |

在排序算法中，"稳定"是指当存在两个相等的元素A和B，它们在排序前的相对顺序和排序后的相对顺序仍然保持不变。换句话说，如果在排序前A在B之前，那么在排序后A仍然会在B之前

## 测试函数及所需头文件

````c++
#include<iostream>
#include<random>
#include<functional>
#include<chrono>

void test_accuracy(std::function<void(int[],int)>sort_function)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1,1000);
    int arr[1000];
    int i=0;

    for(;i<1000;++i)
        arr[i]=dis(gen);

    std::cout<<"Before sorting: \n";
    for(i=0;i<1000;++i)
        std::cout<<arr[i]<<" ";
    std::cout<<"After sorting: \n";
    sort_function(arr,1000);
    for(i=0;i<1000;++i)
        std::cout<<arr[i]<<" ";
}

void test_time(std::function<void(int[],int)>sort_function)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1,100000);
    
    int* arr;
    const int size=100000;
    arr=new int[size];
    
    for(int i=0;i<size;++i)
        arr[i]=dis(gen);

    auto start=std::chrono::high_resolution_clock::now();
    sort_function(arr,size);
    auto end=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout<<"Function execution time: "<<duration.count()<<"ms"<<std::endl;
    delete arr;
}
````



# 一、冒泡排序

## 1、算法步骤

- 从第一个元素开始遍历数组元素，如果第一个元素大于第二个元素就交换两个元素，从第一对到最后一对，这一步最后可以得到元素的最大值、次大值...；
- 针对所有元素不断重复上述步骤，比较到上一次得到的最大值即可；
- 需要比较的元素越来越少，直到没有任何一对数字需要比较结束。

## 2、代码

```c++
void bubble_sort(int arr[],int size)
{
    for(int i=0;i<size;++i)
    {
        for(int j=0;j<size-i-1;++j)
        {
            if(arr[j]>arr[j+1])
            {
                //利用异或避免临时变量
                arr[j]^=arr[j+1];
                arr[j+1]^=arr[j];
                arr[j]^=arr[j+1];
            }
        }
    }
}
```

# 二、选择排序

## 1、算法步骤

- 在未排序的序列中找到最小的元素放到起始位置；
- 从剩余的未排序序列中继续寻找最小的元素，放到已排序序列的末尾；
- 以此类推，直到所有元素全部排序完成。

## 2、代码

```c++
void select_sort(int arr[],int size)
{
    int min_idx;
    for(int i=0;i<size-1;++i)
    {
        min_idx=i;
        for(int j=i+1;j<size;++j)
        {
            if(arr[min_idx]>arr[j])
                min_idx=j;
        }

        if(min_idx!=i)
        {
            arr[i]^=arr[min_idx];
            arr[min_idx]^=arr[i];
            arr[i]^=arr[min_idx];
        }
    }
}
```

# 三、插入排序

##  1、算法步骤

- 将整个序列分为已排序和未排序，第一个元素默认是已排序的第一个元素；
- 将未排序的数据提取出来，倒序访问已排序的部分，找到未排序数据该插入的位置并插入；
- 不断重复上一步，直到将所有数据都排序。

## 2、代码

````c++
void insert_sort(int arr[],int size)
{
    for(int i=1;i<size;++i)
    {
        int index=i;
        int tmp=arr[i];

        while(index>0&&tmp<arr[index-1])
        {
            arr[index]=arr[index-1];
            index--;
        }
        arr[index]=tmp;
    }
}
````

# 四、快速排序

## 1、算法步骤

- 将第一个元素作为基准，将小于它的元素放到它左边；大于它的元素放到右边；
- 对左右子区间重复上一步，直到各个区间只有一个元素

## 2、代码

```c++
void quick_sort(int arr[],int left,int right)
{
    if(left>=right)
        return;
    
    int low=left;
    int high=right;
    int value=arr[low];

    while(low<high)
    {
        while(low<high&&arr[high]>value)
            high--;
        
        if(low<high)
            arr[low++]=arr[high];

        while(low<high&&arr[low]<value)
            low++;
        
        if(low<high)
            arr[high--]=arr[low];
    }
    arr[low]=value;

    quick_sort(arr,left,low-1);
    quick_sort(arr,high+1,right);
}
```

# 五、堆排序

## 1、算法步骤

- 自底向上构建大根堆；
- 将堆顶元素放到最后一个节点，将区间末尾-1，重新构建指定区间的大根堆；
- 不断重复，直到最后一个元素。

##  2、代码

````c++
//父节点：i，则左子节点为：2*i+1，右子节点为：2*i+2；最后一个父节点：size/2-1
void max_heapify(int arr[],int start,int end)
{
    int father=start;   //父节点
    int son=start*2+1;  //左子节点

    while(son<=end)
    {
        if(son+1<=end&&arr[son+1]>arr[son])
            son++;
        if(arr[father]>arr[son])
            return;
        else
        {
            std::swap(arr[father],arr[son]);
            father=son;
            son=father*2+1;
        }
    }
}

void heap_sort(int arr[],int size)
{
    //从最后一个父节点向上整理
    for(int i=size/2-1;i>=0;--i)
        max_heapify(arr,i,size-1);

    //找到最大的值放到最后一位，并缩减查找区间
    for(int i=size-1;i>0;--i)
    {
        std::swap(arr[0],arr[i]);
        max_heapify(arr,0,i-1);
    }
}
````

# 六、归并排序

## 1、算法步骤

- 整体原则是先将一个数组拆分成多个小数组；然后将这些小数组不断地有序的合成完整的数组；
- 先将整个数组根据中间值拆分成两个数组，不断重复这个过程，直到只有一个元素；
- 借助辅助数组，由于是将父数组拆分成两个子数组，因此在归并时判断两个子数组的大小顺序，合并成一个有序的数组，不断地重复，直到将所有元素都有序的存储到辅助数组中；
- 将辅助数组中的元素复制到原数组。

## 2、代码

````c++
//注意传递的是边界还是下标，这会决定在条件判断是否需要=
void merge(int arr[],int auxiliary_arr[],int left,int mid,int right)
{
    int idx_l=left;     //左半区第一个
    int idx_r=mid+1;    //右半区第一个
    int idx=left;       //辅助数组第一个

    while(idx_l<=mid&&idx_r<=right)
    {
        if(arr[idx_l]<arr[idx_r])
            auxiliary_arr[idx++]=arr[idx_l++];
        else    
            auxiliary_arr[idx++]=arr[idx_r++];
    }

    while(idx_l<=mid)
        auxiliary_arr[idx++]=arr[idx_l++];

    while(idx_r<=right)
        auxiliary_arr[idx++]=arr[idx_r++];

    while(left<=right)
    {
        arr[left]=auxiliary_arr[left];
        left++;
    }
}

void merge_sort(int arr[],int auxiliary_arr[],int left,int right)
{
    if (left < right) {
        int mid = left + (right - left) / 2;  // 避免整数溢出
        merge_sort(arr, auxiliary_arr, left, mid);
        merge_sort(arr, auxiliary_arr, mid + 1, right);
        merge(arr, auxiliary_arr, left, mid, right);
    }
}
````

