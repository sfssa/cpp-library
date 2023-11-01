# 从零开始的JSON库教程（六）：解析对象

## JSON对象

JSON 对象和 JSON 数组非常相似，区别包括 JSON 对象以花括号 `{}`（`U+007B`、`U+007D`）包裹表示，另外 JSON 对象由对象成员（member）组成，而 JSON 数组由 JSON 值组成。所谓对象成员，就是键值对，键必须为 JSON 字符串，然后值是任何 JSON 值，中间以冒号 `:`（`U+003A`）分隔。完整语法如下：

```
member = string ws %x3A ws value
object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D
```

1. `member` 表示 JSON 对象的成员，也就是对象中的一个键值对。它的构成是：一个字符串（`string`）后跟一个冒号 `:` 后再跟一个 JSON 值（`value`）。
2. `object` 表示 JSON 对象的完整表示，它以花括号 `{}` 包裹起来，表示对象的开始和结束。对象可以包含零个或多个成员。
3. `ws` 表示空格字符，允许在语法中添加空格以增加可读性。
4. `[ member *( ws %x2C ws member ) ]` 表示 JSON 对象中成员的结构。它是一个可选的部分，可以包含一个或多个 `member`，每两个 `member` 之间用逗号 `,` 分隔。

具体示例如下，这是一个包含两个成员的 JSON 对象：

```
{
    "name": "John",
    "age": 30
}
```

在这个示例中，JSON 对象包含两个成员。第一个成员的键是 `"name"`，对应的值是字符串 `"John"`。第二个成员的键是 `"age"`，对应的值是数字 `30`。这遵循 JSON 对象的语法规则，其中成员是键值对，键是字符串，值可以是任何 JSON 值。成员之间用逗号 `,` 分隔，整个对象用花括号 `{}` 包裹。

## 数据结构

要表示键值对的集合，有很多数据结构可供选择，例如：

- 动态数组（dynamic array）：可扩展容量的数组，如 C++ 的 [`std::vector`](https://en.cppreference.com/w/cpp/container/vector)。
- 有序动态数组（sorted dynamic array）：和动态数组相同，但保证元素已排序，可用二分搜寻查询成员。
- 平衡树（balanced tree）：平衡二叉树可有序地遍历成员，如红黑树和 C++ 的 [`std::map`](https://en.cppreference.com/w/cpp/container/map)（[`std::multi_map`](https://en.cppreference.com/w/cpp/container/multimap) 支持重复键）。
- 哈希表（hash table）：通过哈希函数能实现平均 O(1) 查询，如 C++11 的 [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map)（[`unordered_multimap`](https://en.cppreference.com/w/cpp/container/unordered_multimap) 支持重复键）。

设一个对象有 n 个成员，数据结构的容量是 m，n ⩽ m，那么一些常用操作的时间／空间复杂度如下：

|                 | 动态数组  | 有序动态数组 | 平衡树     | 哈希表                 |
| --------------- | --------- | ------------ | ---------- | ---------------------- |
| 有序            | 否        | 是           | 是         | 否                     |
| 自定成员次序    | 可        | 否           | 否         | 否                     |
| 初始化 n 个成员 | O(n)      | O(n log n)   | O(n log n) | 平均 O(n)、最坏 O(n^2) |
| 加入成员        | 分摊 O(1) | O(n)         | O(log n)   | 平均 O(1)、最坏 O(n)   |
| 移除成员        | O(n)      | O(n)         | O(log n)   | 平均 O(1)、最坏 O(n)   |
| 查询成员        | O(n)      | O(log n)     | O(log n)   | 平均 O(1)、最坏 O(n)   |
| 遍历成员        | O(n)      | O(n)         | O(n)       | O(m)                   |
| 检测对象相等    | O(n^2)    | O(n)         | O(n)       | 平均 O(n)、最坏 O(n^2) |
| 空间            | O(m)      | O(m)         | O(n)       | O(m)                   |

在 ECMA-404 标准中，并没有规定对象中每个成员的键一定要唯一的，也没有规定是否需要维持成员的次序。

为了简单起见，我们的 leptjson 选择用动态数组的方案。我们将在单元八才加入动态功能，所以这单元中，每个对象仅仅是成员的数组。那么它跟上一单元的数组非常接近：

```
// 用于封装JSON值和类型
struct lept_value
{
    union 
    {
        struct array
        {   
            lept_value* e;
            size_t len;
        }m_array;

        struct str
        {
            char *s;
            size_t len;
        }m_str;

        struct object
        {
            lept_member* m;
            size_t size;
        }m_object;
        
        double n;
    }m_union;

    lept_type type;
};

struct lept_member
{
    char* k;        // 对象的键
    size_t klen;    // 对象的键长度
    lept_value v;   // 对象的值
};
```

成员结构 `lept_member` 是一个 `lept_value` 加上键的字符串。如同 JSON 字符串的值，我们也需要同时保留字符串的长度，因为字符串本身可能包含空字符 `\u0000`。

在这单元中，我们仅添加了最基本的访问函数，用于撰写单元测试：

```
size_t lept_get_object_size(const lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);
```

在软件开发过程中，许多时候，选择合适的数据结构后已等于完成一半工作。没有完美的数据结构，所以最好考虑多一些应用的场合，看看时间／空间复杂度以至相关系数是否合适。

## 重构字符串解析

在软件工程中，[代码重构](https://zh.wikipedia.org/wiki/代码重构)（code refactoring）是指在不改变软件外在行为时，修改代码以改进结构。代码重构十分依赖于单元测试，因为我们是通过单元测试去维护代码的正确性。有了足够的单元测试，我们可以放胆去重构，尝试并评估不同的改进方式，找到合乎心意而且能通过单元测试的改动，我们才提交它。

我们知道，成员的键也是一个 JSON 字符串，然而，我们不使用 `lept_value` 存储键，因为这样会浪费了当中 `type` 这个无用的字段。由于 `lept_parse_string()` 是直接地把解析的结果写进一个 `lept_value`，所以我们先用「提取方法（extract method，见下注）」的重构方式，把解析 JSON 字符串及写入 `lept_value` 分拆成两部分：

```
/* 解析 JSON 字符串，把结果写入 str 和 len */
/* str 指向 c->stack 中的元素，需要在 c->stack  */
static int lept_parse_string_raw(lept_context* c, char** str, size_t* len) {
    size_t head = c->top;
    unsigned u, u2;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                *len = c->top - head;
                *str = lept_context_pop(c, *len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\\':
                switch (*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if (!(p = lept_parse_hex4(p, &u)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*p++ != '\\')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        lept_encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20)
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                PUTC(c, ch);
        }
    }
}

static int lept_parse_string(lept_context* c, lept_value* v) {
    int ret;
    char* s;
    size_t len;
    if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK)
        lept_set_string(v, s, len);
    return ret;
}
```

这样的话，我们实现对象的解析时，就可以使用 `lept_parse_string_raw()`　来解析 JSON 字符串，然后把结果复制至 `lept_member` 的 `k` 和 `klen` 字段。

## 实现

`lept_parse_object` 函数用于解析 JSON 字符串中的 JSON 对象。JSON 对象的语法要求对象以花括号 `{}` 包裹，其中包含一系列键值对。解析的详细步骤如下：

1. 函数开始时，期望当前 JSON 字符串的位置处有一个左花括号 `{`，如果没有找到它，函数将返回错误码 `LEPT_PARSE_EXPECT_VALUE`。
2. 然后，它跳过任何空白字符，并检查对象是否为空（由右花括号 `}` 表示）。如果对象为空，它将设置 `lept_value` 对象 `v` 表示一个空的 JSON 对象，并返回 `LEPT_PARSE_OK`。
3. 函数初始化一个 `lept_member` 结构 `m`，用于表示每个键值对。键（`m.k`）最初被设置为 `NULL`，而 `size` 用于跟踪对象中键值对的数量。
4. 函数进入循环，用于解析键值对。在循环内部：
   - 它期望键是一个字符串，通过检查当前字符是否为双引号 `"` 来判断。如果不是，它会返回错误码 `LEPT_PARSE_MISS_KEY`。
   - 然后，它调用 `lept_parse_string_raw` 来解析键字符串，并将其存储在 `str` 指针中。如果解析键字符串失败，它将返回 `lept_parse_string_raw` 返回的错误码。
   - 为键字符串分配内存，并将解析后的键复制到其中。
   - 函数确保键以 null 结尾。
   - 然后，它跳过空白字符并检查是否有冒号 `':'`。如果缺少冒号，它将返回错误码 `LEPT_PARSE_MISS_COLON`。
   - 冒号后，它跳过更多的空白字符。
   - 函数使用 `lept_parse_value` 解析与键关联的值。
5. 解析后的 `m`（键值对）使用 `lept_context_push` 推入堆栈。
6. 递增 `size` 以跟踪键值对的数量。
7. 为键字符串分配的内存将设置为 `NULL`，以将所有权转移到堆栈上的 `m` 结构。
8. 然后，函数检查是否有逗号 `,` 或右花括号 `}`，以确定是否有更多的键值对或对象是否已完成。
   - 如果找到逗号，它继续循环以处理下一个键值对。
   - 如果找到右花括号，它准备返回并完成解析。
9. 如果在解析的任何部分中发生错误，它会立即返回错误码。
10. 循环结束后，如果函数运行到这一步，它将返回错误码 `LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET`，表示期望逗号或右花括号，但遇到其他字符。
11. 如果解析成功，它将释放键字符串和堆栈上每个 `lept_member` 的值，并返回表示已解析的 JSON 对象的最终 `lept_value` 对象。
12. 如果在任何解析步骤中发生错误，它将释放任何已分配的内存，并将 `v` 设置为 `LEPT_NULL`，表示空值。

```
static int lept_parse_object(lept_context* c, lept_value* v) {
    size_t i, size;
    lept_member m;
    int ret;
    EXPECT(c, '{');
    lept_parse_whitespace(c);
    // 空对象
    if (*c->json == '}') {
        c->json++;
        v->type = LEPT_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return LEPT_PARSE_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;) {
        char* str;
        lept_init(&m.v);
        // 对象内部是用键值对存放数据的，此时解析键
        if (*c->json != '"') {
            ret = LEPT_PARSE_MISS_KEY;
            break;
        }
        if ((ret = lept_parse_string_raw(c, &str, &m.klen)) != LEPT_PARSE_OK)
            break;
        memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
        m.k[m.klen] = '\0';
        /* 
        parse ws 表示解析空白字符（whitespace）的步骤。在 JSON 中，空白字符包括空格、制表符、换行符等。在解析 JSON 对象时，解析器通常会跳过这些空白字符，以便找到实际的键。

		colon 表示解析冒号 : 的步骤。在 JSON 中，键和值之间由冒号分隔。在解析 JSON 对象时，解析器会查找冒号来区分键和值。

		ws 再次表示解析空白字符的步骤。这是因为冒号后可以有任意数量的空白字符，而解析器需要跳过这些空白字符以获取值。

		所以，parse ws colon ws 意味着在解析 JSON 对象时，解析器会首先跳过空白字符，然后查找冒号，再次跳过可能存在的空白字符，以将键和值正确分开。
        */
        lept_parse_whitespace(c);
        if (*c->json != ':') {
            ret = LEPT_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        lept_parse_whitespace(c);
        // 解析值
        if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK)
            break;
        memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
        size++;
        m.k = NULL; // 将所有权转移到栈上
        /*
        ws 表示解析空白字符（whitespace）的步骤。在 JSON 中，空白字符包括空格、制表符、换行符等。解析器通常会跳过这些字符以使 JSON 更容易阅读，因此在这一步中，解析器会跳过任何空白字符。

		[comma | right-curly-brace] 表示在此位置可以接受两种不同的标记：逗号（,）或右花括号（}）。在 JSON 对象中，逗号用于分隔对象的不同键值对，右花括号用于表示对象的结尾。

		parse ws [comma | right-curly-brace] ws,通过该描述，表示解析器在这一步骤会跳过可能存在的空白字符，然后检查下一个字符。如果下一个字符是逗号（,），那么解析器会继续解析下一个键值对；如果下一个字符是右花括号（}），那么解析器会认为对象解析结束。

这段描述告诉我们在解析 JSON 对象时，解析器会在键值对之间的逗号和对象的结束右花括号之间进行处理，以便正确分隔不同的键值对，或者标识对象的结束。
        */
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if (*c->json == '}') {
            size_t s = sizeof(lept_member) * size;
            c->json++;
            v->type = LEPT_OBJECT;
            v->u.o.size = size;
            memcpy(v->u.o.m = (lept_member*)malloc(s), lept_context_pop(c, s), s);
            return LEPT_PARSE_OK;
        }
        else {
            ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    /* Pop and free members on the stack */
    free(m.k);
    for (i = 0; i < size; i++) {
        lept_member* m = (lept_member*)lept_context_pop(c, sizeof(lept_member));
        free(m->k);
        lept_free(&m->v);
    }
    v->type = LEPT_NULL;
    return ret;
}
```

