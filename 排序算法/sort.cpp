#include<iostream>
#include<random>
#include<functional>
#include<chrono>
#include<vector>
#include<algorithm>
#include<string.h>

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

void insertion_sort(std::vector<int>& arr) {
    for (int i = 1; i < arr.size(); i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

std::vector<int> bucket_sort(std::vector<int>& arr,int bucket_size)
{   
    if (arr.size() == 0) {
        return arr;
    }

    int min = arr[0];
    int max = arr[0];
    for (int i = 1; i < arr.size(); i++) {
        if (arr[i] < min) {
            min = arr[i];
        } else if (arr[i] > max) {
            max = arr[i];
        }
    }

    int DEFAULT_BUCKET_SIZE = 5;
    bucket_size = (bucket_size == 0) ? DEFAULT_BUCKET_SIZE : bucket_size;

    int bucket_count = std::floor((max - min) / bucket_size) + 1;
    std::vector<std::vector<int>> buckets(bucket_count);

    for(int i=0;i<arr.size();++i)
    {
        //映射到不同的桶里
        int bucket_index=std::floor((arr[i]-min)/bucket_size);
        buckets[bucket_index].emplace_back(arr[i]);
    }

    arr.clear();

    for(int i=0;i<buckets.size();++i)
    {
        insertion_sort(buckets[i]);
        for(int j=0;j<buckets[i].size();j++)
            arr.emplace_back(buckets[i][j]);
    }

    return arr;
}

//获取数字指定位数的数值
int get_digit(int num,int digit)
{
    int divisor=1;
    for(int i=0;i<digit;++i)
    {
        divisor*=10;
    }
    return num/divisor%10;
}

void radix_sort(std::vector<int>& arr)
{
    int max=arr[0];
    for(const int t:arr)
    {
        if(t>max)
            max=t;
    }

    //最多有多少位
    int num_digits=0;
    while(max>0)
    {
        max/=10;
        num_digits++;
    }

    std::vector<std::vector<int>> buckets(10);

    for(int digit=0;digit<num_digits;++digit)
    {
        //将元素分配到桶中
        for(int num:arr)
        {
            int index=get_digit(num,digit);
            buckets[index].emplace_back(num);
        }

        int pos=0;
        for(auto& bucket:buckets)
        {
            for(int num:bucket)
                arr[pos++]=num;
            bucket.clear();
        }
    }
}

void test_radix_sort()
{
    std::vector<int> arr = {170, 45, 75, 90, 802, 24, 2, 66};
    radix_sort(arr);

    std::cout << "Sorted array: ";
    for (int num : arr) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

void count_sort(int arr[],int size)
{
    for(int i=0;i<size;++i)
        std::cout<<arr[i]<<" ";
    std::cout<<std::endl;

    int max=arr[0],min=arr[0];
    for(int i=1;i<size;++i)
    {
        if(arr[i]>max)
            max=arr[i];
        if(arr[i]<min)
            min=arr[i];
    }
    
    int* tmp=new int[max-min+1];
    memset(tmp,0,(max-min+1)*sizeof(int));
    for(int i=0;i<size;++i)
        tmp[arr[i]-min]++;

    for(int i=1;i<max-min+1;++i)
        tmp[i]+=tmp[i-1];

    int* obj_arr=new int[size];
    for(int i=size-1;i>=0;--i)
        obj_arr[--tmp[arr[i]-min]]=arr[i];
    
    for(int i=0;i<size;++i)
        std::cout<<obj_arr[i]<<" ";

    delete[] obj_arr;
    delete[] tmp;
}

void test_count_sort()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 1000);
    int arr[1000];
    int i = 0;
    int size=1000;
    for (; i < size; ++i)
        arr[i] = dis(gen);

    count_sort(arr,size);
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

void shell_sort(int arr[],int size)
{
    int h=1;
    while(h<size/3)
        h=h*3+1;

    while(h>=1)
    {
        for(int i=h;i<size;++i)
        {
            for(int j=i;j>=h&&arr[j]<arr[j-h];j-=h)
                std::swap(arr[j],arr[j-h]);
        }
        h=h/3;
    }
}

void test_bucket_sort()
{
    std::vector<int> arr = {3, 6, 8, 10, 1, 2, 1};
    std::vector<int> sortedArr = bucket_sort(arr, 3);

    for (int num : sortedArr) {
        std::cout << num << " ";
    }
}

void test_get_digit()
{
    int x=1234;
    int digit=5;
    int ret=get_digit(x,digit);
}

int main() {
    test_radix_sort();
    //test_get_digit();
    //test_bucket_sort();
    //test_count_sort();
    // test_accuracy(shell_sort);
    // test_time(shell_sort);
    return 0;
}
