#include "ev_json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static ev_json_value_t *ev_json_alloc(ev_json_type_t type)
{
    ev_json_value_t *val = (ev_json_value_t *)calloc(1, sizeof(ev_json_value_t));
    if (val) val->type = type;
    return val;
}

static uintptr_t ev_json_skip_ws(const char *json, uintptr_t len, uintptr_t pos)
{
    while (pos < len)
    {
        char c = json[pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            pos++;
        else
            break;
    }
    return pos;
}

static ev_json_value_t *ev_json_parse_value(const char *json, uintptr_t len, uintptr_t *pos);

static uintptr_t ev_json_parse_string_raw(const char *json, uintptr_t len, uintptr_t pos, char **out_str)
{
    uintptr_t start;
    uintptr_t di;
    char *buf;
    if (pos >= len || json[pos] != '"') return pos;
    pos++;
    start = pos;
    buf = (char *)malloc(len - start + 1);
    if (!buf) return pos;
    di = 0;
    while (pos < len && json[pos] != '"')
    {
        if (json[pos] == '\\' && pos + 1 < len)
        {
            pos++;
            switch (json[pos])
            {
            case '"':  buf[di++] = '"';  break;
            case '\\': buf[di++] = '\\'; break;
            case '/':  buf[di++] = '/';  break;
            case 'b':  buf[di++] = '\b'; break;
            case 'f':  buf[di++] = '\f'; break;
            case 'n':  buf[di++] = '\n'; break;
            case 'r':  buf[di++] = '\r'; break;
            case 't':  buf[di++] = '\t'; break;
            case 'u':
                if (pos + 4 < len)
                {
                    unsigned int cp = 0;
                    int i;
                    for (i = 1; i <= 4; i++)
                    {
                        char hc = json[pos + i];
                        cp <<= 4;
                        if (hc >= '0' && hc <= '9') cp |= hc - '0';
                        else if (hc >= 'a' && hc <= 'f') cp |= hc - 'a' + 10;
                        else if (hc >= 'A' && hc <= 'F') cp |= hc - 'A' + 10;
                    }
                    pos += 4;
                    if (cp < 0x80)
                    {
                        buf[di++] = (char)cp;
                    }
                    else if (cp < 0x800)
                    {
                        buf[di++] = (char)(0xC0 | (cp >> 6));
                        buf[di++] = (char)(0x80 | (cp & 0x3F));
                    }
                    else
                    {
                        buf[di++] = (char)(0xE0 | (cp >> 12));
                        buf[di++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                        buf[di++] = (char)(0x80 | (cp & 0x3F));
                    }
                }
                break;
            default:
                buf[di++] = json[pos];
                break;
            }
        }
        else
        {
            buf[di++] = json[pos];
        }
        pos++;
    }
    buf[di] = 0;
    if (pos < len && json[pos] == '"') pos++;
    *out_str = buf;
    return pos;
}

static ev_json_value_t *ev_json_parse_string(const char *json, uintptr_t len, uintptr_t *pos)
{
    char *str;
    ev_json_value_t *val;
    uintptr_t new_pos = ev_json_parse_string_raw(json, len, *pos, &str);
    if (!str) return NULL;
    val = ev_json_alloc(EV_JSON_TYPE_STRING);
    if (!val) { free(str); return NULL; }
    val->u.string_val = str;
    *pos = new_pos;
    return val;
}

static ev_json_value_t *ev_json_parse_number(const char *json, uintptr_t len, uintptr_t *pos)
{
    char buf[64];
    uintptr_t i = 0;
    ev_json_value_t *val;
    while (*pos < len && i < sizeof(buf) - 1)
    {
        char c = json[*pos];
        if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E')
        {
            buf[i++] = c;
            (*pos)++;
        }
        else
        {
            break;
        }
    }
    buf[i] = 0;
    val = ev_json_alloc(EV_JSON_TYPE_NUMBER);
    if (!val) return NULL;
    val->u.number_val = atof(buf);
    return val;
}

static ev_json_value_t *ev_json_parse_array(const char *json, uintptr_t len, uintptr_t *pos)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_ARRAY);
    uintptr_t capacity = 16;
    if (!val) return NULL;
    val->u.array.items = (ev_json_value_t **)malloc(capacity * sizeof(ev_json_value_t *));
    if (!val->u.array.items) { free(val); return NULL; }
    val->u.array.count = 0;
    (*pos)++;
    *pos = ev_json_skip_ws(json, len, *pos);
    if (*pos < len && json[*pos] == ']') { (*pos)++; return val; }
    for (;;)
    {
        ev_json_value_t *item;
        *pos = ev_json_skip_ws(json, len, *pos);
        item = ev_json_parse_value(json, len, pos);
        if (!item) { ev_json_value_free(val); return NULL; }
        if (val->u.array.count >= capacity)
        {
            capacity *= 2;
            val->u.array.items = (ev_json_value_t **)realloc(val->u.array.items, capacity * sizeof(ev_json_value_t *));
            if (!val->u.array.items) { ev_json_value_free(item); ev_json_value_free(val); return NULL; }
        }
        val->u.array.items[val->u.array.count++] = item;
        *pos = ev_json_skip_ws(json, len, *pos);
        if (*pos < len && json[*pos] == ',') { (*pos)++; continue; }
        if (*pos < len && json[*pos] == ']') { (*pos)++; break; }
        break;
    }
    return val;
}

static ev_json_value_t *ev_json_parse_object(const char *json, uintptr_t len, uintptr_t *pos)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_OBJECT);
    uintptr_t capacity = 16;
    if (!val) return NULL;
    val->u.object.keys = (char **)malloc(capacity * sizeof(char *));
    val->u.object.values = (ev_json_value_t **)malloc(capacity * sizeof(ev_json_value_t *));
    if (!val->u.object.keys || !val->u.object.values)
    {
        free(val->u.object.keys);
        free(val->u.object.values);
        free(val);
        return NULL;
    }
    val->u.object.count = 0;
    (*pos)++;
    *pos = ev_json_skip_ws(json, len, *pos);
    if (*pos < len && json[*pos] == '}') { (*pos)++; return val; }
    for (;;)
    {
        char *key;
        ev_json_value_t *v;
        *pos = ev_json_skip_ws(json, len, *pos);
        if (*pos >= len || json[*pos] != '"') { ev_json_value_free(val); return NULL; }
        *pos = ev_json_parse_string_raw(json, len, *pos, &key);
        if (!key) { ev_json_value_free(val); return NULL; }
        *pos = ev_json_skip_ws(json, len, *pos);
        if (*pos >= len || json[*pos] != ':') { free(key); ev_json_value_free(val); return NULL; }
        (*pos)++;
        *pos = ev_json_skip_ws(json, len, *pos);
        v = ev_json_parse_value(json, len, pos);
        if (!v) { free(key); ev_json_value_free(val); return NULL; }
        if (val->u.object.count >= capacity)
        {
            capacity *= 2;
            val->u.object.keys = (char **)realloc(val->u.object.keys, capacity * sizeof(char *));
            val->u.object.values = (ev_json_value_t **)realloc(val->u.object.values, capacity * sizeof(ev_json_value_t *));
            if (!val->u.object.keys || !val->u.object.values)
            {
                free(key);
                ev_json_value_free(v);
                ev_json_value_free(val);
                return NULL;
            }
        }
        val->u.object.keys[val->u.object.count] = key;
        val->u.object.values[val->u.object.count] = v;
        val->u.object.count++;
        *pos = ev_json_skip_ws(json, len, *pos);
        if (*pos < len && json[*pos] == ',') { (*pos)++; continue; }
        if (*pos < len && json[*pos] == '}') { (*pos)++; break; }
        break;
    }
    return val;
}

static ev_json_value_t *ev_json_parse_value(const char *json, uintptr_t len, uintptr_t *pos)
{
    *pos = ev_json_skip_ws(json, len, *pos);
    if (*pos >= len) return NULL;
    switch (json[*pos])
    {
    case '"': return ev_json_parse_string(json, len, pos);
    case '{':  return ev_json_parse_object(json, len, pos);
    case '[': return ev_json_parse_array(json, len, pos);
    case 't':
        if (*pos + 3 < len && json[*pos+1] == 'r' && json[*pos+2] == 'u' && json[*pos+3] == 'e')
        {
            ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_BOOL);
            if (val) { val->u.bool_val = 1; *pos += 4; }
            return val;
        }
        return NULL;
    case 'f':
        if (*pos + 4 < len && json[*pos+1] == 'a' && json[*pos+2] == 'l' && json[*pos+3] == 's' && json[*pos+4] == 'e')
        {
            ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_BOOL);
            if (val) { val->u.bool_val = 0; *pos += 5; }
            return val;
        }
        return NULL;
    case 'n':
        if (*pos + 3 < len && json[*pos+1] == 'u' && json[*pos+2] == 'l' && json[*pos+3] == 'l')
        {
            ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_NULL);
            if (val) *pos += 4;
            return val;
        }
        return NULL;
    default:
        if ((json[*pos] >= '0' && json[*pos] <= '9') || json[*pos] == '-')
            return ev_json_parse_number(json, len, pos);
        return NULL;
    }
}

ev_json_value_t *ev_json_parse(const char *json, uintptr_t len)
{
    uintptr_t pos = 0;
    if (!json || len == 0) return NULL;
    return ev_json_parse_value(json, len, &pos);
}

void ev_json_value_free(ev_json_value_t *val)
{
    uintptr_t i;
    if (!val) return;
    switch (val->type)
    {
    case EV_JSON_TYPE_STRING:
        free(val->u.string_val);
        break;
    case EV_JSON_TYPE_ARRAY:
        for (i = 0; i < val->u.array.count; i++)
            ev_json_value_free(val->u.array.items[i]);
        free(val->u.array.items);
        break;
    case EV_JSON_TYPE_OBJECT:
        for (i = 0; i < val->u.object.count; i++)
        {
            free(val->u.object.keys[i]);
            ev_json_value_free(val->u.object.values[i]);
        }
        free(val->u.object.keys);
        free(val->u.object.values);
        break;
    default:
        break;
    }
    free(val);
}

ev_json_value_t *ev_json_object_get(const ev_json_value_t *obj, const char *key)
{
    uintptr_t i;
    if (!obj || obj->type != EV_JSON_TYPE_OBJECT || !key) return NULL;
    for (i = 0; i < obj->u.object.count; i++)
    {
        if (strcmp(obj->u.object.keys[i], key) == 0)
            return obj->u.object.values[i];
    }
    return NULL;
}

ev_json_value_t *ev_json_array_get(const ev_json_value_t *arr, uintptr_t index)
{
    if (!arr || arr->type != EV_JSON_TYPE_ARRAY || index >= arr->u.array.count) return NULL;
    return arr->u.array.items[index];
}

const char *ev_json_get_string(const ev_json_value_t *val)
{
    if (!val || val->type != EV_JSON_TYPE_STRING) return NULL;
    return val->u.string_val;
}

int ev_json_get_bool(const ev_json_value_t *val)
{
    if (!val || val->type != EV_JSON_TYPE_BOOL) return 0;
    return val->u.bool_val;
}

double ev_json_get_number(const ev_json_value_t *val)
{
    if (!val || val->type != EV_JSON_TYPE_NUMBER) return 0.0;
    return val->u.number_val;
}

uintptr_t ev_json_array_count(const ev_json_value_t *arr)
{
    if (!arr || arr->type != EV_JSON_TYPE_ARRAY) return 0;
    return arr->u.array.count;
}

ev_json_value_t *ev_json_create_object(void)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_OBJECT);
    if (!val) return NULL;
    val->u.object.keys = (char **)malloc(16 * sizeof(char *));
    val->u.object.values = (ev_json_value_t **)malloc(16 * sizeof(ev_json_value_t *));
    if (!val->u.object.keys || !val->u.object.values)
    {
        free(val->u.object.keys);
        free(val->u.object.values);
        free(val);
        return NULL;
    }
    val->u.object.count = 0;
    return val;
}

ev_json_value_t *ev_json_create_array(void)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_ARRAY);
    if (!val) return NULL;
    val->u.array.items = (ev_json_value_t **)malloc(16 * sizeof(ev_json_value_t *));
    if (!val->u.array.items) { free(val); return NULL; }
    val->u.array.count = 0;
    return val;
}

ev_json_value_t *ev_json_create_string(const char *s)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_STRING);
    if (!val) return NULL;
    val->u.string_val = _strdup(s ? s : "");
    return val;
}

ev_json_value_t *ev_json_create_number(double n)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_NUMBER);
    if (!val) return NULL;
    val->u.number_val = n;
    return val;
}

ev_json_value_t *ev_json_create_bool(int b)
{
    ev_json_value_t *val = ev_json_alloc(EV_JSON_TYPE_BOOL);
    if (!val) return NULL;
    val->u.bool_val = b ? 1 : 0;
    return val;
}

ev_json_value_t *ev_json_create_null(void)
{
    return ev_json_alloc(EV_JSON_TYPE_NULL);
}

int ev_json_object_set(ev_json_value_t *obj, const char *key, ev_json_value_t *val)
{
    uintptr_t i;
    uintptr_t capacity;
    if (!obj || obj->type != EV_JSON_TYPE_OBJECT || !key) return 0;
    for (i = 0; i < obj->u.object.count; i++)
    {
        if (strcmp(obj->u.object.keys[i], key) == 0)
        {
            ev_json_value_free(obj->u.object.values[i]);
            obj->u.object.values[i] = val;
            return 1;
        }
    }
    capacity = (obj->u.object.count + 16) & ~15;
    obj->u.object.keys = (char **)realloc(obj->u.object.keys, capacity * sizeof(char *));
    obj->u.object.values = (ev_json_value_t **)realloc(obj->u.object.values, capacity * sizeof(ev_json_value_t *));
    if (!obj->u.object.keys || !obj->u.object.values) return 0;
    obj->u.object.keys[obj->u.object.count] = _strdup(key);
    obj->u.object.values[obj->u.object.count] = val;
    obj->u.object.count++;
    return 1;
}

int ev_json_array_push(ev_json_value_t *arr, ev_json_value_t *val)
{
    uintptr_t capacity;
    if (!arr || arr->type != EV_JSON_TYPE_ARRAY) return 0;
    capacity = (arr->u.array.count + 16) & ~15;
    arr->u.array.items = (ev_json_value_t **)realloc(arr->u.array.items, capacity * sizeof(ev_json_value_t *));
    if (!arr->u.array.items) return 0;
    arr->u.array.items[arr->u.array.count++] = val;
    return 1;
}

static uintptr_t ev_json_serialize_value(const ev_json_value_t *val, char *buf, uintptr_t buf_size)
{
    uintptr_t i;
    uintptr_t pos = 0;
    if (!val) return 0;
    switch (val->type)
    {
    case EV_JSON_TYPE_NULL:
        pos += _snprintf(buf + pos, buf_size - pos, "null");
        break;
    case EV_JSON_TYPE_BOOL:
        pos += _snprintf(buf + pos, buf_size - pos, val->u.bool_val ? "true" : "false");
        break;
    case EV_JSON_TYPE_NUMBER:
        pos += _snprintf(buf + pos, buf_size - pos, "%.17g", val->u.number_val);
        break;
    case EV_JSON_TYPE_STRING:
        pos += _snprintf(buf + pos, buf_size - pos, "\"");
        if (val->u.string_val)
        {
            const char *s = val->u.string_val;
            while (*s && pos < buf_size - 6)
            {
                switch (*s)
                {
                case '"':  pos += _snprintf(buf + pos, buf_size - pos, "\\\""); break;
                case '\\': pos += _snprintf(buf + pos, buf_size - pos, "\\\\"); break;
                case '\b': pos += _snprintf(buf + pos, buf_size - pos, "\\b");  break;
                case '\f': pos += _snprintf(buf + pos, buf_size - pos, "\\f");  break;
                case '\n': pos += _snprintf(buf + pos, buf_size - pos, "\\n");  break;
                case '\r': pos += _snprintf(buf + pos, buf_size - pos, "\\r");  break;
                case '\t': pos += _snprintf(buf + pos, buf_size - pos, "\\t");  break;
                default:
                    if ((unsigned char)*s < 0x20)
                        pos += _snprintf(buf + pos, buf_size - pos, "\\u%04x", (unsigned char)*s);
                    else
                        buf[pos++] = *s;
                    break;
                }
                s++;
            }
        }
        pos += _snprintf(buf + pos, buf_size - pos, "\"");
        break;
    case EV_JSON_TYPE_ARRAY:
        buf[pos++] = '[';
        for (i = 0; i < val->u.array.count; i++)
        {
            if (i > 0) buf[pos++] = ',';
            pos += ev_json_serialize_value(val->u.array.items[i], buf + pos, buf_size - pos);
        }
        buf[pos++] = ']';
        break;
    case EV_JSON_TYPE_OBJECT:
        buf[pos++] = '{';
        for (i = 0; i < val->u.object.count; i++)
        {
            if (i > 0) buf[pos++] = ',';
            pos += _snprintf(buf + pos, buf_size - pos, "\"%s\":", val->u.object.keys[i]);
            pos += ev_json_serialize_value(val->u.object.values[i], buf + pos, buf_size - pos);
        }
        buf[pos++] = '}';
        break;
    }
    return pos;
}

char *ev_json_serialize(const ev_json_value_t *val, uintptr_t *out_len)
{
    uintptr_t size = 65536;
    char *buf = (char *)malloc(size);
    uintptr_t len;
    if (!buf) return NULL;
    len = ev_json_serialize_value(val, buf, size - 1);
    buf[len] = 0;
    if (out_len) *out_len = len;
    return buf;
}
