#include "chunk.h"
#include <iostream>
#include "memory_pool.h"

void testChunk()
{
    // 创建一个 Chunk 对象，大小为 50
    Chunk chunk(50);

    // 打印初始状态
    printf("Capacity: %d\n", chunk.capacity);
    printf("Length: %d\n", chunk.length);
    printf("Head: %d\n", chunk.head);
    printf("Data: %s\n", chunk.data);

    // 写入一些数据
    const char* testData = "Hello, Chunk!";
    int testDataLen = strlen(testData);
    memcpy(chunk.data, testData, testDataLen);
    chunk.length=testDataLen;

    // 打印更新后的状态
    printf("Capacity: %d\n", chunk.capacity);
    printf("Length: %d\n", chunk.length);
    printf("Head: %d\n", chunk.head);
    printf("Data: ");
    chunk.print_data();

    
    chunk.pop(5);
    // 打印清空后的状态
    printf("Capacity: %d\n", chunk.capacity);
    printf("Length: %d\n", chunk.length);
    printf("Head: %d\n", chunk.head);
    printf("Data: ");
    chunk.print_data();

    // 清空并调整 Chunk
    chunk.clear();
    chunk.adjust();
    printf("Capacity: %d\n", chunk.capacity);
    printf("Length: %d\n", chunk.length);
    printf("Head: %d\n", chunk.head);
    printf("Data: ");
    chunk.print_data();
}

void test_memory_pool()
{
    MemPool& memPool = MemPool::get_instance();

    // 分配内存块
    Chunk* chunk1 = memPool.alloc_chunk(4096);
    Chunk* chunk2 = memPool.alloc_chunk(16384);
    
    const char* my_string="Hello World!";
    int str_length=strlen(my_string);
    if(str_length<=chunk1->capacity)
    {
        memcpy(chunk1->data,my_string,str_length);
        chunk1->length=str_length;
        chunk1->data[str_length] = '\0';
    }
    else
        std::cerr << "String too long for the allocated chunk!" << std::endl;

    chunk1->print_data();
    memPool.print_list_content(MEM_4K);
    if (chunk1 && chunk2) {
        printf("Allocated memory chunks successfully.\n");

        // 打印内存池中不同大小内存块的容量
        printf("Size of 4K chunks in the memory pool: %d bytes\n", memPool.get_list_size_byte(MEM_4K));
        printf("Size of 16K chunks in the memory pool: %d bytes\n", memPool.get_list_size_byte(MEM_16K));

        // 将内存块返回到内存池
        memPool.retrieve(chunk1);
        memPool.retrieve(chunk2);

        printf("Memory chunks have been retrieved.\n");
    } else {
        printf("Failed to allocate memory chunks.\n");
    }
}

int main()
{
    testChunk();        // 测试chunk类
    test_memory_pool();
    return 0;
}
