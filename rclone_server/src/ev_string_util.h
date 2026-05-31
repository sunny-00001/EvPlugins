#ifndef EV_STRING_UTIL_H
#define EV_STRING_UTIL_H

#include <windows.h>
#include "everything_plugin.h"

uintptr_t ev_string_length(const char *s);
int ev_string_compare(const char *a, const char *b);
int ev_string_compare_n(const char *a, const char *b, uintptr_t n);
int ev_string_copy(char *dst, const char *src, uintptr_t dst_size);
char *ev_string_duplicate(const char *s);
int ev_string_to_int(const char *s, int default_value);
void ev_int_to_string(int value, char *buf, uintptr_t buf_size);
int ev_string_starts_with(const char *str, const char *prefix);
int ev_string_ends_with(const char *str, const char *suffix);
void ev_string_trim(char *s);
uintptr_t ev_string_url_encode(const char *src, char *dst, uintptr_t dst_size);
uintptr_t ev_string_url_decode(const char *src, char *dst, uintptr_t dst_size);

#endif
