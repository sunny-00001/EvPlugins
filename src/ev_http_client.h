#ifndef EV_HTTP_CLIENT_H
#define EV_HTTP_CLIENT_H

#include <windows.h>

typedef struct ev_http_response_s
{
    int status_code;
    char *headers;
    uintptr_t headers_len;
    char *body;
    uintptr_t body_len;
} ev_http_response_t;

int ev_http_client_init(void);
void ev_http_client_cleanup(void);
ev_http_response_t *ev_http_post(const char *host, int port, const char *path, const char *content_type, const char *body, uintptr_t body_len);
ev_http_response_t *ev_http_get(const char *host, int port, const char *path);
void ev_http_response_free(ev_http_response_t *resp);

#endif
