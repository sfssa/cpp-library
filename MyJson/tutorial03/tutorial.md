# 从零开始的JSON库教程（三）：解析字符串

## JSON字符串语法

JSON 的字符串语法和 C 语言很相似，都是以双引号把字符括起来，如 `"Hello"`。但字符串采用了双引号作分隔，那么怎样可以在字符串中插入一个双引号？长久以来，无论是哪门语言，都是通过转义字符来解决这个问题的。因此，本个JSON项目需要引入转义字符（escape character），C/C++ 语言和 JSON 都使用 `\`（反斜线）作为转义字符，那么 `"` 在字符串中就表示为 `\"`，`a"b` 的 JSON 字符串则写成 `"a\"b"`。如以下的字符串语法所示，JSON 共支持 9 种转义序列：

```
string = quotation-mark *char quotation-mark
char = unescaped /
   escape (
       %x22 /          ; "    quotation mark  U+0022
       %x5C /          ; \    reverse solidus U+005C
       %x2F /          ; /    solidus         U+002F
       %x62 /          ; b    backspace       U+0008
       %x66 /          ; f    form feed       U+000C
       %x6E /          ; n    line feed       U+000A
       %x72 /          ; r    carriage return U+000D
       %x74 /          ; t    tab             U+0009
       %x75 4HEXDIG )  ; uXXXX                U+XXXX
escape = %x5C          ; \
quotation-mark = %x22  ; "
unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
```

简单来说：string：一个字符串由两个引号（quotation-mark）包围，并且中间可以有零个或多个字符（char）。

char：字符可以是未转义的字符（unescaped）或者转义的字符（escape）。转义的字符可以是以下几种：

\"：引号（quotation mark）
\\：反斜杠（reverse solidus）
\/：斜杠（solidus）
\b：退格（backspace）
\f：换页符（form feed）
\n：换行符（line feed）
\r：回车符（carriage return）
\t：制表符（tab）
\uXXXX：Unicode字符，其中XXXX是4位十六进制数。
escape：反斜杠，用于表示转义字符。

quotation-mark：引号，用于包围字符串。

unescaped：未转义的字符，可以是U+0020到U+0021、U+0023到U+005B、U+005D到U+10FFFF之间的任何Unicode字符。

转义序列有 9 种，都是以反斜线开始，如常见的 `\n` 代表换行符。

## 字符串表示

在 C/C++ 语言中，字符串一般表示为空结尾字符串（null-terminated string），即以空字符（`'\0'`）代表字符串的结束。但是每次都要遍历整个字符串，当找到'\0'所在时才知道到达了字符串的末尾，并不方便。在本个项目中采用redis中的simple dynamic string。

我们分配内存存储解析后的字符，并通过一个整型记录字符串的长度，由于大部分 C 程序都假设字符串是空结尾字符串，我们还是在最后加上一个空字符，那么不需处理 `\u0000` 这种字符的应用可以简单地把它当作是空结尾字符串（兼容C的函数）。

`lept_value` 事实上是一种变体类型（variant type），我们通过 `type` 来决定它现时是哪种类型，而这也决定了哪些成员是有效的。首先我们简单地在这个结构中加入两个成员：

```
struct lept_value{
    char* s;
    size_t len;
    double n;
    lept_type type;
};
```

我们很容易发现，一个json类型不能既是浮点型，又是字符串类型。无论是哪种类型，在使用时只需要一种类型。根据这个特点有没有想到关联的结构？没错，就是联合体！于是，将代码修改如下：

```
struct lept_value{
    union u{
        struct str{ char* s; size_t len; };  // 字符串类型
        double n;                          	 // 数字类型
    };
    lept_type type;
};
```

两者的内存分布如下所示：

![](https://github.com/sfssa/cpp-library/blob/master/MyJson/static/memory.png)

我们要把之前的 `v->n` 改成 `v->u.n`。而要访问字符串的数据，则要使用 `v->u.s.s` 和 `v->u.s.len`。这种写法比较麻烦吧，其实 C11 新增了匿名 struct/union 语法，就可以采用 `v->n`、`v->s`、`v->len` 来作访问。

## 内存管理

由于字符串的长度不是固定的，我们要动态分配内存。为简单起见，我们使用标准库 `<stdlib.h>` 中的 `malloc()`、`realloc()` 和 `free()` 来分配／释放内存。当设置一个值为字符串时，我们需要把参数中的字符串复制一份：

```
void lept_set_string(lept_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char*)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}
```

注意，在设置这个 `v` 之前，我们需要先调用 `lept_free(v)` 去清空 `v` 可能分配到的内存。例如原来已有一字符串，我们要先把它释放。然后就是简单地用 `malloc()` 分配及用 `memcpy()` 复制，并补上结尾空字符。`malloc(len + 1)` 中的 1 是因为结尾空字符。

那么，再看看 `lept_free()`：

```
void lept_free(lept_value* v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING)
        free(v->u.s.s);
    v->type = LEPT_NULL;
}
```

现时仅当值是字符串类型，我们才要处理，之后我们还要加上对数组及对象的释放。`lept_free(v)` 之后，会把它的类型变成 null。这个设计能避免重复释放。

但也由于我们会检查 `v` 的类型，在调用所有访问函数之前，我们必须初始化该类型。所以我们加入 `lept_init(v)`，因非常简单我们用宏实现：

```
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)
```

用上 `do { ... } while(0)` 是为了把表达式转为语句，模仿无返回值的函数。

其实在前两个单元中，我们只提供读取值的 API，没有写入的 API，就是因为写入时我们还要考虑释放内存。我们在本单元中把它们补全：

```
#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
```

由于 `lept_free()` 实际上也会把 `v` 变成 null 值，我们只用一个宏来提供 `lept_set_null()` 这个 API。

应用方的代码在调用 `lept_parse()` 之后，最终也应该调用 `lept_free()` 去释放内存。我们把之前的单元测试也加入此调用。

如果不使用 `lept_parse()`，我们需要先初始化再释放内存。

## 缓冲区与堆栈

我们解析字符串（以及之后的数组、对象）时，需要把解析的结果先储存在一个临时的缓冲区，最后再用 `lept_set_string()` 把缓冲区的结果设进值之中。在完成解析一个字符串之前，这个缓冲区的大小是不能预知的。因此，我们可以采用动态数组（dynamic array）这种数据结构，即数组空间不足时，能自动扩展。C++ 标准库的 `std::vector` 也是一种动态数组。

如果每次解析字符串时，都重新建一个动态数组，那么是比较耗时的。我们可以重用这个动态数组，每次解析 JSON 时就只需要创建一个。而且我们将会发现，无论是解析字符串、数组或对象，我们也只需要以先进后出的方式访问这个动态数组。换句话说，我们需要一个动态的堆栈（stack）数据结构。

我们把一个动态堆栈的数据放进 `lept_context` 里：

```
struct lept_context{
    const char* json;
    char* stack;
    size_t size, top;
};
```

当中 `size` 是当前的堆栈容量，`top` 是栈顶的位置（由于我们会扩展 `stack`，所以不要把 `top` 用指针形式存储）。

然后，我们在创建 `lept_context` 的时候初始化 `stack` 并最终释放内存：

```
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;        /* <- */
    c.size = c.top = 0;    /* <- */
    lept_init(v);
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        /* ... */
    }
    assert(c.top == 0);    /* <- */
    free(c.stack);         /* <- */
    return ret;
}
```

在释放时，加入了断言确保所有数据都被弹出。

然后，我们实现堆栈的压入及弹出操作。和普通的堆栈不一样，我们这个堆栈是以字节储存的。每次可要求压入任意大小的数据，它会返回数据起始的指针：

```
const size_t LEPT_PARSE_STACK_INIT_SIZE=256;

static void* lept_context_push(lept_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* lept_context_pop(lept_context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}
```

lept_context_push接受两个参数：一个指向lept_context的指针和空间的大小。在栈中请求一定的空间，并返回这块空间的地址：

- 检查请求的空间是否大于0，不是则触发断言错误；
- 检查栈是否有足够空间满足要求。如果没有则需要扩大栈容量。扩容策略是：如果当前栈的空间是0，就将容量设置为全局变量的大小，否则不断扩容直至足够大以满足请求，每次扩容1.5倍；
- 使用realloc函数重新分配栈的内存，分配成功static将指向新分配的内存；
- 最后，计算新分配空间的地址，并更新栈的当前大小；

其中，json还是通过'\0'来判断字符串的结束，size和top都是为栈服务的。在`lept_context`这个结构体中，`json`和`stack`分别有各自的用途：

- `json`：这是一个指向字符的常量指针，通常用于存储待解析的JSON字符串。例如，如果我们有一个JSON字符串`"{\"name\":\"John\",\"age\":30}"`，那么我们可以让`json`指向这个字符串。
- `stack`：这是一个字符指针，通常用于存储解析过程中需要的临时数据。例如，在解析上述JSON字符串时，我们可能需要临时存储键（“name"和"age”）和值（"John"和30）。这些临时数据可以存储在`stack`中。

## 解析字符串

重新解释下所有的枚举类型所代表的意义：

- `LEPT_PARSE_OK=0`：解析成功，没有错误。
- `LEPT_PARSE_EXPECT_VALUE`：预期一个值，但没有找到。例如，输入的字符串为空或只包含空白字符。
- `LEPT_PARSE_INVALID_VALUE`：输入的值无效。例如，一个非法的数字或字符串。
- `LEPT_PARSE_ROOT_NOT_SINGULAR`：在值之后还有其他字符。按照JSON的语法，一个有效的JSON文本只能包含一个值。
- `LEPT_PARSE_NUMBER_TOO_BIG`：数字太大，超出了解析器能处理的范围。
- `LEPT_PARSE_MISS_QUOTATION_MERK`：缺少引号。在JSON中，字符串必须用双引号包围。
- `LEPT_PARSE_INVALID_STRING_ESCAPE`：字符串中有无效的转义字符。
- `LEPT_PARSE_INVALID_STRING_CHAR`：字符串中有无效的字符。

有了以上的工具，解析字符串的任务就变得很简单。我们只需要先备份栈顶，然后把解析到的字符压栈，最后计算出长度并一次性把所有字符弹出，再设置至值里便可以。以下是部分实现，没有处理转义和一些不合法字符的校验。

```
#define PUTC(c, ch) do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)

static int lept_parse_string(lept_context* c,lept_value* v)
{
    size_t head=c->top,len;
    const char* p;
    EXPECT(c,'\"');
    p=c->json;
    for(;;)
    {
        char ch=*p++;
        switch(ch)
        {
            case '\"':
            {
                len=c->top-head;
                lept_set_string(v,(const char*)lept_context_pop(c,len),len);
                c->json=p;
                return LEPT_PARSE_OK;
            }
            case '\\':
            {
                switch(*p++)
                {
                    case '\"':PUTC(c,'\"');break;
                    case '\\':PUTC(c,'\\');break;
                    case '/':PUTC(c,'/');break;
                    case 'b':PUTC(c,'\b');break;
                    case 'f':PUTC(c,'\f');break;
                    case 'n':PUTC(c,'\n');break;
                    case 'r':PUTC(c,'\r');break;
                    case 't':PUTC(c,'\t');break;
                    default:
                    {
                        c->top=head;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                    }
                }
                break;
            }
            case '\0':
            {
                c->top=head;
                return LEPT_PARSE_MISS_QUOTATION_MERK;
            }
            default:
                PUTC(c,ch);
        }
    }
}
```

首先用head记录当前解析的字符串的起始地址，比较字符串是否是以双引号开头，如果不是则表示不是有效的字符串。进入一个无限循环中，如果遇到了另一个双引号，则代表字符串解析完毕，计算字符串长度len，将栈内的字符全部出栈并赋值给变量lept_value，返回解析成功；如果是转义字符，则将转义字符转换成ASCII码并入栈；如果遇到了\0则代表字符串未正常关闭；如果不满足上述情况，则代表是一个字符，将字符入栈。

![](https://github.com/sfssa/cpp-library/blob/master/MyJson/static/03.jpg)
