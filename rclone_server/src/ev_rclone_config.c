#include "ev_rclone_config.h"
#include "ev_rclone_rc.h"
#include "ev_string_util.h"
#include <commdlg.h>
#include <commctrl.h>
#include <shlobj.h>
#include <string.h>

void ev_rclone_load_settings(void)
{
    const char *val;

    g_rclone.is_enabled = g_rclone.api.plugin_get_setting_int("enabled", 0);
    g_rclone.auto_start = g_rclone.api.plugin_get_setting_int("auto_start", 0);
    g_rclone.index_on_start = g_rclone.api.plugin_get_setting_int("index_on_start", 0);
    g_rclone.rc_port = g_rclone.api.plugin_get_setting_int("rc_port", EV_RCLONE_DEFAULT_RC_PORT);

    val = g_rclone.api.plugin_get_setting_string("rclone_path", "");
    ev_string_copy(g_rclone.rclone_path, val, MAX_PATH);

    val = g_rclone.api.plugin_get_setting_string("rc_addr", EV_RCLONE_DEFAULT_RC_ADDR);
    ev_string_copy(g_rclone.rc_addr, val, sizeof(g_rclone.rc_addr));
}

void ev_rclone_save_settings(void)
{
    g_rclone.api.plugin_set_setting_int("enabled", g_rclone.is_enabled);
    g_rclone.api.plugin_set_setting_int("auto_start", g_rclone.auto_start);
    g_rclone.api.plugin_set_setting_int("index_on_start", g_rclone.index_on_start);
    g_rclone.api.plugin_set_setting_int("rc_port", g_rclone.rc_port);
    g_rclone.api.plugin_set_setting_string("rclone_path", g_rclone.rclone_path);
    g_rclone.api.plugin_set_setting_string("rc_addr", g_rclone.rc_addr);
}

void ev_rclone_config_add_options_page(void *user_data)
{
    g_rclone.api.ui_options_add_plugin_page(EV_RCLONE_PLUGIN_NAME, user_data);
}

void ev_rclone_config_load_options_page(HWND hwnd, void *user_data)
{
    int y;
    int x;
    int label_w;
    int edit_w;
    int btn_w;
    int row_h;
    HWND ctl;

    y = 8;
    x = 8;
    label_w = 120;
    edit_w = 280;
    btn_w = 80;
    row_h = 26;

    ctl = g_rclone.api.os_create_checkbox(hwnd,
        EV_RCLONE_OPT_ID_ENABLED_CHECKBOX, 0,
        "Enable Rclone Plugin",
        x, y, edit_w + label_w, EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
    SendMessage(ctl, BM_SETCHECK, g_rclone.is_enabled ? BST_CHECKED : BST_UNCHECKED, 0);
    y += row_h;

    g_rclone.api.os_create_static(hwnd,
        EV_RCLONE_OPT_ID_RCLONE_PATH_STATIC, 0,
        "Rclone Path:",
        x, y + 3, label_w, EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);

    g_rclone.api.os_create_edit(hwnd,
        EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT, 0,
        x + label_w, y, edit_w - btn_w - 8, EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);

    g_rclone.api.os_create_button(hwnd,
        EV_RCLONE_OPT_ID_RCLONE_PATH_BROWSE_BUTTON, 0,
        "Browse...",
        x + label_w + edit_w - btn_w, y, btn_w, EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);

    {
        everything_plugin_utf8_buf_t buf;
        g_rclone.api.utf8_buf_init(&buf);
        g_rclone.api.utf8_buf_copy_string(&buf, g_rclone.rclone_path);
        SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT, buf.buf);
        g_rclone.api.utf8_buf_kill(&buf);
    }
    y += row_h;

    g_rclone.api.os_create_static(hwnd,
        EV_RCLONE_OPT_ID_RC_ADDR_STATIC, 0,
        "RC Address:",
        x, y + 3, label_w, EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);

    g_rclone.api.os_create_edit(hwnd,
        EV_RCLONE_OPT_ID_RC_ADDR_EDIT, 0,
        x + label_w, y, edit_w, EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);

    {
        everything_plugin_utf8_buf_t buf;
        g_rclone.api.utf8_buf_init(&buf);
        g_rclone.api.utf8_buf_copy_string(&buf, g_rclone.rc_addr);
        SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RC_ADDR_EDIT, buf.buf);
        g_rclone.api.utf8_buf_kill(&buf);
    }
    y += row_h;

    g_rclone.api.os_create_static(hwnd,
        EV_RCLONE_OPT_ID_RC_PORT_STATIC, 0,
        "RC Port:",
        x, y + 3, label_w, EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);

    g_rclone.api.os_create_edit(hwnd,
        EV_RCLONE_OPT_ID_RC_PORT_EDIT, 0,
        x + label_w, y, 80, EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);

    {
        char port_str[16];
        ev_int_to_string(g_rclone.rc_port, port_str, sizeof(port_str));
        SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RC_PORT_EDIT, port_str);
    }
    y += row_h;

    ctl = g_rclone.api.os_create_checkbox(hwnd,
        EV_RCLONE_OPT_ID_AUTO_START_CHECKBOX, 0,
        "Auto start rclone daemon",
        x, y, edit_w + label_w, EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
    SendMessage(ctl, BM_SETCHECK, g_rclone.auto_start ? BST_CHECKED : BST_UNCHECKED, 0);
    y += row_h;

    ctl = g_rclone.api.os_create_checkbox(hwnd,
        EV_RCLONE_OPT_ID_INDEX_ON_START_CHECKBOX, 0,
        "Index cloud files on start",
        x, y, edit_w + label_w, EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
    SendMessage(ctl, BM_SETCHECK, g_rclone.index_on_start ? BST_CHECKED : BST_UNCHECKED, 0);
    y += row_h;

    g_rclone.api.os_create_button(hwnd,
        EV_RCLONE_OPT_ID_RESTORE_DEFAULTS, 0,
        "Restore Defaults",
        x, y, 120, EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
}

void ev_rclone_config_save_options_page(void *user_data)
{
    char buf[MAX_PATH];
    UINT checked;

    checked = IsDlgButtonChecked((HWND)user_data, EV_RCLONE_OPT_ID_ENABLED_CHECKBOX);
    g_rclone.is_enabled = (checked == BST_CHECKED) ? 1 : 0;

    GetDlgItemTextA((HWND)user_data, EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT, buf, MAX_PATH);
    ev_string_copy(g_rclone.rclone_path, buf, MAX_PATH);

    GetDlgItemTextA((HWND)user_data, EV_RCLONE_OPT_ID_RC_ADDR_EDIT, buf, sizeof(buf));
    ev_string_copy(g_rclone.rc_addr, buf, sizeof(g_rclone.rc_addr));

    GetDlgItemTextA((HWND)user_data, EV_RCLONE_OPT_ID_RC_PORT_EDIT, buf, sizeof(buf));
    g_rclone.rc_port = ev_string_to_int(buf, EV_RCLONE_DEFAULT_RC_PORT);

    checked = IsDlgButtonChecked((HWND)user_data, EV_RCLONE_OPT_ID_AUTO_START_CHECKBOX);
    g_rclone.auto_start = (checked == BST_CHECKED) ? 1 : 0;

    checked = IsDlgButtonChecked((HWND)user_data, EV_RCLONE_OPT_ID_INDEX_ON_START_CHECKBOX);
    g_rclone.index_on_start = (checked == BST_CHECKED) ? 1 : 0;

    ev_rclone_save_settings();
}

void ev_rclone_config_get_options_page_minmax(void *user_data, int *min_width, int *min_height, int *max_width, int *max_height)
{
    if (min_width) *min_width = 400;
    if (min_height) *min_height = 200;
    if (max_width) *max_width = 0;
    if (max_height) *max_height = 0;
}

void ev_rclone_config_size_options_page(void *user_data, int width, int height)
{
}

INT_PTR ev_rclone_config_options_page_proc(void *user_data, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case EV_RCLONE_OPT_ID_RCLONE_PATH_BROWSE_BUTTON:
        {
            OPENFILENAMEA ofn;
            char filename[MAX_PATH];
            memset(&ofn, 0, sizeof(ofn));
            filename[0] = 0;
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = "Executables\0*.exe\0All Files\0*.*\0";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            ofn.lpstrTitle = "Select rclone.exe";
            if (GetOpenFileNameA(&ofn))
            {
                SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT, filename);
            }
            return TRUE;
        }
        case EV_RCLONE_OPT_ID_RESTORE_DEFAULTS:
        {
            SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RCLONE_PATH_EDIT, "rclone");
            SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RC_ADDR_EDIT, EV_RCLONE_DEFAULT_RC_ADDR);
            SetDlgItemTextA(hwnd, EV_RCLONE_OPT_ID_RC_PORT_EDIT, "5572");
            CheckDlgButton(hwnd, EV_RCLONE_OPT_ID_ENABLED_CHECKBOX, BST_UNCHECKED);
            CheckDlgButton(hwnd, EV_RCLONE_OPT_ID_AUTO_START_CHECKBOX, BST_UNCHECKED);
            CheckDlgButton(hwnd, EV_RCLONE_OPT_ID_INDEX_ON_START_CHECKBOX, BST_UNCHECKED);
            return TRUE;
        }
        }
        break;
    }
    return FALSE;
}

void ev_rclone_config_kill_options_page(void *user_data)
{
}
