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

