#pragma once

// JSON的不同类型
enum lept_type
{
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
};

// 用于封装JSON值和类型(目前还没有值)
struct lept_value
{
    double n;
    lept_type type;
};

enum parse_result
{
    LEPT_PARSE_OK=0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

// 解析数据
int lept_parse(lept_value* v,const char* json);

// 获得解析的结果
lept_type lept_get_type(const lept_value* v);

// 获取数组的值
double lept_get_number(const lept_value* v);