#pragma once

#include <stddef.h>

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
struct lept_value;
struct lept_member;

// 用于封装JSON值和类型(目前还没有值)
struct lept_value
{
    union 
    {
        struct array
        {   
            lept_value* e;
            size_t len;
        }m_array;

        struct object
        {
            lept_member* m;
            size_t size;
        }m_object;

        struct str
        {
            char *s;
            size_t len;
        }m_str;

        double n;
    }m_union;

    lept_type type;
};

struct lept_member
{
    char* k;
    size_t klen;
    lept_value v;
};

enum parse_result
{
    // JSON解析成功
    LEPT_PARSE_OK=0,
    // 预期一个JSON值但是输入找不到有效的值
    LEPT_PARSE_EXPECT_VALUE,
    // 无效的JSON值，出现了无效或不支持的值
    LEPT_PARSE_INVALID_VALUE,
    // 解析完一个值后仍然有其他的值
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    // JSON数字的值太大
    LEPT_PARSE_NUMBER_TOO_BIG,
    // 缺少字符串的结束引号
    LEPT_PARSE_MISS_QUOTATION_MARK,
    // 包含了无效的转义序列
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    // 包含了无效字符
    LEPT_PARSE_INVALID_STRING_CHAR,
    // 无效的Unicode编码十六进制数
    LEPT_PARSE_INVALID_UNICODE_HEX,
    // 无效的Unicode编码对
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    // 缺少逗号或者右方括号
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    // 对象中缺少键
    LEPT_PARSE_MISS_KEY,
    // 对象中缺少冒号
    LEPT_PARSE_MISS_COLON,
    // 对象中缺少逗号或者右大括号
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

// 把表达式转为语句，模仿无返回值的函数。
#define lept_init(v) do{(v)->type=LEPT_NULL;} while(0)

char* lept_stringify(const lept_value* v, size_t* length);

// 解析数据
int lept_parse(lept_value* v,const char* json);

// 释放字符串中指针所指向的内存空间
void lept_free(lept_value* v);

// 获得解析的结果
lept_type lept_get_type(const lept_value* v);

// 将字符串的指针指向nullptr
#define lept_set_null(v)    lept_free(v);

// 获得boolean值
int lept_get_boolean(const lept_value* v);

// 设置boolean值
void lept_set_boolean(lept_value* v,int b);

// 获取数字的值
double lept_get_number(const lept_value* v);

// 设置数字的值
void lept_set_number(lept_value* v,double n);

// 获得字符串
const char* lept_get_string(const lept_value* v);

// 获得字符串长度
size_t lept_get_string_length(const lept_value* v);

// 设置字符串的值
void lept_set_string(lept_value* v,const char* s,size_t len);

// 获得数组的大小
size_t lept_get_array_size(const lept_value* v);

// 获得数组的元素
lept_value* lept_get_array_element(const lept_value* v,size_t index);

// 获取兑现的大小
size_t lept_get_object_size(const lept_value* v);

// 获取对象的KEY值
const char* lept_get_object_key(const lept_value* v, size_t index);

// 获取对象的KEY大小
size_t lept_get_object_key_length(const lept_value* v, size_t index);

// 获取对象的值
lept_value* lept_get_object_value(const lept_value* v, size_t index);