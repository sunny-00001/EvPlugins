#ifndef EV_RCLONE_RC_H
#define EV_RCLONE_RC_H

#include "ev_rclone.h"
#include "ev_json.h"

typedef struct ev_rclone_rc_result_s
{
    int success;
    ev_json_value_t *json;
    char error[512];
} ev_rclone_rc_result_t;

int ev_rclone_rc_start(const char *rclone_path, const char *addr, int port);
void ev_rclone_rc_stop(void);
int ev_rclone_rc_is_running(void);
ev_rclone_rc_result_t ev_rclone_rc_call(const char *method, const ev_json_value_t *params);
void ev_rclone_rc_result_free(ev_rclone_rc_result_t *result);
int ev_rclone_rc_list_remotes(char remotes[][256], int max_remotes);
int ev_rclone_rc_list_dir(const char *remote_path, ev_json_value_t **out_list);

#endif
