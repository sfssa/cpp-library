#pragma once

#include <unordered_map>
#include <mutex>
#include <assert.h>

#include "chunk.h"

typedef std::unordered_map<int,Chunk*>pool_t;

// 内存块大小倍数
const size_t MEM_CAP_MULTI_POWER=4;

// 内存块最小倍数
const size_t MIN_MEM_POOL_SIZE=4*1024;

// 内存块最大倍数
const size_t MAX_MEM_POOL_SIZE=4*1024*1024;

enum MemCap
{
    MEM_LOW=4096,
    MEM_4K=MEM_LOW,
    MEM_16K=MEM_4K*MEM_CAP_MULTI_POWER,
    MEM_64K=MEM_16K*MEM_CAP_MULTI_POWER,
    MEM_256K=MEM_64K*MEM_CAP_MULTI_POWER,
    MEM_1M=MEM_256K*MEM_CAP_MULTI_POWER,
    MEM_4M=MEM_1M*MEM_CAP_MULTI_POWER,
    MEM_UP=MEM_4M
};

class MemPool
{
public:
    // 单例模式获得内存池
    static MemPool& get_instance()
    {
        static MemPool instance;
        return instance;
    }

    // 开辟指定大小的内存块
    Chunk* alloc_chunk(int n);

    // 默认内存块大小4k
    Chunk* alloc_chunk()
    {
        return alloc_chunk(MEM_4K);
    }

    // 
    void retrieve(Chunk* block);

    // 
    ~MemPool()=default;

    size_t get_total_size_kb()
    {
        return this->m_total_size_kb_;
    }

    int get_left_size_kn()
    {
        return this->m_left_size_kb_;
    }

    int get_list_size_byte(MemCap index);

    void print_list_content(MemCap index);

private:
    MemPool();
    MemPool(const MemPool&)=delete;
    MemPool(MemPool&&)=delete;
    MemPool& operator =(const MemPool)=delete;
    MemPool& operator=(MemPool&&)=delete;

    void mem_init(MemCap size,int chunk_num);

    pool_t m_pool_;                 // 内存池的map
    uint64_t m_total_size_kb_;      // 内存池的总空间大小
    uint64_t m_left_size_kb_;       // 内存池的空闲空间大小
    std::mutex m_mutex_;            // 互斥量
};
