#include "chunk.h"

void testChunk()
{
    // 创建一个 Chunk 对象，大小为 50
    Chunk chunk(50);

    // 打印初始状态
    printf("Capacity: %d\n", chunk.getCapacity());
    printf("Length: %d\n", chunk.getLength());
    printf("Head: %d\n", chunk.getHead());
    printf("Data: %s\n", chunk.getData());

    // 写入一些数据
    const char* testData = "Hello, Chunk!";
    int testDataLen = strlen(testData);
    memcpy(chunk.getData(), testData, testDataLen);
    chunk.setLength(testDataLen);

    // 打印更新后的状态
    printf("Capacity: %d\n", chunk.getCapacity());
    printf("Length: %d\n", chunk.getLength());
    printf("Head: %d\n", chunk.getHead());
    printf("Data: ");
    chunk.printData();

    
    chunk.pop(5);
    // 打印清空后的状态
    printf("Capacity: %d\n", chunk.getCapacity());
    printf("Length: %d\n", chunk.getLength());
    printf("Head: %d\n", chunk.getHead());
    printf("Data: ");
    chunk.printData();

    // 清空并调整 Chunk
    chunk.clear();
    chunk.adjust();
    printf("Capacity: %d\n", chunk.getCapacity());
    printf("Length: %d\n", chunk.getLength());
    printf("Head: %d\n", chunk.getHead());
    printf("Data: ");
    chunk.printData();
}

int main()
{
    testChunk();        // 测试chunk类
    return 0;
}