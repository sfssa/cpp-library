#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "leptjson.h"

#define EXPECT(c, ch) \
    do { \
        assert(*c->json==(ch)); \
        c->json++; \
    } while (0)

#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')

#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')

struct lept_context
{
    const char* json;
};

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
    v->n=strtod(c->json,nullptr);
    if(errno==ERANGE&&(v->n==HUGE_VAL||v->n==-HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type=LEPT_NUMBER;
    c->json=p;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context*c,lept_value* v)
{
    switch(*c->json)
    {
        case 'n':return lept_parse_literal(c,v,"null",LEPT_NULL);
        case 't':return lept_parse_literal(c,v,"true",LEPT_TRUE);
        case 'f':return lept_parse_literal(c,v,"false",LEPT_FALSE);
        case '\0':return LEPT_PARSE_EXPECT_VALUE;
        default:return lept_parse_number(c,v);
    }
}

int lept_parse(lept_value* v,const char* json)
{
    lept_context c;
    assert(v!=nullptr);
    c.json=json;
    v->type=LEPT_NULL;
    lept_parse_whilespace(&c);

    int ret=-1;
    if((ret=lept_parse_value(&c,v))==LEPT_PARSE_OK)
    {
        // 解析完一个值之后还有其他的值
        lept_parse_whilespace(&c);
        if(*c.json!='\0')
            ret=LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v)
{
    assert(v!=nullptr);
    return v->type;
}

double lept_get_number(const lept_value* v)
{
    assert(v!=nullptr&&v->type==LEPT_NUMBER);
    return v->n;
}