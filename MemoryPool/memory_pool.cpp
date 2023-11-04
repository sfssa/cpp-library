#include "memory_pool.h"

// 初始化：创建chunk_num个空间大小为size的内存块
void MemPool::mem_init(MemCap size,int chunk_num)
{
    Chunk* prev;
    m_pool_[size]=new (std::nothrow)Chunk(size);
    if(m_pool_[size]==nullptr)
    {
        printf("new chunk %d error",static_cast<int>(size));
        exit(1);
    }
    prev=m_pool_[size];

    for(int i=1;i<chunk_num;++i)
    {
        prev->next=new (std::nothrow)Chunk(size);
        if(prev->next==nullptr)
        {
            printf("new chunk %d error",static_cast<int>(size));

            // 分配失败就将前面申请的内存释放掉
            while(m_pool_[size]!=nullptr)
            {
                Chunk* temp=m_pool_[size];
                m_pool_[size]=temp->next;
                delete temp;
            }

            exit(1);
        }
        prev=prev->next;
    }
    m_total_size_kb_+=size/1024*chunk_num;
}

// 创建不同数目的不同空间的内存块
MemPool::MemPool()
    :m_total_size_kb_(0),m_left_size_kb_(0)
{
    // 不同大小的内存块*参数一）设置不同的数目（参数二）
    mem_init(MEM_4K,4);
    mem_init(MEM_16K,500);
    mem_init(MEM_64K,250);
    mem_init(MEM_256K,100);
    mem_init(MEM_1M,25);
    mem_init(MEM_4M,10);
    m_left_size_kb_=m_total_size_kb_;
}

// 分配内存块，需要n，找到第一个满足添加的内存块
Chunk* MemPool::alloc_chunk(int n)
{
    int index;
    bool found=false;
    // 找到小于n的内存块
    for(index=MEM_LOW;index<=MEM_UP;index=index*MEM_CAP_MULTI_POWER)
    {
        if(n<=index)
        {
            found=true;
            break;
        }
    }
    if(!found)
        return nullptr;

    // 新建内存块
    std::lock_guard<std::mutex> lock(m_mutex_);
    if(m_pool_[index]==nullptr)
    {
        // 超出了内存块最大的数目
        if(m_total_size_kb_+index/1024>=MAX_MEM_POOL_SIZE)
        {
            printf("beyond the limit size of memory!\n");
            exit(1);
        }

        Chunk* new_buf=new (std::nothrow) Chunk(index);
        if(new_buf==nullptr)
        {
            printf("new chunk error\n");
            exit(1);
        }
        m_total_size_kb_+=index/1024;
        return new_buf;
    }

    // 分配已有的内存块
    Chunk* target=m_pool_[index];
    m_pool_[index]=target->next;
    target->next=nullptr;
    m_left_size_kb_-=index/1024;
    return target;
}

// 将使用完的内存块拆入到对应的队列中
void MemPool::retrieve(Chunk* block)
{
    size_t index=block->capacity;
    block->length=0;
    block->head=0;

    std::lock_guard<std::mutex> lock(m_mutex_);
    assert(m_pool_.find(index)!=m_pool_.end());

    block->next=m_pool_[index];
    m_pool_[index]=block;
    m_left_size_kb_+=block->capacity/1024;
}

// 获取空间为x的内存块列表的容量
int MemPool::get_list_size_byte(MemCap index)
{
    int size=0;
    std::lock_guard<std::mutex> lock(m_mutex_);
    // assert(m_pool_.find(index)!=m_pool_.end());
    // 修改为返回0；
    if(m_pool_.find(index)==m_pool_.end())
    {
        printf("no such size block!\n");
        return 0;
    }
    Chunk* node=m_pool_[index];

    while(node)
    {
        size+=node->capacity;
        node=node->next;
    }
    return size;
}

void MemPool::print_list_content(MemCap index)
{
    std::lock_guard<std::mutex> lock(m_mutex_);
    int count=0;
    printf("**********start tot print %d kb chunk_size list data**********\n",index/1024);
    // assert(m_pool_.find(index)!=m_pool_.end());
    if(m_pool_.find(index)==m_pool_.end())
    {
        printf("no such size block!\n");
        return;
    }
    Chunk* node=m_pool_[index];

    while(node)
    {
        if(count<5)
            node->print_data();
        count++;
        node=node->next;
    }

    printf("...\n");
    printf("*************end,node count is %d*************\n\n",count);
    fflush(stdout);
}
