#ifndef EV_RCLONE_CONFIG_H
#define EV_RCLONE_CONFIG_H

#include "ev_rclone.h"

void ev_rclone_load_settings(void);
void ev_rclone_save_settings(void);
void ev_rclone_config_add_options_page(void *user_data);
void ev_rclone_config_load_options_page(HWND hwnd, void *user_data);
void ev_rclone_config_save_options_page(void *user_data);
void ev_rclone_config_get_options_page_minmax(void *user_data, int *min_width, int *min_height, int *max_width, int *max_height);
void ev_rclone_config_size_options_page(void *user_data, int width, int height);
INT_PTR ev_rclone_config_options_page_proc(void *user_data, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ev_rclone_config_kill_options_page(void *user_data);

#endif
