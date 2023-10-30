# 从零开始JSON库（二）：解析数字
## JSON数字语法

```
number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit
```

number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。只有整数是必需部分。注意和直觉可能不同的是，正号是不合法的。

整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）。也就是说，`0123` 不是一个合法的 JSON 数字。

小数部分比较直观，就是小数点后是一或多个数字（0-9）。

JSON 可使用科学记数法，指数部分由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。

![](https://github.com/sfssa/cpp-library/blob/master/MyJson/static/number.png)

## 数字表示方式

为了提高精度，使用双精度类型double来处理数字。在之前的lept_value中添加数字成员：

```
struct lept_value
{
	double val;
	lept_type type;
};
```

只有type==LEPT_NUMBER时，n表示JSON数字的具体数值，因此获取数值时需要检验类型是否匹配。

```
double lept_get_number(const lept_value* v)
{
	assert(v!=nullptr&&v->type==LEPT_NUMBER);
	return v->n;
}
```

## 十进制转换二进制

将十进制数字转换成二进制的doubel，使用标准库strtod来进行转换。由于一些JSON不容许的格式也可以被strtod函数转换，因此需要在转换前做格式校验。

```
static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}
```

加入了 number 后，value 的语法变成：

```
value = null / false / true / number
```

## 解决代码冗余

在第一节中，创建了lept_parse_null，lept_parse_false，lept_parse_true三个函数，这三个函数整体逻辑相同，通过修改函数参数来合成一个函数，避免代码冗余。

原函数接收的参数分别是：json的内容和类型，检查内容是否和true等相等，观察三个函数，最根本的区别就在于比较的内容不一样，分别比较的"true"，"false"，"null"，因此，只需要将类型作为参数传递过去即可，同时由于最终的类型不一致，因此需要将成功解析出来的结果也作为参数传递过去，如下所示：

```
static int lept_parse_literal(lept_context* c,lept_value*v,const char* literal,lept_type type)
{
    size_t i;
    EXPECT(c,literal[0]);
    for(i=0;literal[i+1];++i)
    {
        if(c->json[i]!=literal[i+1])
            return LEPT_PARSE_INVALID_VALUE;
        c->json+=i;
        v->type=type;
        return LEPT_PARSE_OK;
    }
}
```

## 代码详解

首先解释下`strtod`函数，这个函数用来将字符串类型(str)转换成(to)浮点类型(double)。

```
double strtod(const char *nptr, char **endptr);
nptr：要转换的字符串。
endptr：如果转换成功，它会指向字符串中的第一个无效字符的位置，即指向字符串的结束位置。如果没有无效字符，它会指向字符串的结束字符（null 终止字符）。
```

`strtod` 函数会从 `nptr` 字符串中解析浮点数，并将结果以 `double` 类型返回。函数会跳过前导的空格字符，然后尝试解析浮点数，直到遇到不合法的字符或字符串结束。

- 根据上文的数字转换方式，在解析时首先查看数字是否时负数，如果是的话继续向下检查；
- 如果字符是'0'那么表示这是一个以0开头的整数，检查是否是从1-9的数字开头，是就将指针p移动到整数末尾，继续移动指针；
- 如果遇到小数，表示这是一个浮点数，继续移动指针；
- 如果遇到'e'或'E'，表示这是一个科学计数法表示的数字，继续移动指针；
- 如果有'+'或'-'，继续移动 指针；
- 中途不断跳过连续的数字。

如果在转换时没有出现问题，那么就通过strtod函数进行转换，并将值和类型保存到lept_value中。其中`ERANGE` 是 C 标准库中定义的一个错误码，它表示在转换数值时发生了范围错误（range error）。通常，`strtod` 函数会在转换时检查数值是否超出了可表示的范围。如果超出范围，`errno` 会被设置为 `ERANGE`，然后返回一个特定的值（通常是正或负的无穷大，如 `HUGE_VAL` 或 `-HUGE_VAL`）以指示错误。
