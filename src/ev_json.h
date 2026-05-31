#ifndef EV_JSON_H
#define EV_JSON_H

#include <windows.h>

typedef enum ev_json_type_e
{
    EV_JSON_TYPE_NULL = 0,
    EV_JSON_TYPE_BOOL,
    EV_JSON_TYPE_NUMBER,
    EV_JSON_TYPE_STRING,
    EV_JSON_TYPE_ARRAY,
    EV_JSON_TYPE_OBJECT
} ev_json_type_t;

typedef struct ev_json_value_s ev_json_value_t;

struct ev_json_value_s
{
    ev_json_type_t type;
    union
    {
        int bool_val;
        double number_val;
        char *string_val;
        struct
        {
            ev_json_value_t **items;
            uintptr_t count;
        } array;
        struct
        {
            char **keys;
            ev_json_value_t **values;
            uintptr_t count;
        } object;
    } u;
};

ev_json_value_t *ev_json_parse(const char *json, uintptr_t len);
void ev_json_value_free(ev_json_value_t *val);
ev_json_value_t *ev_json_object_get(const ev_json_value_t *obj, const char *key);
ev_json_value_t *ev_json_array_get(const ev_json_value_t *arr, uintptr_t index);
const char *ev_json_get_string(const ev_json_value_t *val);
int ev_json_get_bool(const ev_json_value_t *val);
double ev_json_get_number(const ev_json_value_t *val);
uintptr_t ev_json_array_count(const ev_json_value_t *arr);
char *ev_json_serialize(const ev_json_value_t *val, uintptr_t *out_len);
ev_json_value_t *ev_json_create_object(void);
ev_json_value_t *ev_json_create_array(void);
ev_json_value_t *ev_json_create_string(const char *s);
ev_json_value_t *ev_json_create_number(double n);
ev_json_value_t *ev_json_create_bool(int b);
ev_json_value_t *ev_json_create_null(void);
int ev_json_object_set(ev_json_value_t *obj, const char *key, ev_json_value_t *val);
int ev_json_array_push(ev_json_value_t *arr, ev_json_value_t *val);

#endif
