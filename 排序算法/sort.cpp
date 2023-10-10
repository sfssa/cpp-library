#include<iostream>
#include<random>
#include<functional>
#include<chrono>
#include<algorithm>

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

void test_accuracy()
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
    quick_sort(arr,0,1000);
    for(i=0;i<1000;++i)
        std::cout<<arr[i]<<" ";
}

void test_time()
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
    quick_sort(arr,0,size);
    auto end=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout<<"Function execution time: "<<duration.count()<<"ms"<<std::endl;
    delete arr;
}

void test_merge_sort()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 1000);
    int arr[1000];
    int i = 0;

    for (; i < 1000; ++i)
        arr[i] = dis(gen);

    std::cout << "Before sorting: \n";
    for (i = 0; i < 1000; ++i)
        std::cout << arr[i] << " ";
    std::cout << "After sorting: \n";

    int size = 1000;
    int* aux_arr = new int[size];
    merge_sort(arr, aux_arr, 0, size - 1);

    for (i = 0; i < 1000; ++i)
        std::cout << arr[i] << " ";

    delete[] aux_arr; // 释放辅助数组的内存
}

int main() {
    test_accuracy();
    test_time();
    return 0;
}
