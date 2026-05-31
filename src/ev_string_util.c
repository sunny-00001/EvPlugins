#include "ev_string_util.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

uintptr_t ev_string_length(const char *s)
{
    if (!s) return 0;
    return (uintptr_t)strlen(s);
}

int ev_string_compare(const char *a, const char *b)
{
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return strcmp(a, b);
}

int ev_string_compare_n(const char *a, const char *b, uintptr_t n)
{
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return strncmp(a, b, (size_t)n);
}

int ev_string_copy(char *dst, const char *src, uintptr_t dst_size)
{
    if (!dst || dst_size == 0) return 0;
    if (!src)
    {
        dst[0] = 0;
        return 0;
    }
    strncpy(dst, src, (size_t)(dst_size - 1));
    dst[dst_size - 1] = 0;
    return 1;
}

char *ev_string_duplicate(const char *s)
{
    uintptr_t len;
    char *dup;
    if (!s) return NULL;
    len = ev_string_length(s);
    dup = (char *)malloc(len + 1);
    if (!dup) return NULL;
    memcpy(dup, s, len + 1);
    return dup;
}

int ev_string_to_int(const char *s, int default_value)
{
    int value;
    char *end;
    if (!s || *s == 0) return default_value;
    value = (int)strtol(s, &end, 10);
    if (end == s) return default_value;
    return value;
}

void ev_int_to_string(int value, char *buf, uintptr_t buf_size)
{
    if (!buf || buf_size == 0) return;
    _snprintf(buf, buf_size - 1, "%d", value);
    buf[buf_size - 1] = 0;
}

int ev_string_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix) return 0;
    while (*prefix)
    {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

int ev_string_ends_with(const char *str, const char *suffix)
{
    uintptr_t str_len;
    uintptr_t suffix_len;
    if (!str || !suffix) return 0;
    str_len = ev_string_length(str);
    suffix_len = ev_string_length(suffix);
    if (suffix_len > str_len) return 0;
    return ev_string_compare(str + str_len - suffix_len, suffix) == 0;
}

void ev_string_trim(char *s)
{
    char *start;
    char *end;
    if (!s) return;
    start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n'))
        start++;
    end = start + ev_string_length(start);
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
        end--;
    *end = 0;
    if (start != s) memmove(s, start, end - start + 1);
}

uintptr_t ev_string_url_encode(const char *src, char *dst, uintptr_t dst_size)
{
    uintptr_t si;
    uintptr_t di;
    static const char *hex = "0123456789ABCDEF";
    if (!src || !dst || dst_size == 0) return 0;
    si = 0;
    di = 0;
    while (src[si] && di < dst_size - 4)
    {
        unsigned char c = (unsigned char)src[si];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            dst[di++] = c;
        }
        else
        {
            dst[di++] = '%';
            dst[di++] = hex[c >> 4];
            dst[di++] = hex[c & 0x0F];
        }
        si++;
    }
    dst[di] = 0;
    return di;
}

uintptr_t ev_string_url_decode(const char *src, char *dst, uintptr_t dst_size)
{
    uintptr_t si;
    uintptr_t di;
    if (!src || !dst || dst_size == 0) return 0;
    si = 0;
    di = 0;
    while (src[si] && di < dst_size - 1)
    {
        if (src[si] == '%' && src[si + 1] && src[si + 2])
        {
            char hex[3];
            hex[0] = src[si + 1];
            hex[1] = src[si + 2];
            hex[2] = 0;
            dst[di++] = (char)strtol(hex, NULL, 16);
            si += 3;
        }
        else if (src[si] == '+')
        {
            dst[di++] = ' ';
            si++;
        }
        else
        {
            dst[di++] = src[si++];
        }
    }
    dst[di] = 0;
    return di;
}
