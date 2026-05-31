#include "ev_rclone.h"
#include "ev_rclone_rc.h"
#include "ev_rclone_config.h"
#include "ev_http_client.h"
#include "ev_json.h"
#include "ev_string_util.h"
#include <string.h>
#include <stdio.h>

ev_rclone_t g_rclone;

static void ev_rclone_init_api(everything_plugin_get_proc_address_t get_proc_address)
{
    everything_plugin_api_t *api = &g_rclone.api;
    memset(api, 0, sizeof(*api));

    api->ansi_buf_copy_utf8_string = (everything_plugin_utf8_buf_t *(*)(everything_plugin_utf8_buf_t *, const everything_plugin_utf8_string_t *))get_proc_address("ansi_buf_copy_utf8_string");
    api->ansi_buf_init = (everything_plugin_utf8_buf_t *(*)(everything_plugin_utf8_buf_t *))get_proc_address("ansi_buf_init");
    api->ansi_buf_kill = (void (*)(everything_plugin_utf8_buf_t *))get_proc_address("ansi_buf_kill");
    api->config_get_int_value = (int (*)(const everything_plugin_utf8_t *, const everything_plugin_utf8_t *, int))get_proc_address("config_get_int_value");
    api->config_set_int_value = (void (*)(const everything_plugin_utf8_t *, const everything_plugin_utf8_t *, int))get_proc_address("config_set_int_value");
    api->db_add_local_ref = (void (*)(everything_plugin_db_t *, const everything_plugin_utf8_t *, const everything_plugin_fileinfo_fd_t *))get_proc_address("db_add_local_ref");
    api->db_file_exists = (int (*)(everything_plugin_db_t *, const everything_plugin_utf8_t *))get_proc_address("db_file_exists");
    api->db_find_close = (void (*)(everything_plugin_db_find_t *))get_proc_address("db_find_close");
    api->db_find_first_file = (everything_plugin_db_find_t *(*)(everything_plugin_db_t *, const everything_plugin_utf8_t *))get_proc_address("db_find_first_file");
    api->db_find_get_count = (uintptr_t (*)(everything_plugin_db_find_t *))get_proc_address("db_find_get_count");
    api->db_find_next_file = (int (*)(everything_plugin_db_find_t *))get_proc_address("db_find_next_file");
    api->db_folder_exists = (int (*)(everything_plugin_db_t *, const everything_plugin_utf8_t *))get_proc_address("db_folder_exists");
    api->db_get_indexed_fd = (everything_plugin_fileinfo_fd_t *(*)(everything_plugin_db_t *, uintptr_t))get_proc_address("db_get_indexed_fd");
    api->db_is_index_folder_size = (int (*)(everything_plugin_db_t *))get_proc_address("db_is_index_folder_size");
    api->db_onready_add = (void (*)(void (*)(void *), void *))get_proc_address("db_onready_add");
    api->db_onready_remove = (void (*)(void (*)(void *), void *))get_proc_address("db_onready_remove");
    api->db_release = (void (*)(everything_plugin_db_t *))get_proc_address("db_release");
    api->mem_alloc = (void *(*)(uintptr_t))get_proc_address("mem_alloc");
    api->mem_calloc = (void *(*)(uintptr_t, uintptr_t))get_proc_address("mem_calloc");
    api->mem_free = (void (*)(void *))get_proc_address("mem_free");
    api->os_thread_create = (void *(*)(LPTHREAD_START_ROUTINE, void *))get_proc_address("os_thread_create");
    api->os_thread_wait_and_close = (void (*)(void *))get_proc_address("os_thread_wait_and_close");
    api->os_zero_memory = (void (*)(void *, uintptr_t))get_proc_address("os_zero_memory");
    api->utf8_buf_copy_string = (void (*)(everything_plugin_utf8_buf_t *, const everything_plugin_utf8_t *))get_proc_address("utf8_buf_copy_string");
    api->utf8_buf_init = (everything_plugin_utf8_buf_t *(*)(everything_plugin_utf8_buf_t *))get_proc_address("utf8_buf_init");
    api->utf8_buf_kill = (void (*)(everything_plugin_utf8_buf_t *))get_proc_address("utf8_buf_kill");
    api->utf8_buf_printf = (void (*)(everything_plugin_utf8_buf_t *, const everything_plugin_utf8_t *, ...))get_proc_address("utf8_buf_printf");
    api->debug_printf = (void (*)(const char *, ...))get_proc_address("debug_printf");
    api->debug_error_printf = (void (*)(const char *, ...))get_proc_address("debug_error_printf");
    api->plugin_get_setting_int = (int (*)(const everything_plugin_utf8_t *, int))get_proc_address("plugin_get_setting_int");
    api->plugin_set_setting_int = (void (*)(const everything_plugin_utf8_t *, int))get_proc_address("plugin_set_setting_int");
    api->plugin_get_setting_string = (const everything_plugin_utf8_t *(*)(const everything_plugin_utf8_t *, const everything_plugin_utf8_t *))get_proc_address("plugin_get_setting_string");
    api->plugin_set_setting_string = (void (*)(const everything_plugin_utf8_t *, const everything_plugin_utf8_t *))get_proc_address("plugin_set_setting_string");
    api->ui_options_add_plugin_page = (void (*)(const everything_plugin_utf8_t *, void *))get_proc_address("ui_options_add_plugin_page");
    api->os_create_checkbox = (HWND (*)(HWND, int, DWORD, const everything_plugin_utf8_t *, int, int, int, int))get_proc_address("os_create_checkbox");
    api->os_create_edit = (HWND (*)(HWND, int, DWORD, int, int, int, int))get_proc_address("os_create_edit");
    api->os_create_static = (HWND (*)(HWND, int, DWORD, const everything_plugin_utf8_t *, int, int, int, int))get_proc_address("os_create_static");
    api->os_create_button = (HWND (*)(HWND, int, DWORD, const everything_plugin_utf8_t *, int, int, int, int))get_proc_address("os_create_button");
    api->os_create_group_box = (HWND (*)(HWND, int, DWORD, const everything_plugin_utf8_t *, int, int, int, int))get_proc_address("os_create_group_box");
    api->os_get_dlg_text = (void (*)(HWND, int, everything_plugin_utf8_buf_t *))get_proc_address("os_get_dlg_text");
    api->os_set_dlg_text = (void (*)(HWND, int, const everything_plugin_utf8_t *))get_proc_address("os_set_dlg_text");
    api->os_enable_or_disable_dlg_item = (void (*)(HWND, int, int))get_proc_address("os_enable_or_disable_dlg_item");
    api->output_stream_close = (everything_plugin_output_stream_t *(*)(everything_plugin_output_stream_t *))get_proc_address("output_stream_close");
    api->output_stream_write_printf = (void (*)(everything_plugin_output_stream_t *, const char *, ...))get_proc_address("output_stream_write_printf");
    api->output_stream_append_file = (void (*)(everything_plugin_output_stream_t *, const everything_plugin_utf8_t *))get_proc_address("output_stream_append_file");
    api->event_post = (void (*)(void (*)(void *), void *))get_proc_address("event_post");
    api->event_remove = (void (*)(void (*)(void *), void *))get_proc_address("event_remove");
    api->os_event_create = (void *(*)(void))get_proc_address("os_event_create");
    api->os_event_is_set = (int (*)(void *))get_proc_address("os_event_is_set");
    api->property_get_builtin_type = (const everything_plugin_property_t *(*)(int))get_proc_address("property_get_builtin_type");
    api->property_get_type = (int (*)(const everything_plugin_property_t *))get_proc_address("property_get_type");
    api->localization_get_string = (void (*)(int, everything_plugin_utf8_buf_t *))get_proc_address("localization_get_string");
    api->localization_get_en_us_string = (void (*)(int, everything_plugin_utf8_buf_t *))get_proc_address("localization_get_en_us_string");
    api->db = (everything_plugin_db_t *)get_proc_address("db");
}

static void ev_rclone_index_cloud_files(void)
{
    char remotes[EV_RCLONE_MAX_REMOTES][256];
    int remote_count;
    int i;

    if (!g_rclone.api.db) return;

    remote_count = ev_rclone_rc_list_remotes(remotes, EV_RCLONE_MAX_REMOTES);
    if (remote_count <= 0) return;

    EnterCriticalSection(&g_rclone.cs);
    g_rclone.remote_count = 0;
    for (i = 0; i < remote_count && g_rclone.remote_count < EV_RCLONE_MAX_REMOTES; i++)
    {
        ev_string_copy(g_rclone.remotes[g_rclone.remote_count].name, remotes[i], 256);
        g_rclone.remotes[g_rclone.remote_count].is_indexed = 1;
        g_rclone.remote_count++;
    }
    LeaveCriticalSection(&g_rclone.cs);

    for (i = 0; i < remote_count; i++)
    {
        char remote_path[512];
        ev_json_value_t *list;
        uintptr_t j;

        _snprintf(remote_path, sizeof(remote_path), "%s:", remotes[i]);

        if (!ev_rclone_rc_list_dir(remote_path, &list)) continue;

        for (j = 0; j < ev_json_array_count(list); j++)
        {
            ev_json_value_t *item = ev_json_array_get(list, j);
            ev_json_value_t *name_val;
            ev_json_value_t *is_dir_val;
            ev_json_value_t *size_val;
            ev_json_value_t *mod_time_val;
            char full_path[EV_RCLONE_MAX_PATH_LEN];
            everything_plugin_fileinfo_fd_t fd;

            if (!item) continue;

            name_val = ev_json_object_get(item, "Name");
            if (!name_val || name_val->type != EV_JSON_TYPE_STRING) continue;

            _snprintf(full_path, sizeof(full_path), "%s\\%s", remote_path, ev_json_get_string(name_val));

            memset(&fd, 0, sizeof(fd));

            is_dir_val = ev_json_object_get(item, "IsDir");
            if (is_dir_val && is_dir_val->type == EV_JSON_TYPE_BOOL && ev_json_get_bool(is_dir_val))
            {
                fd.attributes = FILE_ATTRIBUTE_DIRECTORY;
            }

            size_val = ev_json_object_get(item, "Size");
            if (size_val && size_val->type == EV_JSON_TYPE_NUMBER)
            {
                fd.size = (everything_plugin_qword_t)ev_json_get_number(size_val);
            }

            mod_time_val = ev_json_object_get(item, "ModTime");
            if (mod_time_val && mod_time_val->type == EV_JSON_TYPE_STRING)
            {
                SYSTEMTIME st;
                const char *time_str = ev_json_get_string(mod_time_val);
                memset(&st, 0, sizeof(st));
                sscanf(time_str, "%d-%d-%dT%d:%d:%d",
                    &st.wYear, &st.wMonth, &st.wDay,
                    &st.wHour, &st.wMinute, &st.wSecond);
                SystemTimeToFileTime(&st, (FILETIME *)&fd.date_modified);
            }

            g_rclone.api.db_add_local_ref(g_rclone.api.db, full_path, &fd);
        }

        ev_json_value_free(list);
    }
}

static void ev_rclone_on_db_ready(void *data)
{
    if (g_rclone.is_enabled && g_rclone.index_on_start && g_rclone.rclone_running)
    {
        ev_rclone_index_cloud_files();
    }
}

void *EVERYTHING_PLUGIN_API everything_plugin_proc(DWORD msg, void *data)
{
    switch (msg)
    {
    case EVERYTHING_PLUGIN_PM_INIT:
    {
        everything_plugin_get_proc_address_t get_proc_address =
            (everything_plugin_get_proc_address_t)data;
        if (!get_proc_address) return NULL;

        ev_rclone_init_api(get_proc_address);

        InitializeCriticalSection(&g_rclone.cs);

        ev_rclone_load_settings();

        ev_http_client_init();

        return (void *)EVERYTHING_PLUGIN_VERSION;
    }

    case EVERYTHING_PLUGIN_PM_GET_PLUGIN_VERSION:
        return (void *)EVERYTHING_PLUGIN_VERSION;

    case EVERYTHING_PLUGIN_PM_GET_NAME:
        return (void *)EV_RCLONE_PLUGIN_NAME;

    case EVERYTHING_PLUGIN_PM_GET_DESCRIPTION:
        return (void *)EV_RCLONE_PLUGIN_DESCRIPTION;

    case EVERYTHING_PLUGIN_PM_GET_AUTHOR:
        return (void *)EV_RCLONE_PLUGIN_AUTHOR;

    case EVERYTHING_PLUGIN_PM_GET_VERSION:
        return (void *)EV_RCLONE_VERSION_STRING;

    case EVERYTHING_PLUGIN_PM_GET_LINK:
        return (void *)EV_RCLONE_PLUGIN_LINK;

    case EVERYTHING_PLUGIN_PM_START:
    {
        if (g_rclone.is_enabled && g_rclone.auto_start)
        {
            ev_rclone_rc_start(g_rclone.rclone_path, g_rclone.rc_addr, g_rclone.rc_port);
        }

        if (g_rclone.api.db_onready_add)
        {
            g_rclone.api.db_onready_add(ev_rclone_on_db_ready, NULL);
        }

        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_STOP:
    {
        if (g_rclone.api.db_onready_remove)
        {
            g_rclone.api.db_onready_remove(ev_rclone_on_db_ready, NULL);
        }

        ev_rclone_rc_stop();
        ev_http_client_cleanup();

        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_KILL:
    {
        ev_rclone_rc_stop();
        ev_http_client_cleanup();
        DeleteCriticalSection(&g_rclone.cs);
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_UNINSTALL:
        return (void *)1;

    case EVERYTHING_PLUGIN_PM_ADD_OPTIONS_PAGES:
    {
        everything_plugin_load_options_page_t *load_data =
            (everything_plugin_load_options_page_t *)data;
        if (load_data)
        {
            ev_rclone_config_add_options_page(load_data->user_data);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_LOAD_OPTIONS_PAGE:
    {
        everything_plugin_load_options_page_t *load_data =
            (everything_plugin_load_options_page_t *)data;
        if (load_data)
        {
            ev_rclone_config_load_options_page(load_data->hwnd, load_data->user_data);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_SAVE_OPTIONS_PAGE:
    {
        everything_plugin_save_options_page_t *save_data =
            (everything_plugin_save_options_page_t *)data;
        if (save_data)
        {
            ev_rclone_config_save_options_page(save_data->user_data);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_GET_OPTIONS_PAGE_MINMAX:
    {
        everything_plugin_get_options_page_minmax_t *mm =
            (everything_plugin_get_options_page_minmax_t *)data;
        if (mm)
        {
            ev_rclone_config_get_options_page_minmax(mm->user_data,
                &mm->min_width, &mm->min_height,
                &mm->max_width, &mm->max_height);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_SIZE_OPTIONS_PAGE:
    {
        everything_plugin_size_options_page_t *sp =
            (everything_plugin_size_options_page_t *)data;
        if (sp)
        {
            ev_rclone_config_size_options_page(sp->user_data, sp->width, sp->height);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_OPTIONS_PAGE_PROC:
    {
        everything_plugin_options_page_proc_t *op =
            (everything_plugin_options_page_proc_t *)data;
        if (op)
        {
            INT_PTR result = ev_rclone_config_options_page_proc(
                op->user_data, op->hwnd, op->msg, op->wParam, op->lParam);
            return (void *)result;
        }
        return (void *)FALSE;
    }

    case EVERYTHING_PLUGIN_PM_KILL_OPTIONS_PAGE:
    {
        everything_plugin_kill_options_page_t *kp =
            (everything_plugin_kill_options_page_t *)data;
        if (kp)
        {
            ev_rclone_config_kill_options_page(kp->user_data);
        }
        return (void *)1;
    }

    case EVERYTHING_PLUGIN_PM_SAVE_SETTINGS:
    {
        ev_rclone_save_settings();
        return (void *)1;
    }

    default:
        return NULL;
    }
}
