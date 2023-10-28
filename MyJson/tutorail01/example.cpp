#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret=0;          // 记录测试是否通过
static int test_count=0;        // 测试的总数目
static int test_pass=0;         // 测试通过的数目

// 检查预期值和实际值是否相等
// equality是预期值和实际值是否想的的表达式，expect是预期值，actual是实际值，format打印错误消息的格式
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

// EXPECT_EQ_BASE的一个特例，传入表达式并调用EXPECT_EQ_BASE
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

// 测试解析null值的功能
static void test_parse_null()
{
    lept_value v;
    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"null"));
    EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
}

// 测试解析true值的功能
static void test_parse_true()
{
    lept_value v;
    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"true"));
    EXPECT_EQ_INT(LEPT_TRUE,lept_get_type(&v));
}

// 测试解析false值的功能
static void test_parse_false()
{
    lept_value v;
    v.type=LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"false"));
    EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));
}

// 测试在预期有值但是没有值的情况下的错误处理
static void test_parse_expect_value()
{
    lept_value v;
    v.type=LEPT_FALSE;

    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE,lept_parse(&v,"false"));
    EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));

    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE,lept_parse(&v,""));
    EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
}

// 测试解析无效值的错误处理
static void test_parse_invalid_value()
{
    lept_value v;
    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"null"));
    EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));

    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"?"));
    EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));
}

// 测试根植后有额外字符的错误处理
static void test_parse_root_not_singular()
{
    lept_value v;
    v.type=LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"null x"));
    EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));
}

static void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n",test_pass,test_count,test_pass*100.0/test_count);
    return main_ret;
}