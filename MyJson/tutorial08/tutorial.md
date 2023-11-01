# 从零开始的JSON库教程（八）：访问与其他功能

## 对象键值查询

我们在第六个单元实现了 JSON 对象的数据结构，它仅为一个 `lept_value` 的数组：

```
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
```

为了做相应的解析测试，我们实现了最基本的查询功能：

```
size_t lept_get_object_size(const lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);
```

在实际使用时，我们许多时候需要查询一个键值是否存在，如存在，要获得其相应的值。我们可以提供一个函数，简单地用线性搜寻实现这个查询功能（时间复杂度 O(n)）：

```
#define LEPT_KEY_NOT_EXIST ((size_t)-1)

size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen) {
    size_t i;
    assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
    for (i = 0; i < v->u.o.size; i++)
        if (v->u.o.m[i].klen == klen && memcmp(v->u.o.m[i].k, key, klen) == 0)
            return i;
    return LEPT_KEY_NOT_EXIST;
}}
```

若对象内没有所需的键，此函数返回 `LEPT_KEY_NOT_EXIST`。使用时：

```
lept_value o;
size_t index;
lept_init(&o);
lept_parse(&o, "{\"name\":\"Milo\", \"gender\":\"M\"}");
index = lept_find_object_index(&o, "name", 4);
if (index != LEPT_KEY_NOT_EXIST) {
    lept_value* v = lept_get_object_value(&o, index);
    printf("%s\n", lept_get_string(v));
}
lept_free(&o);
```

由于一般也是希望获取键对应的值，而不需要索引，我们再加入一个辅助函数，返回类型改为 `lept_value*`：

```
lept_value* lept_find_object_value(const lept_value* v, const char* key, size_t klen) {
    size_t index = lept_find_object_index(v, key, klen);
    return index != LEPT_KEY_NOT_EXIST ? &v->u.o.m[index].v : NULL;
}
```

上述例子便可简化为：

```
lept_value o, *v;
/* ... */
if ((v = lept_find_object_value(&o, "name", 4)) != NULL)
    printf("%s\n", lept_get_string(v));
```

## 相等比较

在实现数组和对象的修改之前，为了测试结果的正确性，我们先实现 `lept_value` 的[相等比较](https://zh.wikipedia.org/zh-cn/關係運算子)（equality comparison）。首先，两个值的类型必须相同，对于 true、false、null 这三种类型，比较类型后便完成比较。而对于数字和字符串，需进一步检查是否相等：

```
int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs->type != rhs->type)
        return 0;
    switch (lhs->type) {
        case LEPT_STRING:
            return lhs->u.s.len == rhs->u.s.len && 
                memcmp(lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
        case LEPT_NUMBER:
            return lhs->u.n == rhs->u.n;
        /* ... */
        default:
            return 1;
    }
}
```

由于值可能复合类型（数组和对象），也就是一个树形结构。当我们要比较两个树是否相等，可通过递归实现。例如，对于数组，我们先比较元素数目是否相等，然后递归检查对应的元素是否相等：

```
int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
    size_t i;
    /* ... */
    switch (lhs->type) {
        /* ... */
        case LEPT_ARRAY:
            if (lhs->u.a.size != rhs->u.a.size)
                return 0;
            for (i = 0; i < lhs->u.a.size; i++)
                if (!lept_is_equal(&lhs->u.a.e[i], &rhs->u.a.e[i]))
                    return 0;
            return 1;
        /* ... */
    }
}
```

而对象与数组的不同之处，在于概念上对象的键值对是无序的。例如，`{"a":1,"b":2}` 和 `{"b":2,"a":1}` 虽然键值的次序不同，但这两个 JSON 对象是相等的。我们可以简单地利用 `lept_find_object_index()` 去找出对应的值，然后递归作比较。

## 复制、移动和转换

功能，使用者也可能会用到，例如把一个 JSON 复制一个版本出来修改，保持原来的不变。所以，我们实现一个公开的深度复制函数：

```
void lept_copy(lept_value* dst, const lept_value* src) {
    size_t i;
    assert(src != NULL && dst != NULL && src != dst);
    switch (src->type) {
        case LEPT_STRING:
            lept_set_string(dst, src->u.s.s, src->u.s.len);
            break;
        case LEPT_ARRAY:
            /* \todo */
            break;
        case LEPT_OBJECT:
            /* \todo */
            break;
        default:
            lept_free(dst);
            memcpy(dst, src, sizeof(lept_value));
            break;
    }
}
```

C++11 加入了右值引用的功能，可以从语言层面区分复制和移动语意。而在 C 语言中，我们也可以通过实现不同版本的接口（不同名字的函数），实现这两种语意。但为了令接口更简单和正交（orthogonal），我们修改了 `lept_set_object_value()` 的设计，让它返回新增键值对的值指针，所以我们可以用 `lept_copy()` 去复制赋值，也可以简单地改变新增的键值：

```
/* 返回新增键值对的指针 */
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen);

void f() {
    lept_value v;
    lept_init(&v);
    lept_parse(&v, "{}");
    lept_set_string(lept_set_object_value(&v, "s"), "Hello", 5);
    /* {"s":"Hello"} */
    lept_copy(
        lept_add_object_keyvalue(&v, "t"),
        lept_get_object_keyvalue(&v, "s", 1));
    /* {"s":"Hello","t":"Hello"} */
    lept_free(&v);
}
```

我们还提供了 `lept_move()`，它的实现也非常简单：

```
void lept_move(lept_value* dst, lept_value* src) {
    assert(dst != NULL && src != NULL && src != dst);
    lept_free(dst);
    memcpy(dst, src, sizeof(lept_value));
    lept_init(src);
}
```

类似地，我们也实现了一个交换值的接口：

```
void lept_swap(lept_value* lhs, lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs) {
        lept_value temp;
        memcpy(&temp, lhs, sizeof(lept_value));
        memcpy(lhs,   rhs, sizeof(lept_value));
        memcpy(rhs, &temp, sizeof(lept_value));
    }
}
```

当我们要修改对象或数组里的值时，我们可以利用这 3 个函数。例如：

```
const char* json = "{\"a\":[1,2],\"b\":3}";
char *out;
lept_value v;
lept_init(&v);
lept_parse(&v, json);
lept_copy(
    lept_find_object_value(&v, "b", 1),
    lept_find_object_value(&v, "a", 1));
printf("%s\n", out = lept_stringify(&v, NULL)); /* {"a":[1,2],"b":[1,2]} */
free(out);

lept_parse(&v, json);
lept_move(
    lept_find_object_value(&v, "b", 1),
    lept_find_object_value(&v, "a", 1));
printf("%s\n", out = lept_stringify(&v, NULL)); /* {"a":null,"b":[1,2]} */
free(out);

lept_parse(&v, json);
lept_swap(
    lept_find_object_value(&v, "b", 1),
    lept_find_object_value(&v, "a", 1));
printf("%s\n", out = lept_stringify(&v, NULL)); /* {"a":3,"b":[1,2]} */
free(out);

lept_free(&v);
```

在使用时，可尽量避免 `lept_copy()`，而改用 `lept_move()` 或 `lept_swap()`，因为后者不需要分配内存。当中 `lept_swap()` 更是无须做释放的工作，令它达到 O(1) 时间复杂度，其性能与值的内容无关。

## 动态数组

在此单元之前的实现里，每个数组的元素数目在解析后是固定不变的，其数据结构是：

```
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
```

用这种数据结构增删元素时，我们需要重新分配一个数组，把适当的旧数据拷贝过去。但这种做法是非常低效的。例如我们想要从一个空的数组加入 n个元素，便要做 n(n−1)/2 次元素复制，即 O(n2) 的时间复杂度。

其中一个改进方法，是使用动态数组（dynamic array，或称可增长数组／growable array）的数据结构。C++ STL 标准库中最常用的 `std::vector` 也是使用这种数据结构的容器。

改动也很简单，只需要在数组中加入容量 `capacity` 字段，表示当前已分配的元素数目，而 `size` 则表示现时的有效元素数目：

```
struct array
{   
	lept_value* e;
    size_t len;
    size_t capacity;
}m_array;
```

我们终于提供设置数组的函数，而且它可提供初始的容量：

```
void lept_set_array(lept_value* v, size_t capacity) {
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_ARRAY;
    v->u.a.size = 0;
    v->u.a.capacity = capacity;
    v->u.a.e = capacity > 0 ? (lept_value*)malloc(capacity * sizeof(lept_value)) : NULL;
}
```

我们需要稍修改 `lept_parse_array()`，调用 `lept_set_array()` 去设置类型和分配空间。

另外，类似于 `lept_get_array_size()`，也加入获取当前容量的函数：

```
size_t lept_get_array_capacity(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.capacity;
}
```

如果当前的容量不足，我们需要扩大容量，标准库的 `realloc()` 可以分配新的内存并把旧的数据拷背过去：

```
void lept_reserve_array(lept_value* v, size_t capacity) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.a.capacity < capacity) {
        v->u.a.capacity = capacity;
        v->u.a.e = (lept_value*)realloc(v->u.a.e, capacity * sizeof(lept_value));
    }
}
```

当数组不需要再修改，可以使用以下的函数，把容量缩小至刚好能放置现有元素：

```
void lept_shrink_array(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.a.capacity > v->u.a.size) {
        v->u.a.capacity = v->u.a.size;
        v->u.a.e = (lept_value*)realloc(v->u.a.e, v->u.a.capacity * sizeof(lept_value));
    }
}
```

我们不逐一检视每个数组修改函数，仅介绍一下两个例子：

```
lept_value* lept_pushback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.a.size == v->u.a.capacity)
        lept_reserve_array(v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2);
    lept_init(&v->u.a.e[v->u.a.size]);
    return &v->u.a.e[v->u.a.size++];
}

void lept_popback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY && v->u.a.size > 0);
    lept_free(&v->u.a.e[--v->u.a.size]);
}
```

`lept_pushback_array_element()` 在数组末端压入一个元素，返回新的元素指针。如果现有的容量不足，就需要调用 `lept_reserve_array()` 扩容。我们现在用了一个最简单的扩容公式：若容量为 0，则分配 1 个元素；其他情况倍增容量。

`lept_popback_array_element()` 则做相反的工作，记得删去的元素需要调用 `lept_free()`。

## 动态对象

