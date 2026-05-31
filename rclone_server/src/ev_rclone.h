#ifndef EV_RCLONE_H
#define EV_RCLONE_H

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "everything_plugin.h"
#include "version.h"

#define EV_RCLONE_PLUGIN_NAME        "Rclone"
#define EV_RCLONE_PLUGIN_DESCRIPTION "Integrate rclone cloud storage with Everything. Browse, search, and manage cloud files."
#define EV_RCLONE_PLUGIN_AUTHOR      "sunny-00001"
#define EV_RCLONE_PLUGIN_LINK        "https://github.com/sunny-00001/EvPlugins"

#define EV_RCLONE_DEFAULT_RC_PORT    5572
#define EV_RCLONE_DEFAULT_RC_ADDR    "127.0.0.1"
#define EV_RCLONE_HTTP_RECV_BUF_SIZE 65536
#define EV_RCLONE_HTTP_SEND_BUF_SIZE 65536
#define EV_RCLONE_MAX_REMOTES        64
#define EV_RCLONE_MAX_PATH_LEN       4096

enum
{
    EV_RCLONE_OPT_ID_ENABLED_CHECKBOX,
    EV_RCLONE_OPT_ID_RCLONE_PATH_STATIC,
    EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT,
    EV_RCLONE_OPT_ID_RCLONE_PATH_BROWSE_BUTTON,
    EV_RCLONE_OPT_ID_RC_PORT_STATIC,
    EV_RCLONE_OPT_ID_RC_PORT_EDIT,
    EV_RCLONE_OPT_ID_RC_ADDR_STATIC,
    EV_RCLONE_OPT_ID_RC_ADDR_EDIT,
    EV_RCLONE_OPT_ID_AUTO_START_CHECKBOX,
    EV_RCLONE_OPT_ID_INDEX_ON_START_CHECKBOX,
    EV_RCLONE_OPT_ID_RESTORE_DEFAULTS
};

typedef struct ev_rclone_remote_s
{
    char name[256];
    char type[64];
    int is_indexed;
} ev_rclone_remote_t;

typedef struct ev_rclone_s
{
    everything_plugin_api_t api;
    int is_enabled;
    int auto_start;
    int index_on_start;
    char rclone_path[MAX_PATH];
    char rc_addr[256];
    int rc_port;
    PROCESS_INFORMATION rclone_proc_info;
    int rclone_running;
    HANDLE rclone_monitor_thread;
    int rclone_monitor_stop;
    ev_rclone_remote_t remotes[EV_RCLONE_MAX_REMOTES];
    int remote_count;
    CRITICAL_SECTION cs;
} ev_rclone_t;

extern ev_rclone_t g_rclone;

void ev_rclone_load_settings(void);
void ev_rclone_save_settings(void);

#endif
