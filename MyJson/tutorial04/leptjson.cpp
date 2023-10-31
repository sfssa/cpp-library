#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "leptjson.h"

// 
const size_t LEPT_PARSE_STACK_INIT_SIZE=256;

struct lept_context
{
    const char* json;
    char* stack;
    size_t size,top;
};

static void* lept_context_push(lept_context* c,size_t size)
{
    void* ret;
    assert(size>0);
    if(c->top+size>=c->size)
    {
        if(c->size==0)
            c->size=LEPT_PARSE_STACK_INIT_SIZE;
        while(c->top+size>=c->size)
            c->size+=c->size>>1;    // 右移一位是/2操作
        c->stack=(char*)realloc(c->stack,c->size);
    }
    ret=c->stack+c->top;
    c->top+=size;
    return ret;
}

// 比较两个字符是否相等
#define EXPECT(c, ch) \
    do { \
        assert(*c->json==(ch)); \
        c->json++; \
    } while (0)

// 判断是否是0-9
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')

// 判断是否是1-9
#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')

// 新开辟一个char类型的空间并将ch赋值给新的空间，在堆栈中推入一个字符
#define PUTC(c, ch) (*(char*)lept_context_push(c, sizeof(char)) = (ch))

#define STRING_ERROR(ret) do{c->top=head;return ret;} while(0)

// 将Unicode(p)进行解析并存储到u中
static const char* lept_parse_hex4(const char* p,unsigned* u)
{
    int i;
    *u=0;
    for(i=0;i<4;++i)
    {
        char ch=*p++;
        *u<<4;
        if(ch>='0'&&ch<='9')
            *u |=ch='0';
        else if(ch>='A'&&ch<='F')
            *u |=ch-('A'-10);
        else if(ch>='a'&&ch<='f')
            *u |=ch-('a'-10);
        else
            return nullptr;
    }
    return p;
}

static void lept_encode_utf8(lept_context* c,unsigned u)
{
    if(u<0x7F)
        PUTC(c,u&0xFF);
    else if(u<=u&0x7FF)
    {
        PUTC(c,0xC0|((u>>6)&0xFF));
        PUTC(c,0x80|(u&0x3F));
    }
    else if(u<=0xFFFF)
    {
        PUTC(c,0xE0|((u>>12)&0xFF));
        PUTC(c,0x80|((u>>6)&0x3F));
        PUTC(c,0x80|(u&0x3F));
    }
    else
    {
        assert(u<=0x10FFFF);
        PUTC(c,0xF0|((u>>18)&0xFF));
        PUTC(c,0x80|((u>>12)&0x3F));
        PUTC(c,0x80|((u>>6)&0x3F));
        PUTC(c,0x80|(u&0x3F));
    }
}

// 栈弹出一个字节的数据
static void* lept_context_pop(lept_context* c,size_t size)
{
    assert(c->top>=size);
    return c->stack+(c->top-=size);
}

static void lept_parse_whilespace(lept_context* c)
{
    const char*p=c->json;
    while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')
        p++;
    
    c->json=p;
}

static int lept_parse_literal(lept_context* c,lept_value*v,const char* literal,lept_type type)
{
    size_t i;
    EXPECT(c,literal[0]);
    for(i=0;literal[i+1];++i)
    {
        if(c->json[i]!=literal[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json+=i;
    v->type=type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c,lept_value* v)
{
    const char* p=c->json;
    if(*p=='-')
        ++p;
    if(*p=='0')
        ++p;
    else
    {
        if(!ISDIGIT1TO9(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT(*p);++p);
    }
    if(*p=='.')
    {
        ++p;
        if(!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT(*p);++p);
    }
    if(*p=='e'||*p=='E')
    {
        ++p;
        if(*p=='+'||*p=='-')
            ++p;
        if(!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT(*p);++p);
    }
    errno=0;
    v->m_union.n=strtod(c->json, nullptr);

    if(errno==ERANGE&&(v->m_union.n==HUGE_VAL||v->m_union.n==-HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type=LEPT_NUMBER;
    c->json=p;
    return LEPT_PARSE_OK;
}

static int lept_parse_string(lept_context* c,lept_value* v)
{
    size_t head=c->top,len;
    const char* p;
    unsigned u,u2;
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
                    case 'u':
                    {
                        if(!(p=lept_parse_hex4(p,&u)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);

                        if(u>=0xD800 && u<= 0xDBFF)
                        {
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
                    }
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            }
            case '\0':
            {
                c->top=head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            }
            default:
            {
                if((unsigned char)ch<0x20)
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                PUTC(c,ch);
            }
        }
    }
}

static int lept_parse_value(lept_context*c,lept_value* v)
{
    switch(*c->json)
    {
        case 'n':return lept_parse_literal(c,v,"null",LEPT_NULL);
        case 't':return lept_parse_literal(c,v,"true",LEPT_TRUE);
        case 'f':return lept_parse_literal(c,v,"false",LEPT_FALSE);
        case '"':return lept_parse_string(c,v);
        case '\0':return LEPT_PARSE_EXPECT_VALUE;
        default:return lept_parse_number(c,v);
    }
}

int lept_parse(lept_value* v,const char* json)
{
    lept_context c;
    int ret;
    assert(v!=nullptr);
    c.json=json;
    c.stack=nullptr;
    c.size=c.top=0;
    lept_init(v);
    lept_parse_whilespace(&c);

    if((ret=lept_parse_value(&c,v))==LEPT_PARSE_OK)
    {
        // 解析完一个值之后还有其他的值
        lept_parse_whilespace(&c);
        if(*c.json!='\0')
        {
            v->type=LEPT_NULL;
            ret=LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top==0);
    free(c.stack);
    return ret;
}

void lept_free(lept_value* v)
{
    assert(v!=nullptr);
    if(v->type==LEPT_STRING)
        free(v->m_union.m_str.s);
    v->type=LEPT_NULL;
}

lept_type lept_get_type(const lept_value* v)
{
    assert(v!=nullptr);
    return v->type;
}

int lept_get_boolean(const lept_value* v)
{
    assert(v!=nullptr&&(v->type==LEPT_TRUE||v->type==LEPT_FALSE));
    return v->type==LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b)
{
    lept_free(v);
    v->type=b?LEPT_TRUE:LEPT_FALSE;
}

double lept_get_number(const lept_value* v)
{
    assert(v!=nullptr&&v->type==LEPT_NUMBER);
    return v->m_union.n;
}

void lept_set_number(lept_value* v,double n)
{
    lept_free(v);
    v->m_union.n=n;
    v->type=LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v)
{
    assert(v != nullptr && v->type == LEPT_STRING);
    return v->m_union.m_str.s;
}

size_t lept_get_string_length(const lept_value* v)
{
    assert(v!=nullptr&&v->type==LEPT_STRING);
    return v->m_union.m_str.len;
}

void lept_set_string(lept_value* v,const char* s,size_t len)
{
    // 这里的(s!=nullptr||len==0)主要是为了避免当s=nullptr而len!=0的情况
    // 这意味着试图复制空指针的内容，非常不安全
    assert(v!=nullptr&&(s!=nullptr||len==0));
    lept_free(v);
    v->m_union.m_str.s=(char*)malloc(len+1);
    memcpy(v->m_union.m_str.s,s,len);
    v->m_union.m_str.s[len]='\0';
    v->m_union.m_str.len=len;
    v->type=LEPT_STRING;
}