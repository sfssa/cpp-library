#include "chunk.h"

Chunk::Chunk(size_t size)
    :capacity(size),data(new char[size]),length(0),head(0),next(nullptr)
{
    assert(data);
}   

Chunk::~Chunk()
{
    if(data)
        delete[] data;
    // 处理指向下一个内存块的指针
}

void Chunk::clear()
{
    length=head=0;
}

void Chunk::adjust()
{
    if(head!=0)
    {
        if(length!=0)
            memmove(data,data+head,length);
    }
    head=0;
}

void Chunk::copy(const Chunk* other)
{
    memcpy(data,other->data+other->head,other->length);
    head=0;
    length=other->length;
}

void Chunk::pop(size_t len)
{
    length-=len;
    head+=len;
}

void Chunk::print_data()
{
    int temp=head;
    for(int i=0;i<length;++i)
        printf("%c",data[temp++]);
    printf("\n");
}