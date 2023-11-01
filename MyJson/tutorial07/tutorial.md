# 从零开始的JSON库教程（七）：生成器

## JSON生成器

我们在前 6 个单元实现了一个合乎标准的 JSON 解析器，它把 JSON 文本解析成一个树形数据结构，整个结构以 `lept_value` 的节点组成。

JSON 生成器（generator）负责相反的事情，就是把树形数据结构转换成 JSON 文本。这个过程又称为「字符串化（stringify）」。

![](https://github.com/sfssa/cpp-library/blob/master/MyJson/static/07.jpg)

相对于解析器，通常生成器更容易实现，而且生成器几乎不会造成运行时错误。因此，生成器的 API 设计为以下形式，直接返回 JSON 的字符串：

```
char* lept_stringify(const lept_value* v, size_t* length);
```

`length` 参数是可选的，它会存储 JSON 的长度，传入 `NULL` 可忽略此参数。使用方需负责用 `free()` 释放内存。

为了简单起见，我们不做换行、缩进等美化（prettify）处理，因此它生成的 JSON 会是单行、无空白字符的最紧凑形式。

## 再利用lept_context做动态数组

在实现 JSON 解析时，我们加入了一个动态变长的堆栈，用于存储临时的解析结果。而现在，我们也需要存储生成的结果，所以最简单是再利用该数据结构，作为输出缓冲区。

```
const int LEPT_PARSE_STRINGIFY_INIT_SIZE = 256;

int lept_stringify(const lept_value* v, char** json, size_t* length) {
    lept_context c;
    int ret;
    assert(v != NULL);
    assert(json != NULL);
    c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);
    c.top = 0;
    if ((ret = lept_stringify_value(&c, v)) != LEPT_STRINGIFY_OK) {
        free(c.stack);
        *json = NULL;
        return ret;
    }
    if (length)
        *length = c.top;
    PUTC(&c, '\0');
    *json = c.stack;
    return LEPT_STRINGIFY_OK;
}
```

1. `int lept_stringify(const lept_value* v, char** json, size_t* length)`：这是函数的声明，它接受一个 `lept_value` 结构体指针 `v`，用于表示要序列化的 JSON 数据；`char** json` 用于接收序列化后的 JSON 字符串，`size_t* length` 用于接收 JSON 字符串的长度。
2. `lept_context c;`：在函数内部创建一个 `lept_context` 结构体 `c`，它用于维护序列化过程中的状态和缓冲区。
3. `assert(v != NULL);` 和 `assert(json != NULL);`：这些断言用于确保传入的参数 `v` 和 `json` 不为 NULL，如果为 NULL，将会触发断言失败并终止程序。
4. `c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);`：分配一个初始大小的字符缓冲区用于存储 JSON 字符串。`LEPT_PARSE_STRINGIFY_INIT_SIZE` 可能是一个预定义的常数，用于指定初始缓冲区大小。函数同时初始化了 `c.stack` 和 `c.size` 字段。
5. `c.top = 0;`：初始化 `c.top` 为 0，表示缓冲区中目前没有字符。
6. `if ((ret = lept_stringify_value(&c, v)) != LEPT_STRINGIFY_OK)`：调用 `lept_stringify_value` 函数，传递 `c` 和 `v` 作为参数进行 JSON 序列化。如果序列化失败，`lept_stringify_value` 会返回一个错误码，保存在 `ret` 变量中。
7. `free(c.stack);`：无论序列化是否成功，都需要释放之前分配的缓冲区 `c.stack`，以免内存泄漏。
8. `*json = NULL;`：将 `json` 指向的指针置为空，以确保不会包含任何随机数据。
9. `if (length)`：检查 `length` 指针是否为非空，如果是非空，则表示用户想要获取 JSON 字符串的长度。
10. `*length = c.top;`：如果 `length` 指针非空，将 JSON 字符串的长度存储在 `length` 指向的位置。
11. `PUTC(&c, '\0');`：在 JSON 字符串的末尾添加一个 null 终止字符，以确保生成的字符串可以用作 C 字符串。
12. `*json = c.stack;`：将 `json` 指向 `c.stack`，这样用户就可以通过 `json` 指针来访问序列化后的 JSON 字符串。
13. 最后，函数返回 `LEPT_STRINGIFY_OK` 或序列化过程中可能发生的其他错误码。

生成根节点的值之后，我们还需要加入一个空字符作结尾。

如前所述，此 API 还提供了 `length` 可选参数，当传入非空指针时，就能获得生成 JSON 的长度。或许读者会疑问，为什么需要获得长度，我们不是可以用 `strlen()` 获得么？是的，因为 JSON 不会含有空字符（若 JSON 字符串中含空字符，必须转义为 `\u0000`），用 `strlen()` 是没有问题的。但这样做会带来不必要的性能消耗，理想地是避免调用方有额外消耗。

## 生成null,false和true

接下来，我们生成最简单的 JSON 类型，就是 3 种 JSON 字面值。为贯彻 TDD，先写测试：

```
#define TEST_ROUNDTRIP(json)\
    do {\
        lept_value v;\
        char* json2;\
        size_t length;\
        lept_init(&v);\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRINGIFY_OK, lept_stringify(&v, &json2, &length));\
        EXPECT_EQ_STRING(json, json2, length);\
        lept_free(&v);\
        free(json2);\
    } while(0)

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    /* ... */
}
```

这里我们采用一个最简单的测试方式，把一个 JSON 解析，然后再生成另一 JSON，逐字符比较两个 JSON 是否一模一样。这种测试可称为往返（roundtrip）测试。但需要注意，同一个 JSON 的内容可以有多种不同的表示方式，例如可以插入不定数量的空白字符，数字 `1.0` 和 `1` 也是等价的。所以另一种测试方式，是比较两次解析的结果（`lept_value` 的树）是否相同，此功能将会在下一单元讲解。

然后，我们实现 `lept_stringify_value`，加入一个 `PUTS()` 宏去输出字符串：

```
#define PUTS(c, s, len)     memcpy(lept_context_push(c, len), s, len)

static int lept_stringify_value(lept_context* c, const lept_value* v) {
    size_t i;
    int ret;
    switch (v->type) {
        case LEPT_NULL:   PUTS(c, "null",  4); break;
        case LEPT_FALSE:  PUTS(c, "false", 5); break;
        case LEPT_TRUE:   PUTS(c, "true",  4); break;
        /* ... */
    }
    return LEPT_STRINGIFY_OK;
}
```

## 生成数字

为了简单起见，我们使用 `sprintf("%.17g", ...)` 来把浮点数转换成文本。`"%.17g"` 是足够把双精度浮点转换成可还原的文本。

最简单的实现方式可能是这样的：

```
case LEPT_NUMBER:
	{
		char buffer[32];
		int length = sprintf(buffer, "%.17g", v->u.n);
		PUTS(c, buffer, length);
	}
	break;
```

但这样需要在 `PUTS()` 中做一次 `memcpy()`，实际上我们可以避免这次复制，只需要生成的时候直接写进 `c` 里的堆栈，然后再按实际长度调查 `c->top`：

```
case LEPT_NUMBER:
	{
		char* buffer = lept_context_push(c, 32);
        int length = sprintf(buffer, "%.17g", v->u.n);
        c->top -= 32 - length;
    }
    break;
```

因每个临时变量只用了一次，我们可以把代码压缩成一行：

```
case LEPT_NUMBER:
	c->top -= 32 - sprintf(lept_context_push(c, 32), "%.17g", v->u.n);
    break;
```

## 生成字符串

```
static void lept_stringify_string(lept_context* c, const char* s, size_t len) {
    size_t i;
    assert(s != NULL);
    PUTC(c, '"');
    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i];
        switch (ch) {
            case '\"': PUTS(c, "\\\"", 2); break;
            case '\\': PUTS(c, "\\\\", 2); break;
            case '\b': PUTS(c, "\\b",  2); break;
            case '\f': PUTS(c, "\\f",  2); break;
            case '\n': PUTS(c, "\\n",  2); break;
            case '\r': PUTS(c, "\\r",  2); break;
            case '\t': PUTS(c, "\\t",  2); break;
            default:
                if (ch < 0x20) {
                    char buffer[7];
                    sprintf(buffer, "\\u%04X", ch);
                    PUTS(c, buffer, 6);
                }
                else
                    PUTC(c, s[i]);
        }
    }
    PUTC(c, '"');
}

static void lept_stringify_value(lept_context* c, const lept_value* v) {
    switch (v->type) {
        /* ... */
        case LEPT_STRING: lept_stringify_string(c, v->u.s.s, v->u.s.len); break;
        /* ... */
    }
}
```

注意到，十六进位输出的字母可以用大写或小写，我们这里选择了大写，所以 roundtrip 测试时也用大写。但这个并不是必然的，输出小写（用 `"\\u%04x"`）也可以。

## 生成数组和对象

生成数组也是非常简单，只要输出 `[` 和 `]`，中间对逐个子值递归调用 `lept_stringify_value()`。只要注意在第一个元素后才加入 `,`。而对象也仅是多了一个键和 `:`。

```
static void lept_stringify_value(lept_context* c, const lept_value* v) {
    size_t i;
    switch (v->type) {
        /* ... */
        case LEPT_ARRAY:
            PUTC(c, '[');
            for (i = 0; i < v->u.a.size; i++) {
                if (i > 0)
                    PUTC(c, ',');
                lept_stringify_value(c, &v->u.a.e[i]);
            }
            PUTC(c, ']');
            break;
        case LEPT_OBJECT:
            PUTC(c, '{');
            for (i = 0; i < v->u.o.size; i++) {
                if (i > 0)
                    PUTC(c, ',');
                lept_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
                PUTC(c, ':');
                lept_stringify_value(c, &v->u.o.m[i].v);
            }
            PUTC(c, '}');
            break;
        /* ... */
    }
}
```

