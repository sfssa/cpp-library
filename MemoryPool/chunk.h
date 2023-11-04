#pragma once

#include <cstddef>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>

class Chunk
{
public:
    // 禁止隐式转换
    explicit Chunk(size_t size);
    
    ~Chunk();

    // 清空内存块
    void clear();

    // 对内存块进行调整
    void adjust();

    // 复制内存块内容
    void copy(const Chunk* other);

    // 从内存块中弹出数据
    void pop(size_t len);

    // 声明这个已经被弃用
    //[[deprecate("chunk debug api deprecated"!)]]
    void print_data();

    Chunk* next;        // 下一个内存块指针
    int capacity;       // 内存块最大可使用空间
    int length;         // 内存块已经使用的空间
    int head;           // 存放数据的数组的偏移量
    char* data;         // 指向实际的数据存储区域
};

