#pragma once

#include <string.h>
#include <assert.h>
#include "./utils/utils.h"
#include "./log/log.h"

#define ATPDXY_ASSERT(x) \
    if(!(x)){ \
        ATPDXY_LOG_ERROR(ATPDXY_LOG_ROOT())<<"ASSERTION: " #x \
            <<"\nbacktrace:\n" \
            <<atpdxy::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#define ATPDXY_ASSERT2(x,w) \
    if(!(x)){ \
        ATPDXY_LOG_ERROR(ATPDXY_LOG_ROOT())<<"ASSERTION: "<<#x \
            <<"\n"<<w \
            <<"\nbacktrace:\n" \
            <<atpdxy::BacktraceToString(100,2,"    "); \
        assert(x); \
    }
