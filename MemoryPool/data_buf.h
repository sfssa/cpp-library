#pragma once

#include <sys/ioctl.h>
#include <unistd.h>
#include "chunk.h"
#include "memory_pool.h"

class BufferBase
{
public:
    BufferBase();
    ~BufferBase();

    const int length() const;
    void pop(int len);
    void clear();
protected:
    Chunk* data_buf;
};

class InputBuffer:public BufferBase
{
public:
    int read_from_fd(int fd);
    const char* get_from_buf() const;
    void adjust();
};

class OutputBuffer:public BufferBase
{
public:
    int write_to_buf(const char* data,int len);
    int write_to_fd(int fd);
};
