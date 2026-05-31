#ifndef EVERYTHING_PLUGIN_H
#define EVERYTHING_PLUGIN_H

#define EVERYTHING_PLUGIN_API WINAPI

#define EVERYTHING_PLUGIN_VERSION 1

#define EVERYTHING_PLUGIN_PM_INIT                    1
#define EVERYTHING_PLUGIN_PM_GET_PLUGIN_VERSION      2
#define EVERYTHING_PLUGIN_PM_GET_NAME                3
#define EVERYTHING_PLUGIN_PM_GET_DESCRIPTION         4
#define EVERYTHING_PLUGIN_PM_GET_AUTHOR              5
#define EVERYTHING_PLUGIN_PM_GET_VERSION             6
#define EVERYTHING_PLUGIN_PM_GET_LINK                7
#define EVERYTHING_PLUGIN_PM_START                   8
#define EVERYTHING_PLUGIN_PM_STOP                    9
#define EVERYTHING_PLUGIN_PM_KILL                    10
#define EVERYTHING_PLUGIN_PM_UNINSTALL               11
#define EVERYTHING_PLUGIN_PM_ADD_OPTIONS_PAGES       12
#define EVERYTHING_PLUGIN_PM_LOAD_OPTIONS_PAGE       13
#define EVERYTHING_PLUGIN_PM_SAVE_OPTIONS_PAGE       14
#define EVERYTHING_PLUGIN_PM_GET_OPTIONS_PAGE_MINMAX 15
#define EVERYTHING_PLUGIN_PM_SIZE_OPTIONS_PAGE       16
#define EVERYTHING_PLUGIN_PM_OPTIONS_PAGE_PROC       17
#define EVERYTHING_PLUGIN_PM_KILL_OPTIONS_PAGE       18
#define EVERYTHING_PLUGIN_PM_SAVE_SETTINGS           19

typedef unsigned char everything_plugin_utf8_t;
typedef unsigned long everything_plugin_dword_t;
typedef unsigned __int64 everything_plugin_qword_t;

typedef struct everything_plugin_utf8_buf_s
{
    everything_plugin_utf8_t *buf;
    uintptr_t len;
    uintptr_t size;
    everything_plugin_utf8_t stack[MAX_PATH];
} everything_plugin_utf8_buf_t;

typedef struct everything_plugin_utf8_string_s
{
    everything_plugin_utf8_t *text;
    uintptr_t len;
} everything_plugin_utf8_string_t;

typedef struct everything_plugin_utf8_basic_string_s
{
    uintptr_t len;
} everything_plugin_utf8_basic_string_t;

#define EVERYTHING_PLUGIN_UTF8_BASIC_STRING_TEXT(s) ((everything_plugin_utf8_t *)(((everything_plugin_utf8_basic_string_t *)(s)) + 1))

typedef struct everything_plugin_fileinfo_fd_s
{
    everything_plugin_qword_t size;
    everything_plugin_qword_t date_created;
    everything_plugin_qword_t date_modified;
    everything_plugin_qword_t date_accessed;
    DWORD attributes;
} everything_plugin_fileinfo_fd_t;

typedef struct everything_plugin_property_s everything_plugin_property_t;
typedef struct everything_plugin_db_s everything_plugin_db_t;
typedef struct everything_plugin_db_query_s everything_plugin_db_query_t;
typedef struct everything_plugin_db_find_s everything_plugin_db_find_t;
typedef struct everything_plugin_db_snapshot_s everything_plugin_db_snapshot_t;
typedef struct everything_plugin_db_journal_notification_s everything_plugin_db_journal_notification_t;
typedef struct everything_plugin_db_remap_list_s everything_plugin_db_remap_list_t;
typedef struct everything_plugin_db_remap_array_s everything_plugin_db_remap_array_t;
typedef struct everything_plugin_output_stream_s everything_plugin_output_stream_t;
typedef struct everything_plugin_os_thread_s everything_plugin_os_thread_t;

typedef void *(EVERYTHING_PLUGIN_API *everything_plugin_get_proc_address_t)(const char *name);

typedef struct everything_plugin_load_options_page_s
{
    HWND hwnd;
    void *user_data;
} everything_plugin_load_options_page_t;

typedef struct everything_plugin_save_options_page_s
{
    void *user_data;
} everything_plugin_save_options_page_t;

typedef struct everything_plugin_get_options_page_minmax_s
{
    void *user_data;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
} everything_plugin_get_options_page_minmax_t;

typedef struct everything_plugin_size_options_page_s
{
    void *user_data;
    int width;
    int height;
} everything_plugin_size_options_page_t;

typedef struct everything_plugin_options_page_proc_s
{
    void *user_data;
    HWND hwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
} everything_plugin_options_page_proc_t;

typedef struct everything_plugin_kill_options_page_s
{
    void *user_data;
} everything_plugin_kill_options_page_t;

typedef struct
{
    everything_plugin_utf8_buf_t *(*ansi_buf_copy_utf8_string)(everything_plugin_utf8_buf_t *buf, const everything_plugin_utf8_string_t *s);
    everything_plugin_utf8_buf_t *(*ansi_buf_init)(everything_plugin_utf8_buf_t *buf);
    void (*ansi_buf_kill)(everything_plugin_utf8_buf_t *buf);
    int (*config_get_int_value)(const everything_plugin_utf8_t *section, const everything_plugin_utf8_t *key, int default_value);
    void (*config_set_int_value)(const everything_plugin_utf8_t *section, const everything_plugin_utf8_t *key, int value);
    void (*db_add_local_ref)(everything_plugin_db_t *db, const everything_plugin_utf8_t *filename, const everything_plugin_fileinfo_fd_t *fd);
    int (*db_file_exists)(everything_plugin_db_t *db, const everything_plugin_utf8_t *filename);
    void (*db_find_close)(everything_plugin_db_find_t *find);
    everything_plugin_db_find_t *(*db_find_first_file)(everything_plugin_db_t *db, const everything_plugin_utf8_t *filename);
    uintptr_t (*db_find_get_count)(everything_plugin_db_find_t *find);
    int (*db_find_next_file)(everything_plugin_db_find_t *find);
    int (*db_folder_exists)(everything_plugin_db_t *db, const everything_plugin_utf8_t *filename);
    everything_plugin_fileinfo_fd_t *(*db_get_indexed_fd)(everything_plugin_db_t *db, uintptr_t index);
    int (*db_is_index_folder_size)(everything_plugin_db_t *db);
    int (*db_journal_file_would_block)(void *file);
    void (*db_journal_notification_register)(everything_plugin_db_journal_notification_t *notification);
    void (*db_journal_notification_unregister)(everything_plugin_db_journal_notification_t *notification);
    void (*db_onready_add)(void (*proc)(void *data), void *data);
    void (*db_onready_remove)(void (*proc)(void *data), void *data);
    void (*db_query_cancel)(everything_plugin_db_query_t *q);
    everything_plugin_db_query_t *(*db_query_create)(everything_plugin_db_t *db);
    void (*db_query_destroy)(everything_plugin_db_query_t *q);
    uintptr_t (*db_query_get_result_count)(everything_plugin_db_query_t *q);
    everything_plugin_qword_t (*db_query_get_result_date_recently_changed)(everything_plugin_db_query_t *q, uintptr_t index);
    const everything_plugin_utf8_t *(*db_query_get_result_file_list_filename)(everything_plugin_db_query_t *q, uintptr_t index);
    everything_plugin_fileinfo_fd_t *(*db_query_get_result_indexed_fd)(everything_plugin_db_query_t *q, uintptr_t index);
    const everything_plugin_utf8_t *(*db_query_get_result_name)(everything_plugin_db_query_t *q, uintptr_t index);
    const everything_plugin_utf8_t *(*db_query_get_result_path)(everything_plugin_db_query_t *q, uintptr_t index);
    int (*db_query_is_fast_sort)(everything_plugin_db_query_t *q, const everything_plugin_property_t *prop);
    int (*db_query_is_folder_result)(everything_plugin_db_query_t *q, uintptr_t index);
    void (*db_query_search)(everything_plugin_db_query_t *q, const everything_plugin_utf8_t *search_string, DWORD filter_flags);
    void (*db_query_search2)(everything_plugin_db_query_t *q, const everything_plugin_utf8_t *search_string, DWORD filter_flags, const everything_plugin_property_t *sort_column_type, int sort_ascending);
    void (*db_query_sort)(everything_plugin_db_query_t *q, const everything_plugin_property_t *sort_column_type, int sort_ascending);
    void (*db_release)(everything_plugin_db_t *db);
    void *(*mem_alloc)(uintptr_t size);
    void *(*mem_calloc)(uintptr_t count, uintptr_t size);
    void (*mem_free)(void *ptr);
    void *(*os_thread_create)(LPTHREAD_START_ROUTINE proc, void *data);
    void (*os_thread_wait_and_close)(void *thread);
    void (*os_zero_memory)(void *ptr, uintptr_t size);
    void (*utf8_buf_copy_string)(everything_plugin_utf8_buf_t *buf, const everything_plugin_utf8_t *s);
    everything_plugin_utf8_buf_t *(*utf8_buf_init)(everything_plugin_utf8_buf_t *buf);
    void (*utf8_buf_kill)(everything_plugin_utf8_buf_t *buf);
    void (*utf8_buf_printf)(everything_plugin_utf8_buf_t *buf, const everything_plugin_utf8_t *format, ...);
    void (*debug_printf)(const char *format, ...);
    void (*debug_error_printf)(const char *format, ...);
    int (*plugin_get_setting_int)(const everything_plugin_utf8_t *key, int default_value);
    void (*plugin_set_setting_int)(const everything_plugin_utf8_t *key, int value);
    const everything_plugin_utf8_t *(*plugin_get_setting_string)(const everything_plugin_utf8_t *key, const everything_plugin_utf8_t *default_value);
    void (*plugin_set_setting_string)(const everything_plugin_utf8_t *key, const everything_plugin_utf8_t *value);
    void (*ui_options_add_plugin_page)(const everything_plugin_utf8_t *name, void *user_data);
    HWND (*os_create_checkbox)(HWND parent, int id, DWORD style, const everything_plugin_utf8_t *text, int x, int y, int w, int h);
    HWND (*os_create_edit)(HWND parent, int id, DWORD style, int x, int y, int w, int h);
    HWND (*os_create_static)(HWND parent, int id, DWORD style, const everything_plugin_utf8_t *text, int x, int y, int w, int h);
    HWND (*os_create_button)(HWND parent, int id, DWORD style, const everything_plugin_utf8_t *text, int x, int y, int w, int h);
    HWND (*os_create_group_box)(HWND parent, int id, DWORD style, const everything_plugin_utf8_t *text, int x, int y, int w, int h);
    void (*os_get_dlg_text)(HWND dlg, int id, everything_plugin_utf8_buf_t *buf);
    void (*os_set_dlg_text)(HWND dlg, int id, const everything_plugin_utf8_t *text);
    void (*os_enable_or_disable_dlg_item)(HWND dlg, int id, int enable);
    everything_plugin_output_stream_t *(*output_stream_close)(everything_plugin_output_stream_t *os);
    void (*output_stream_write_printf)(everything_plugin_output_stream_t *os, const char *format, ...);
    void (*output_stream_append_file)(everything_plugin_output_stream_t *os, const everything_plugin_utf8_t *filename);
    void (*event_post)(void (*proc)(void *data), void *data);
    void (*event_remove)(void (*proc)(void *data), void *data);
    void *(*os_event_create)(void);
    int (*os_event_is_set)(void *event);
    const everything_plugin_property_t *(*property_get_builtin_type)(int type);
    int (*property_get_type)(const everything_plugin_property_t *prop);
    void (*localization_get_string)(int id, everything_plugin_utf8_buf_t *buf);
    void (*localization_get_en_us_string)(int id, everything_plugin_utf8_buf_t *buf);
    everything_plugin_db_t *db;
} everything_plugin_api_t;

#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_RESULTS_CHANGED    0
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_STATUS_CHANGED     1
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_FILE_INFO_CHANGED  2
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_READY              3
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_ACCESS_DENIED      4
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_QUERY_COMPLETE     5
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_SORT_COMPLETE      6
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_QUERY_START        7
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_SORT_START         8
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_ON_LOADED          9
#define EVERYTHING_PLUGIN_DB_QUERY_EVENT_ON_INDEX_CANCELLED 10

#define EVERYTHING_PLUGIN_FILTER_FLAG_CASE       0x00000001
#define EVERYTHING_PLUGIN_FILTER_FLAG_WHOLEWORD  0x00000002
#define EVERYTHING_PLUGIN_FILTER_FLAG_PATH       0x00000004
#define EVERYTHING_PLUGIN_FILTER_FLAG_DIACRITICS 0x00000008
#define EVERYTHING_PLUGIN_FILTER_FLAG_REGEX      0x00000010
#define EVERYTHING_PLUGIN_FILTER_FLAG_PREFIX     0x00000020
#define EVERYTHING_PLUGIN_FILTER_FLAG_SUFFIX     0x00000040

#define EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME                 0
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_PATH                 1
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE                 2
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_EXTENSION            3
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_TYPE                 4
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_MODIFIED        5
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_CREATED         6
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_ACCESSED        7
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_ATTRIBUTES           8
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_RECENTLY_CHANGED 9
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_RUN_COUNT            10
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_RUN             11
#define EVERYTHING_PLUGIN_PROPERTY_TYPE_FILE_LIST_FILENAME   12

#define EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH      15
#define EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH      23
#define EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH        21
#define EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH    15

__declspec(dllexport) void *EVERYTHING_PLUGIN_API everything_plugin_proc(DWORD msg, void *data);

#endif
