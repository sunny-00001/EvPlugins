#include "ev_rclone_rc.h"
#include "ev_rclone.h"
#include "ev_http_client.h"
#include "ev_string_util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static DWORD WINAPI ev_rclone_rc_monitor_thread(void *param);

int ev_rclone_rc_start(const char *rclone_path, const char *addr, int port)
{
    char cmdline[2048];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    if (g_rclone.rclone_running) return 1;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    memset(&pi, 0, sizeof(pi));

    _snprintf(cmdline, sizeof(cmdline),
        "\"%s\" rcd --rc-addr %s:%d --rc-no-auth",
        rclone_path, addr, port);

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        return 0;
    }

    CloseHandle(pi.hThread);
    g_rclone.rclone_proc_info = pi;
    g_rclone.rclone_running = 1;
    g_rclone.rclone_monitor_stop = 0;

    g_rclone.rclone_monitor_thread = CreateThread(NULL, 0,
        ev_rclone_rc_monitor_thread, NULL, 0, NULL);

    return 1;
}

void ev_rclone_rc_stop(void)
{
    if (!g_rclone.rclone_running) return;

    g_rclone.rclone_monitor_stop = 1;

    if (g_rclone.rclone_proc_info.hProcess)
    {
        TerminateProcess(g_rclone.rclone_proc_info.hProcess, 0);
        WaitForSingleObject(g_rclone.rclone_proc_info.hProcess, 5000);
        CloseHandle(g_rclone.rclone_proc_info.hProcess);
        g_rclone.rclone_proc_info.hProcess = NULL;
    }

    g_rclone.rclone_running = 0;

    if (g_rclone.rclone_monitor_thread)
    {
        WaitForSingleObject(g_rclone.rclone_monitor_thread, 5000);
        CloseHandle(g_rclone.rclone_monitor_thread);
        g_rclone.rclone_monitor_thread = NULL;
    }
}

int ev_rclone_rc_is_running(void)
{
    return g_rclone.rclone_running;
}

ev_rclone_rc_result_t ev_rclone_rc_call(const char *method, const ev_json_value_t *params)
{
    ev_rclone_rc_result_t result;
    char path[512];
    char *body;
    uintptr_t body_len;
    ev_http_response_t *resp;
    ev_json_value_t *response_json;

    memset(&result, 0, sizeof(result));

    if (!g_rclone.rclone_running)
    {
        ev_string_copy(result.error, "rclone is not running", sizeof(result.error));
        return result;
    }

    _snprintf(path, sizeof(path), "/%s", method);

    if (params)
    {
        body = ev_json_serialize(params, &body_len);
    }
    else
    {
        body = _strdup("{}");
        body_len = 2;
    }

    if (!body)
    {
        ev_string_copy(result.error, "Failed to serialize JSON", sizeof(result.error));
        return result;
    }

    resp = ev_http_post(g_rclone.rc_addr, g_rclone.rc_port, path,
        "application/json", body, body_len);
    free(body);

    if (!resp)
    {
        ev_string_copy(result.error, "HTTP request failed", sizeof(result.error));
        return result;
    }

    if (resp->status_code != 200)
    {
        _snprintf(result.error, sizeof(result.error),
            "HTTP error: %d", resp->status_code);
        ev_http_response_free(resp);
        return result;
    }

    response_json = ev_json_parse(resp->body, resp->body_len);
    ev_http_response_free(resp);

    if (!response_json)
    {
        ev_string_copy(result.error, "Failed to parse JSON response", sizeof(result.error));
        return result;
    }

    {
        ev_json_value_t *error_val = ev_json_object_get(response_json, "error");
        if (error_val && error_val->type == EV_JSON_TYPE_STRING)
        {
            ev_string_copy(result.error, ev_json_get_string(error_val), sizeof(result.error));
            ev_json_value_free(response_json);
            return result;
        }
    }

    result.success = 1;
    result.json = response_json;
    return result;
}

void ev_rclone_rc_result_free(ev_rclone_rc_result_t *result)
{
    if (!result) return;
    if (result->json)
    {
        ev_json_value_free(result->json);
        result->json = NULL;
    }
}

int ev_rclone_rc_list_remotes(char remotes[][256], int max_remotes)
{
    ev_rclone_rc_result_t result;
    ev_json_value_t *remotes_val;
    int count = 0;
    uintptr_t i;

    result = ev_rclone_rc_call("config/listremotes", NULL);
    if (!result.success)
    {
        ev_rclone_rc_result_free(&result);
        return 0;
    }

    remotes_val = ev_json_object_get(result.json, "remotes");
    if (!remotes_val || remotes_val->type != EV_JSON_TYPE_ARRAY)
    {
        ev_rclone_rc_result_free(&result);
        return 0;
    }

    for (i = 0; i < ev_json_array_count(remotes_val) && count < max_remotes; i++)
    {
        ev_json_value_t *item = ev_json_array_get(remotes_val, i);
        if (item && item->type == EV_JSON_TYPE_STRING)
        {
            ev_string_copy(remotes[count], ev_json_get_string(item), 256);
            count++;
        }
    }

    ev_rclone_rc_result_free(&result);
    return count;
}

int ev_rclone_rc_list_dir(const char *remote_path, ev_json_value_t **out_list)
{
    ev_rclone_rc_result_t result;
    ev_json_value_t *params;
    ev_json_value_t *list_val;

    params = ev_json_create_object();
    ev_json_object_set(params, "fs", ev_json_create_string(remote_path));
    ev_json_object_set(params, "long", ev_json_create_bool(1));

    result = ev_rclone_rc_call("operations/list", params);
    ev_json_value_free(params);

    if (!result.success)
    {
        ev_rclone_rc_result_free(&result);
        return 0;
    }

    list_val = ev_json_object_get(result.json, "list");
    if (!list_val || list_val->type != EV_JSON_TYPE_ARRAY)
    {
        ev_rclone_rc_result_free(&result);
        return 0;
    }

    *out_list = list_val;
    list_val = NULL;

    ev_rclone_rc_result_free(&result);
    return 1;
}

static DWORD WINAPI ev_rclone_rc_monitor_thread(void *param)
{
    while (!g_rclone.rclone_monitor_stop)
    {
        DWORD exit_code;
        if (!GetExitCodeProcess(g_rclone.rclone_proc_info.hProcess, &exit_code))
            break;
        if (exit_code != STILL_ACTIVE)
        {
            g_rclone.rclone_running = 0;
            break;
        }
        Sleep(1000);
    }
    return 0;
}
