# 从零开始的JSON库教程（五）：解析数组
## JSON数组

一个 JSON 数组可以包含零至多个元素，而这些元素也可以是数组类型。换句话说，我们可以表示嵌套（nested）的数据结构。先来看看 JSON 数组的语法：

```
array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
```

当中，`%x5B` 是左中括号 `[`，`%x2C` 是逗号 `,`，`%x5D` 是右中括号 `]` ，`ws` 是空白字符。一个数组可以包含零至多个值，以逗号分隔，例如 `[]`、`[1,2,true]`、`[[1,2],[3,4],"abc"]` 都是合法的数组。但注意 JSON 不接受末端额外的逗号，例如 `[1,2,]` 是不合法的。

JSON 数组的语法很简单，实现的难点不在语法上，而是怎样管理内存。

## 数据结构

JSON 数组存储零至多个元素，最简单就是使用 C 语言的数组。数组最大的好处是能以 �(1) 用索引访问任意元素，次要好处是内存布局紧凑，省内存之余还有高缓存一致性（cache coherence）。但数组的缺点是不能快速插入元素，而且我们在解析 JSON 数组的时候，还不知道应该分配多大的数组才合适。

另一个选择是链表（linked list），它的最大优点是可快速地插入元素（开端、末端或中间），但需要以 �(�) 时间去经索引取得内容。如果我们只需顺序遍历，那么是没有问题的。还有一个小缺点，就是相对数组而言，链表在存储每个元素时有额外内存开销（存储下一节点的指针），而且遍历时元素所在的内存可能不连续，令缓存不命中（cache miss）的机会上升。

在本项目中使用数组，并通过堆栈解决解析JSON数组时未知数组大小的问题。

```
struct lept_value
{
    union 
    {
    	// 新增加的数组类型
        struct array
        {   
            lept_value* e;
            size_t len;	// 元素的个数
        }m_array;

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

## 解析过程

在解析 JSON 字符串时，因为在开始时不能知道字符串的长度，而又需要进行转义，所以需要一个临时缓冲区去存储解析后的结果。我们为此实现了一个动态增长的堆栈，可以不断压入字符，最后一次性把整个字符串弹出，复制至新分配的内存之中。

对于 JSON 数组，我们也可以用相同的方法，而且，我们可以用同一个堆栈！我们只需要把每个解析好的元素压入堆栈，解析到数组结束时，再一次性把所有元素弹出，复制至新分配的内存之中。只不过是利用栈先存储某个数据类型，当截取出完整的数据后将字符出栈并通过lept_value保存，之后将lept_value保存到栈中，然后移动top的指向，从下一个空间开始去存储下一个数据，不断重复直到将所有的数据都以lept_value的方式保存到栈中。

## 实现

```
static int lept_parse_array(lept_context* c,lept_value* v)
{
    size_t size=0;
    int ret;
    EXPECT(c,'[');
    lept_parse_whitespace(c);
    if(*c->json==']')
    {
        c->json++;
        v->type=LEPT_ARRAY;
        v->m_union.m_array.len=0;
        v->m_union.m_array.e=nullptr;
        return LEPT_PARSE_OK;
    }

    for(;;)
    {
        lept_value e;
        lept_init(&e);
        if((ret=lept_parse_value(c,&e))!=LEPT_PARSE_OK)
            return ret;
        memcpy(lept_context_push(c,sizeof(lept_value)),&e,sizeof(lept_value));
        size++;
        lept_parse_whitespace(c);
        if(*c->json==',')
        {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if(*c->json==']')
        {
            c->json++;
            v->type=LEPT_ARRAY;
            v->m_union.m_array.len=size;
            size*=sizeof(lept_value);
            memcpy(v->m_union.m_array.e=(lept_value*)malloc(size),lept_context_pop(c,size),size);
            return LEPT_PARSE_OK;
        }
        else
            return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;// 缺少右括号或逗号
    }
}
```

解析的过程在上文中已经介绍完毕，下面介绍下数组空间的释放。有C/C++基础的同学知道，C++释放数组中的指针需要使用delete []，也就是需要将数组中的每个元素都释放，因此，需要修改lept_free函数如下所示：

```
void lept_free(lept_value* v)
{
    size_t i;
    assert(v!=nullptr);
    switch(v->type)
    {
        case LEPT_STRING:
        {
            free(v->m_union.m_str.s);
            break;
        }
        case LEPT_ARRAY:
        {
            for(i=0;i<v->m_union.m_array.len;++i)
                lept_free(&v->m_union.m_array.e[i]);
            free(v->m_union.m_array.e);
            break;
        }
        default:
            break;
    }
    v->type=LEPT_NULL;
}
```

```
for(i=0;i<v->m_union.m_array.len;++i)
	lept_free(&v->m_union.m_array.e[i]);
free(v->m_union.m_array.e);
```

这里之所以又调用一次free是由于这个数组本身是通过lept_value来存放的，因此需要将自身的这个空间（放置数据的容器）释放掉。

## 指针失效

```
lept_value* e = lept_context_push(c, sizeof(lept_value));
```

看这个压栈过程，`lept_context_push()` 在发现栈满了的时候会用 `realloc()` 扩容。这时候，我们上层的 `e` 就会失效，变成一个悬挂指针（dangling pointer），而且 `lept_parse_value(c, e)` 会通过这个指针写入解析结果，造成非法访问。类似于C++中的vector的迭代器，删除某个数据后，删除点后面的元素的迭代器全部失效。
