#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <assert.h>  
#include <errno.h>   
#include <math.h>    
#include <stdlib.h>  
#include <stdio.h>
#include <string.h> 
#include "leptjson.h" 

const int LEPT_PARSE_STACK_INIT_SIZE=256;
const int LEPT_PARSE_STRINGIFY_INIT_SIZE=256;
const int LEPT_PARSE_OBJECT_INIT_SIZE=128;

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)         do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)
#define PUTS(c, s, len)     memcpy(lept_context_push(c, len), s, len)

struct lept_context{
    const char* json;
    char* stack;
    size_t size, top;
};

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

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->m_union.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->m_union.n == HUGE_VAL || v->m_union.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static const char* lept_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
        else return NULL;
    }
    return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u) {
    if (u <= 0x7F) 
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

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
                *str = (char*)lept_context_pop(c, *len);
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

static int lept_parse_value(lept_context* c, lept_value* v);

static int lept_parse_array(lept_context* c, lept_value* v) {
    size_t i, size = 0;
    int ret;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = LEPT_ARRAY;
        v->m_union.m_array.len = 0;
        v->m_union.m_array.e = NULL;
        return LEPT_PARSE_OK;
    }
    for (;;) {
        lept_value e;
        lept_init(&e);
        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK)
            break;
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        size++;
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if (*c->json == ']') {
            c->json++;
            v->type = LEPT_ARRAY;
            v->m_union.m_array.len = size;
            size *= sizeof(lept_value);
            memcpy(v->m_union.m_array.e = (lept_value*)malloc(size), lept_context_pop(c, size), size);
            return LEPT_PARSE_OK;
        }
        else {
            ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    /* Pop and free values on the stack */
    for (i = 0; i < size; i++)
        lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
    return ret;
}

static int lept_parse_object(lept_context* c, lept_value* v) {
    size_t i, size;
    lept_member m;
    int ret;
    EXPECT(c, '{');
    lept_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = LEPT_OBJECT;
        v->m_union.m_object.m = 0;
        v->m_union.m_object.size = 0;
        return LEPT_PARSE_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;) {
        char* str;
        lept_init(&m.v);
        /* parse key */
        if (*c->json != '"') {
            ret = LEPT_PARSE_MISS_KEY;
            break;
        }
        if ((ret = lept_parse_string_raw(c, &str, &m.klen)) != LEPT_PARSE_OK)
            break;
        memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
        m.k[m.klen] = '\0';
        /* parse ws colon ws */
        lept_parse_whitespace(c);
        if (*c->json != ':') {
            ret = LEPT_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        lept_parse_whitespace(c);
        /* parse value */
        if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK)
            break;
        memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
        size++;
        m.k = NULL; /* ownership is transferred to member on stack */
        /* parse ws [comma | right-curly-brace] ws */
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if (*c->json == '}') {
            size_t s = sizeof(lept_member) * size;
            c->json++;
            v->type = LEPT_OBJECT;
            v->m_union.m_object.size = size;
            memcpy(v->m_union.m_object.m = (lept_member*)malloc(s), lept_context_pop(c, s), s);
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

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '"':  return lept_parse_string(c, v);
        case '[':  return lept_parse_array(c, v);
        case '{':  return lept_parse_object(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

#if 0
// Unoptimized
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
#else
static void lept_stringify_string(lept_context* c, const char* s, size_t len) {
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    size_t i, size;
    char* head, *p;
    assert(s != NULL);
    p = head = (char*)lept_context_push(c, size = len * 6 + 2); /* "\u00xx..." */
    *p++ = '"';
    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i];
        switch (ch) {
            case '\"': *p++ = '\\'; *p++ = '\"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
                if (ch < 0x20) {
                    *p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
                    *p++ = hex_digits[ch >> 4];
                    *p++ = hex_digits[ch & 15];
                }
                else
                    *p++ = s[i];
        }
    }
    *p++ = '"';
    c->top -= size - (p - head);
}
#endif

static void lept_stringify_value(lept_context* c, const lept_value* v) {
    size_t i;
    switch (v->type) {
        case LEPT_NULL:   PUTS(c, "null",  4); break;
        case LEPT_FALSE:  PUTS(c, "false", 5); break;
        case LEPT_TRUE:   PUTS(c, "true",  4); break;
        case LEPT_NUMBER: c->top -= 32 - sprintf((char*)lept_context_push(c, 32), "%.17g", v->m_union.n); break;
        case LEPT_STRING: lept_stringify_string(c, v->m_union.m_str.s, v->m_union.m_str.len); break;
        case LEPT_ARRAY:
            PUTC(c, '[');
            for (i = 0; i < v->m_union.m_array.len; i++) {
                if (i > 0)
                    PUTC(c, ',');
                lept_stringify_value(c, &v->m_union.m_array.e[i]);
            }
            PUTC(c, ']');
            break;
        case LEPT_OBJECT:
            PUTC(c, '{');
            for (i = 0; i < v->m_union.m_object.size; i++) {
                if (i > 0)
                    PUTC(c, ',');
                lept_stringify_string(c, v->m_union.m_object.m[i].k, v->m_union.m_object.m[i].klen);
                PUTC(c, ':');
                lept_stringify_value(c, &v->m_union.m_object.m[i].v);
            }
            PUTC(c, '}');
            break;
        default: assert(0 && "invalid type");
    }
}

char* lept_stringify(const lept_value* v, size_t* length) {
    lept_context c;
    assert(v != NULL);
    c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);
    c.top = 0;
    lept_stringify_value(&c, v);
    if (length)
        *length = c.top;
    PUTC(&c, '\0');
    return c.stack;
}

void lept_free(lept_value* v) {
    size_t i;
    assert(v != NULL);
    switch (v->type) {
        case LEPT_STRING:
            free(v->m_union.m_str.s);
            break;
        case LEPT_ARRAY:
            for (i = 0; i < v->m_union.m_array.len; i++)
                lept_free(&v->m_union.m_array.e[i]);
            free(v->m_union.m_array.e);
            break;
        case LEPT_OBJECT:
            for (i = 0; i < v->m_union.m_object.size; i++) {
                free(v->m_union.m_object.m[i].k);
                lept_free(&v->m_union.m_object.m[i].v);
            }
            free(v->m_union.m_object.m);
            break;
        default: break;
    }
    v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->m_union.n;
}

void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->m_union.n = n;
    v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->m_union.m_str.s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->m_union.m_str.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->m_union.m_str.s = (char*)malloc(len + 1);
    memcpy(v->m_union.m_str.s, s, len);
    v->m_union.m_str.s[len] = '\0';
    v->m_union.m_str.len = len;
    v->type = LEPT_STRING;
}

size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->m_union.m_array.len;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    assert(index < v->m_union.m_array.len);
    return &v->m_union.m_array.e[index];
}

size_t lept_get_object_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    return v->m_union.m_object.size;
}

const char* lept_get_object_key(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->m_union.m_object.size);
    return v->m_union.m_object.m[index].k;
}

size_t lept_get_object_key_length(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->m_union.m_object.size);
    return v->m_union.m_object.m[index].klen;
}

lept_value* lept_get_object_value(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->m_union.m_object.size);
    return &v->m_union.m_object.m[index].v;
}

void lept_copy(lept_value* dst, const lept_value* src) {
    assert(src != NULL && dst != NULL && src != dst);

    // 清空目标值
    lept_free(dst);

    // 根据源值的类型复制数据到目标值
    switch (src->type) {
        case LEPT_NULL:
            break;
        case LEPT_FALSE:
        case LEPT_TRUE:
            dst->type = src->type;
            break;
        case LEPT_NUMBER:
            dst->type = LEPT_NUMBER;
            dst->m_union.n = src->m_union.n;
            break;
        case LEPT_STRING:
            dst->type = LEPT_STRING;
            lept_set_string(dst, src->m_union.m_str.s, src->m_union.m_str.len);
            break;
        case LEPT_ARRAY:
            dst->type = LEPT_ARRAY;
            dst->m_union.m_array.len = src->m_union.m_array.len;
            dst->m_union.m_array.e = (lept_value*)malloc(dst->m_union.m_array.len * sizeof(lept_value));
            for (size_t i = 0; i < dst->m_union.m_array.len; i++) {
                lept_init(&dst->m_union.m_array.e[i]);
                lept_copy(&dst->m_union.m_array.e[i], &src->m_union.m_array.e[i]);
            }
            break;
        case LEPT_OBJECT:
            dst->type = LEPT_OBJECT;
            dst->m_union.m_object.size = src->m_union.m_object.size;
            dst->m_union.m_object.m = (lept_member*)malloc(dst->m_union.m_object.size * sizeof(lept_member));
            for (size_t i = 0; i < dst->m_union.m_object.size; i++) {
                dst->m_union.m_object.m[i].k = (char*)malloc(src->m_union.m_object.m[i].klen + 1);
                memcpy(dst->m_union.m_object.m[i].k, src->m_union.m_object.m[i].k, src->m_union.m_object.m[i].klen);
                dst->m_union.m_object.m[i].k[src->m_union.m_object.m[i].klen] = '\0';
                lept_init(&dst->m_union.m_object.m[i].v);
                lept_copy(&dst->m_union.m_object.m[i].v, &src->m_union.m_object.m[i].v);
            }
            break;
    }
}


void lept_move(lept_value* dst, lept_value* src) {
    assert(dst != NULL && src != NULL && src != dst);
    lept_free(dst);
    memcpy(dst, src, sizeof(lept_value));
    lept_init(src);
}

void lept_swap(lept_value* lhs, lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs) {
        lept_value temp;
        memcpy(&temp, lhs, sizeof(lept_value));
        memcpy(lhs,   rhs, sizeof(lept_value));
        memcpy(rhs, &temp, sizeof(lept_value));
    }
}

int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);

    if (lhs->type != rhs->type) {
        return 0;  // 类型不同，直接返回不相等
    }

    switch (lhs->type) {
        case LEPT_NULL:
        case LEPT_FALSE:
        case LEPT_TRUE:
            return 1;  // 这些类型只要类型相同就相等

        case LEPT_NUMBER:
            return lhs->m_union.n == rhs->m_union.n;  // 直接比较数值

        case LEPT_STRING:
            if (lhs->m_union.m_str.len != rhs->m_union.m_str.len) {
                return 0;  // 字符串长度不同，不相等
            }
            return memcmp(lhs->m_union.m_str.s, rhs->m_union.m_str.s, lhs->m_union.m_str.len) == 0;

        case LEPT_ARRAY:
            if (lhs->m_union.m_array.len != rhs->m_union.m_array.len) {
                return 0;  // 数组长度不同，不相等
            }
            for (size_t i = 0; i < lhs->m_union.m_array.len; i++) {
                if (!lept_is_equal(&lhs->m_union.m_array.e[i], &rhs->m_union.m_array.e[i])) {
                    return 0;  // 递归比较数组元素，有一个不相等就返回不相等
                }
            }
            return 1;  // 所有数组元素都相等

        case LEPT_OBJECT:
            if (lhs->m_union.m_object.size != rhs->m_union.m_object.size) {
                return 0;  // 对象成员个数不同，不相等
            }
            for (size_t i = 0; i < lhs->m_union.m_object.size; i++) {
                if (lhs->m_union.m_object.m[i].klen != rhs->m_union.m_object.m[i].klen ||
                    memcmp(lhs->m_union.m_object.m[i].k, rhs->m_union.m_object.m[i].k, lhs->m_union.m_object.m[i].klen) != 0) {
                    return 0;  // 逐个比较对象成员的键，有一个不相等就返回不相等
                }
                if (!lept_is_equal(&lhs->m_union.m_object.m[i].v, &rhs->m_union.m_object.m[i].v)) {
                    return 0;  // 递归比较对象成员的值，有一个不相等就返回不相等
                }
            }
            return 1;  // 所有对象成员都相等
    }

    return 0;  // 未知类型，默认不相等
}


void lept_set_array(lept_value* v, size_t capacity) {
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_ARRAY;
    v->m_union.m_array.len = 0;
    v->m_union.m_array.capacity = capacity;
    v->m_union.m_array.e = capacity > 0 ? (lept_value*)malloc(capacity * sizeof(lept_value)) : NULL;
}

size_t lept_get_array_capacity(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->m_union.m_array.capacity;
}
void lept_reserve_array(lept_value* v, size_t capacity) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->m_union.m_array.capacity < capacity) {
        v->m_union.m_array.capacity = capacity;
        v->m_union.m_array.e = (lept_value*)realloc(v->m_union.m_array.e, capacity * sizeof(lept_value));
    }
}

void lept_shrink_array(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->m_union.m_array.capacity > v->m_union.m_array.len) {
        v->m_union.m_array.capacity = v->m_union.m_array.len;
        v->m_union.m_array.e = (lept_value*)realloc(v->m_union.m_array.e, v->m_union.m_array.capacity * sizeof(lept_value));
    }
}

void lept_clear_array(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    lept_erase_array_element(v, 0, v->m_union.m_array.len);
}

lept_value* lept_pushback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->m_union.m_array.len == v->m_union.m_array.capacity)
        lept_reserve_array(v, v->m_union.m_array.capacity == 0 ? 1 : v->m_union.m_array.capacity * 2);
    lept_init(&v->m_union.m_array.e[v->m_union.m_array.len]);
    return &v->m_union.m_array.e[v->m_union.m_array.len++];
}

void lept_popback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY && v->m_union.m_array.len > 0);
    lept_free(&v->m_union.m_array.e[--v->m_union.m_array.len]);
}

lept_value* lept_insert_array_element(lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY);  // 确保传入的值是数组类型

    if (index > v->m_union.m_array.len) {
        return NULL;  // 索引超出数组范围，无法插入
    }

    if (v->m_union.m_array.len + 1 > v->m_union.m_array.len) {
        size_t new_size = (v->m_union.m_array.len == 0) ? 8 : v->m_union.m_array.len * 2;
        v->m_union.m_array.e = (lept_value*)realloc(v->m_union.m_array.e, new_size * sizeof(lept_value));
        v->m_union.m_array.len = new_size;
    }

    for (size_t i = v->m_union.m_array.len; i > index; i--) {
        lept_copy(&v->m_union.m_array.e[i], &v->m_union.m_array.e[i - 1]);
    }

    lept_init(&v->m_union.m_array.e[index]);
    v->m_union.m_array.len++;

    return &v->m_union.m_array.e[index];
}


void lept_erase_array_element(lept_value* v, size_t index, size_t count) {
    assert(v != NULL && v->type == LEPT_ARRAY);  // 确保传入的值是数组类型
    assert(index < v->m_union.m_array.len);  // 确保删除的索引在合法范围内

    // 计算实际删除的元素数量（不能大于数组长度）
    size_t actual_count = (index + count <= v->m_union.m_array.len) ? count : v->m_union.m_array.len - index;

    for (size_t i = index; i < v->m_union.m_array.len - actual_count; i++) {
        lept_free(&v->m_union.m_array.e[i]);
        lept_copy(&v->m_union.m_array.e[i], &v->m_union.m_array.e[i + actual_count]);
    }

    v->m_union.m_array.len -= actual_count;

    // 清空多余的元素，释放内存
    for (size_t i = v->m_union.m_array.len; i < v->m_union.m_array.len + actual_count; i++) {
        lept_free(&v->m_union.m_array.e[i]);
        memset(&v->m_union.m_array.e[i], 0, sizeof(lept_value));
    }

    // 如果删除后数组变得很小，可以减小内存占用
    if (v->m_union.m_array.len > 0 && v->m_union.m_array.len * 4 <= v->m_union.m_array.len) {
        size_t new_size = (v->m_union.m_array.len == 0) ? 0 : v->m_union.m_array.len * 2;
        v->m_union.m_array.e = (lept_value*)realloc(v->m_union.m_array.e, new_size * sizeof(lept_value));
        v->m_union.m_array.len = new_size;
    }
}

void lept_set_object(lept_value* v, size_t capacity) {
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_OBJECT;
    v->m_union.m_object.size = 0;
    v->m_union.m_object.capacity = capacity;
    v->m_union.m_object.m = capacity > 0 ? (lept_member*)malloc(capacity * sizeof(lept_member)) : NULL;
}


size_t lept_get_object_capacity(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    return v->m_union.m_object.capacity;
}

void lept_reserve_object(lept_value* v, size_t capacity) {
    assert(v != NULL && v->type == LEPT_OBJECT);  // 确保传入的值是对象类型

    if (v->m_union.m_object.size >= capacity) {
        return;  // 如果已经有足够的容量，无需改变
    }

    if (v->m_union.m_object.size == 0) {
        v->m_union.m_object.size = LEPT_PARSE_OBJECT_INIT_SIZE;
    }

    while (v->m_union.m_object.size < capacity) {
        v->m_union.m_object.size += v->m_union.m_object.size >> 1;  // 扩大为原来的1.5倍
    }

    v->m_union.m_object.m = (lept_member*)realloc(v->m_union.m_object.m, v->m_union.m_object.size * sizeof(lept_member));
}


void lept_shrink_object(lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);

    size_t size = v->m_union.m_object.size;
    if (size <= LEPT_PARSE_OBJECT_INIT_SIZE) {
        return;  // 如果容量已经是初始容量，无法再缩小
    }

    // 缩小容量为初始容量的 1.5 倍
    size_t new_size = LEPT_PARSE_OBJECT_INIT_SIZE;
    while (new_size * 1.5 < size) {
        new_size *= 1.5;
    }

    // 释放多余的成员内存
    size_t excess = size - new_size;
    for (size_t i = 0; i < excess; i++) {
        free(v->m_union.m_object.m[size - i - 1].k);
    }

    // 重新分配成员数组的内存
    v->m_union.m_object.m = (lept_member*)realloc(v->m_union.m_object.m, new_size * sizeof(lept_member));
    v->m_union.m_object.size = new_size;
}

void lept_clear_object(lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);

    for (size_t i = 0; i < v->m_union.m_object.size; i++) {
        free(v->m_union.m_object.m[i].k);
        lept_free(&v->m_union.m_object.m[i].v);
    }

    free(v->m_union.m_object.m);
    v->m_union.m_object.m = NULL;
    v->m_union.m_object.size = 0;
}


lept_value* lept_get_object_value(lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->m_union.m_object.size);
    return &v->m_union.m_object.m[index].v;
}

size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen) {
    size_t i;
    assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
    for (i = 0; i < v->m_union.m_object.size; i++)
        if (v->m_union.m_object.m[i].klen == klen && memcmp(v->m_union.m_object.m[i].k, key, klen) == 0)
            return i;
    return LEPT_KEY_NOT_EXIST;
}

lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen) {
    size_t index = lept_find_object_index(v, key, klen);
    return index != LEPT_KEY_NOT_EXIST ? &v->m_union.m_object.m[index].v : NULL;
}

lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen) {
    assert(v != NULL && v->type == LEPT_OBJECT && key != NULL && klen > 0);

    // 首先检查是否存在相同键的成员
    for (size_t i = 0; i < v->m_union.m_object.size; i++) {
        if (klen == v->m_union.m_object.m[i].klen && memcmp(key, v->m_union.m_object.m[i].k, klen) == 0) {
            // 如果找到相同键的成员，返回该成员的值
            return &v->m_union.m_object.m[i].v;
        }
    }

    // 若没有找到相同键的成员，添加一个新成员
    if (v->m_union.m_object.size == 0) {
        // 若对象还没有成员，分配初始容量
        lept_reserve_object(v, LEPT_PARSE_OBJECT_INIT_SIZE);
    } else if (v->m_union.m_object.size >= v->m_union.m_object.capacity) {
        // 若对象已满，扩展容量
        lept_reserve_object(v, v->m_union.m_object.capacity * 2);
    }

    // 复制键
    char* new_key = (char*)malloc(klen + 1);
    memcpy(new_key, key, klen);
    new_key[klen] = '\0';

    // 复制值
    lept_value new_value;
    lept_init(&new_value);

    // 添加新成员
    v->m_union.m_object.m[v->m_union.m_object.size].k = new_key;
    v->m_union.m_object.m[v->m_union.m_object.size].klen = klen;
    lept_copy(&v->m_union.m_object.m[v->m_union.m_object.size].v, &new_value);
    v->m_union.m_object.size++;

    return &v->m_union.m_object.m[v->m_union.m_object.size - 1].v;
}

void lept_remove_object_value(lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT && index < v->m_union.m_object.size);

    // 释放成员的键和值
    free(v->m_union.m_object.m[index].k);
    lept_free(&v->m_union.m_object.m[index].v);

    // 移动后续成员以覆盖被删除的成员
    for (size_t i = index; i < v->m_union.m_object.size - 1; i++) {
        v->m_union.m_object.m[i] = v->m_union.m_object.m[i + 1];
    }

    v->m_union.m_object.size--;
}
