# 从零开始JSON库（一）：启程
## 什么是JSON？
JSON是一种轻量级数据交换格式，通常用于不同应用程序之间传输数据。JSON数据类型是一种用于表示数据的格式，广泛用于Web开发、API通信和配置文件等领域。
JSON数据类型有以下特点：

1. **基本数据类型**：JSON 支持以下基本数据类型：
   - 字符串（String）：表示文本数据，用双引号括起来。
   - 数值（Number）：表示数字，可以是整数或浮点数。
   - 布尔值（Boolean）：表示 true 或 false。
   - 空值（null）：表示空值或缺失数据。
   - 对象（Object）：表示键值对的集合，用大括号 `{}` 括起来。
   - 数组（Array）：表示有序元素的集合，用方括号 `[]` 括起来。
2. **嵌套结构**：JSON 可以嵌套，即 JSON 对象可以包含其他 JSON 对象或 JSON 数组。这种嵌套结构使其能够表示复杂的数据层次结构。
3. **键值对**：JSON 对象由一组键值对组成，其中键是字符串，值可以是任何有效的 JSON 数据类型。键和值之间用冒号分隔，键值对之间用逗号分隔。
4. **有序性**：JSON 数组是有序的，元素按照它们出现的顺序排列，可以通过索引访问。
5. **易于阅读和写入**：JSON 数据格式是文本格式，易于人类阅读和编写。它通常使用缩进来表示数据层次结构。
6. **广泛支持**：JSON 已经成为一种通用的数据交换格式，得到了许多编程语言的支持。大多数编程语言提供了解析和生成 JSON 数据的库和工具。
7. **无需模式定义**：与某些其他数据存储格式（如XML）不同，JSON 不需要预先定义模式。数据可以动态地包含不同的字段，而不会破坏数据的完整性。

JSON示例：

```
{
  "name": "John Doe",
  "age": 30,
  "isStudent": false,
  "grades": [95, 88, 76],
  "address": {
    "street": "123 Main St",
    "city": "Exampleville"
  },
  "friends": [
    {"name": "Alice", "age": 28},
    {"name": "Bob", "age": 32}
  ]
}
```

## JSON解析的三个步骤

1. 将数据结构转换为JSON文本；
2. 将JSON文本解析成树状数据结构；
3. 提供API接口访问数据；

假设我们有一个包含学生信息的数据结构：

```
student_data = {
    "name": "John Doe",
    "age": 20,
    "courses": ["Math", "Science", "History"],
    "address": {
        "street": "123 Main St",
        "city": "Exampleville"
    }
}
```

经过步骤1后我们可以得到下面的内容：

```
{"name": "John Doe", "age": 20, "courses": ["Math", "Science", "History"], "address": {"street": "123 Main St", "city": "Exampleville"}}
```

经过步骤2后我们可以得到下面内容：

```
姓名: John Doe
年龄: 20
课程: ['Math', 'Science', 'History']
地址: 123 Main St Exampleville
```

同时提供API来访问或修改数据。

## JSON语法子集

下面是此单元的 JSON 语法子集：

```
JSON-text = ws value ws
ws = *(%x20 / %x09 / %x0A / %x0D)
value = null / false / true 
null  = "null"
false = "false"
true  = "true"
```

当中 `%xhh` 表示以 16 进制表示的字符，`/` 是多选一，`*` 是零或多个，`()` 用于分组。

那么第一行的意思是，JSON 文本由 3 部分组成，首先是空白（whitespace），接着是一个值，最后是空白。

第二行告诉我们，所谓空白，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成。

第三行是说，我们现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）。

我们的解析器应能判断输入是否一个合法的 JSON。如果输入的 JSON 不合符这个语法，我们要产生对应的错误码，方便使用者追查问题。

在这个 JSON 语法子集下，我们定义 3 种错误码：

- 若一个 JSON 只含有空白，传回 `LEPT_PARSE_EXPECT_VALUE`。
- 若一个值之后，在空白之后还有其他字符，传回 `LEPT_PARSE_ROOT_NOT_SINGULAR`。
- 若值不是那三种字面值，传回 `LEPT_PARSE_INVALID_VALUE`。

```
enum parse_result
{
    LEPT_PARSE_OK=0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};
```

这里的枚举类型分别代表以下四种情况：

1. `LEPT_PARSE_OK`：
   - 意义：表示 JSON 解析成功，没有发生错误。
   - 例子：当你成功地解析一个有效的 JSON 文本时，可以将解析结果设置为 `LEPT_PARSE_OK`。
2. `LEPT_PARSE_EXPECT_VALUE`：
   - 意义：表示在 JSON 解析过程中，期望找到一个值，但未找到任何有效值。
   - 例子：如果 JSON 文本以不是值的字符开始，比如空格、制表符、换行符等，解析器可能会返回 `LEPT_PARSE_EXPECT_VALUE` 错误。
3. `LEPT_PARSE_INVALID_VALUE`：
   - 意义：表示在 JSON 解析过程中，找到了无效的值，即不符合 JSON 规范的值。
   - 例子：如果 JSON 文本包含无效的数字、非法字符序列或其他不符合 JSON 规范的数据，解析器可能会返回 `LEPT_PARSE_INVALID_VALUE` 错误。
4. `LEPT_PARSE_ROOT_NOT_SINGULAR`：
   - 意义：表示在 JSON 解析过程中，根值后面还有其他字符，即 JSON 文本中根值之后还有额外的内容。
   - 例子：如果 JSON 文本中包含多余的字符，如在根值之后的空格或其他字符，解析器可能会返回 `LEPT_PARSE_ROOT_NOT_SINGULAR` 错误。

## 代码详解

定义JSON不同的数据类型和解析结果：

```
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

enum parse_result
{
    LEPT_PARSE_OK=0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};
```

下面这个结构体用来存放JSON的具体内容：

```
struct lept_context
{
    const char* json;
};
```

下面是获得JSON数据的类型函数，直接访问结构体的type属性即可：

```
lept_type lept_get_type(const lept_value* v)
{
    assert(v!=nullptr);
    return v->type;
}
```

接下来我们从lept_parse函数开始，该函数接收一个JSON类型和一个字符指针，创建lept_context对象并用字符指针初始化对象的json属性。

```
int lept_parse(lept_value* v,const char* json)
{
    lept_context c;
    assert(v!=nullptr);
    c.json=json;
    v->type=LEPT_NULL;
    lept_parse_whilespace(&c);

    int ret=-1;
    if((ret==lept_parse_value(&c,v))==LEPT_PARSE_OK)
    {
        // 解析完一个值之后还有其他的值
        lept_parse_whilespace(&c);
        if(*c.json!='\0')
            ret=LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}
```

先将JSON的类型设置为NULL，然后调用lept_parse_whilespace函数跳过正式文本前的空格：

```
static void lept_parse_whilespace(lept_context* c)
{
    const char*p=c->json;
    while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')
        p++;
    
    c->json=p;
}
```

现在lept_content已经指向了第一个非空字符，然后调用lept_parse_value函数，这个函数用来解析JSON值，根据JSON的第一个字符，调用相应的解析函数：

```
static int lept_parse_value(lept_context*c,lept_value* v)
{
    switch(*c->json)
    {
        case 'n':return lept_parse_null(c,v);
        case 't':return lept_parse_true(c,v);
        case 'f':return lept_parse_false(c,v);
        case '\0':return LEPT_PARSE_EXPECT_VALUE;
        default:return LEPT_PARSE_INVALID_VALUE;
    }
}
```

其中lept_parse_null函数用来判断JSON文本是否是null开头，如果是就更新json的指针并设置JSON数据类型为LEPT_NULL，此时将JSON文本作为NULL来处理，直接返回LEPT_PARSE_OK。

```
static int lept_parse_null(lept_context* c,lept_value* v)
{
    EXPECT(c,'n');
    if(c->json[0]!='u'||c->json[1]!='l'||c->json[2]!='l')
        return LEPT_PARSE_INVALID_VALUE;
    
    c->json+=3;
    v->type=LEPT_NULL;
    return LEPT_PARSE_OK;
}
```

类似上面的函数，完成lept_parse_true和lept_parse_false的代码。

## 流程图

![](https://github.com/sfssa/cpp-library/blob/master/MyJson/static/01.jpg)

## 测试代码

```
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
```

