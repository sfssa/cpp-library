#include "data_buf.h"

BufferBase::BufferBase()
    :data_buf(nullptr)
{

}

BufferBase::~BufferBase()
{
    clear();
}

// 获得一个内存块的大小
const int BufferBase::length() const
{
    return data_buf!=nullptr?data_buf->length:0;
}

// 弹出内存块中的数据
void BufferBase::pop(int len)
{
    assert(data_buf!=nullptr&&len<=data_buf->length);
    data_buf->pop(len);
    int tmp=data_buf->length;
    if(tmp==0)
    {
        MemPool::get_instance().retrieve(data_buf);
        data_buf=nullptr;
    }
}

// 清空内存块的内容并插入内存块队列
void BufferBase::clear()
{
    if(data_buf!=nullptr)
    {
        MemPool::get_instance().retrieve(data_buf);
        data_buf=nullptr;
    }
}

// 
int InputBuffer::read_from_fd(int fd)
{
    int need_read;
    if(ioctl(fd,FIONREAD,&need_read)==-1)
    {
        printf("ioctl FIONREAD error\n");
        return -1;
    }

    if(data_buf==nullptr)
    {
        data_buf=MemPool::get_instance().alloc_chunk(need_read);
        if(data_buf==nullptr)
        {
            printf("no free buf for alloc\n");
            return -1;
        }
    }
    else
    {
        assert(data_buf->head==0);
        if(data_buf->capacity-data_buf->length<(int)need_read)
        {
            Chunk* new_buf=MemPool::get_instance().alloc_chunk(need_read+data_buf->length);
            if(new_buf==nullptr)
            {
                printf("no free buf for alloc\n");
                return -1;
            }
            new_buf->copy(data_buf);
            MemPool::get_instance().retrieve(data_buf);
            data_buf=new_buf;
        }
    }
    int alread_read=0;
    do
    {
        if(need_read==0)
            alread_read=read(fd,data_buf->data+data_buf->length,MEM_4K);
        else
            alread_read=read(fd,data_buf->data+data_buf->length,need_read);
    } while (alread_read==-1 && errno==EINTR);
    
    if(alread_read>0)
    {
        if(need_read!=0)
            assert(alread_read==need_read);
        data_buf->length+=alread_read;
    }
    return 0;
}

const char* InputBuffer::get_from_buf() const
{
    return data_buf!=nullptr?data_buf->data+data_buf->head:nullptr;
}

void InputBuffer::adjust()
{
    if(data_buf!=nullptr)
        data_buf->adjust();
}

int OutputBuffer::write_to_buf(const char* data,int len)
{
    if(data_buf==nullptr)
    {
        data_buf=MemPool::get_instance().alloc_chunk(len);
        if(data_buf==nullptr)
        {
            printf("no free buf for alloc\n");
            return -1;
        }
    }
    else
    {
        assert(data_buf->head==0);
        if(data_buf->capacity-data_buf->length<len)
        {
            Chunk* new_buf=MemPool::get_instance().alloc_chunk(len+data_buf->length);
            if(new_buf==nullptr)
            {
                printf("no free buf for alloc\n");
                return -1;
            }
            new_buf->copy(data_buf);
            MemPool::get_instance().retrieve(data_buf);
            data_buf=new_buf;
        }
    }
    memcpy(data_buf->data+data_buf->length,data,len);
    data_buf->length+=len;
    return 0;
}

int OutputBuffer::write_to_fd(int fd)
{
    assert(data_buf!=nullptr && data_buf->head==0);
    int alread_write=0;

    do
    {
        alread_write=write(fd,data_buf->data,data_buf->length);
    } while (alread_write==-1 && errno==EINTR);
    
    if(alread_write>0)
    {
        data_buf->pop(alread_write);
        data_buf->adjust();
    }

    if(alread_write==-1 && errno==EAGAIN)
        alread_write=0;
    return alread_write;
}
