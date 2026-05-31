#include "ev_http_client.h"
#include "ev_string_util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

static int g_wsa_initialized = 0;

int ev_http_client_init(void)
{
    WSADATA wsaData;
    if (g_wsa_initialized) return 1;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;
    g_wsa_initialized = 1;
    return 1;
}

void ev_http_client_cleanup(void)
{
    if (g_wsa_initialized)
    {
        WSACleanup();
        g_wsa_initialized = 0;
    }
}

static ev_http_response_t *ev_http_response_alloc(void)
{
    ev_http_response_t *resp = (ev_http_response_t *)calloc(1, sizeof(ev_http_response_t));
    return resp;
}

void ev_http_response_free(ev_http_response_t *resp)
{
    if (!resp) return;
    free(resp->headers);
    free(resp->body);
    free(resp);
}

static SOCKET ev_http_connect(const char *host, int port)
{
    SOCKET sock;
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    char port_str[16];
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    _snprintf(port_str, sizeof(port_str), "%d", port);
    rc = getaddrinfo(host, port_str, &hints, &result);
    if (rc != 0 || !result) return INVALID_SOCKET;

    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) { freeaddrinfo(result); return INVALID_SOCKET; }

    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        closesocket(sock);
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return sock;
}

static int ev_http_send_all(SOCKET sock, const char *data, uintptr_t len)
{
    uintptr_t total = 0;
    while (total < len)
    {
        int sent = send(sock, data + total, (int)(len - total), 0);
        if (sent == SOCKET_ERROR) return 0;
        total += sent;
    }
    return 1;
}

static char *ev_http_recv_all(SOCKET sock, uintptr_t *out_len)
{
    uintptr_t capacity = 65536;
    uintptr_t total = 0;
    char *buf = (char *)malloc(capacity);
    if (!buf) return NULL;

    for (;;)
    {
        int received;
        if (total >= capacity - 1)
        {
            capacity *= 2;
            buf = (char *)realloc(buf, capacity);
            if (!buf) return NULL;
        }
        received = recv(sock, buf + total, (int)(capacity - total - 1), 0);
        if (received <= 0) break;
        total += received;
    }

    buf[total] = 0;
    *out_len = total;
    return buf;
}

static int ev_http_parse_response(const char *raw, uintptr_t raw_len, ev_http_response_t *resp)
{
    const char *header_end;
    uintptr_t header_len;
    const char *body_start;
    uintptr_t body_len;

    header_end = strstr(raw, "\r\n\r\n");
    if (!header_end) return 0;

    header_len = (uintptr_t)(header_end - raw);
    body_start = header_end + 4;
    body_len = raw_len - header_len - 4;

    resp->headers = (char *)malloc(header_len + 1);
    if (!resp->headers) return 0;
    memcpy(resp->headers, raw, header_len);
    resp->headers[header_len] = 0;
    resp->headers_len = header_len;

    if (body_len > 0)
    {
        const char *cl_header = strstr(resp->headers, "Content-Length:");
        if (cl_header)
        {
            uintptr_t content_length;
            cl_header += 15;
            while (*cl_header == ' ') cl_header++;
            content_length = (uintptr_t)atol(cl_header);
            if (content_length < body_len) body_len = content_length;
        }
        resp->body = (char *)malloc(body_len + 1);
        if (!resp->body) return 0;
        memcpy(resp->body, body_start, body_len);
        resp->body[body_len] = 0;
        resp->body_len = body_len;
    }
    else
    {
        resp->body = (char *)calloc(1, 1);
        resp->body_len = 0;
    }

    if (raw_len > 9 && ev_string_starts_with(raw, "HTTP/"))
    {
        const char *sp = strchr(raw + 5, ' ');
        if (sp) resp->status_code = atoi(sp + 1);
    }

    return 1;
}

ev_http_response_t *ev_http_post(const char *host, int port, const char *path, const char *content_type, const char *body, uintptr_t body_len)
{
    SOCKET sock;
    char request_header[4096];
    int header_len;
    char *raw_response;
    uintptr_t raw_len;
    ev_http_response_t *resp;

    if (!ev_http_client_init()) return NULL;

    sock = ev_http_connect(host, port);
    if (sock == INVALID_SOCKET) return NULL;

    header_len = _snprintf(request_header, sizeof(request_header),
        "POST %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %llu\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, port, content_type ? content_type : "application/json",
        (unsigned long long)body_len);

    if (!ev_http_send_all(sock, request_header, header_len))
    {
        closesocket(sock);
        return NULL;
    }

    if (body && body_len > 0)
    {
        if (!ev_http_send_all(sock, body, body_len))
        {
            closesocket(sock);
            return NULL;
        }
    }

    raw_response = ev_http_recv_all(sock, &raw_len);
    closesocket(sock);

    if (!raw_response) return NULL;

    resp = ev_http_response_alloc();
    if (!resp) { free(raw_response); return NULL; }

    if (!ev_http_parse_response(raw_response, raw_len, resp))
    {
        free(raw_response);
        ev_http_response_free(resp);
        return NULL;
    }

    free(raw_response);
    return resp;
}

ev_http_response_t *ev_http_get(const char *host, int port, const char *path)
{
    SOCKET sock;
    char request_header[4096];
    int header_len;
    char *raw_response;
    uintptr_t raw_len;
    ev_http_response_t *resp;

    if (!ev_http_client_init()) return NULL;

    sock = ev_http_connect(host, port);
    if (sock == INVALID_SOCKET) return NULL;

    header_len = _snprintf(request_header, sizeof(request_header),
        "GET %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, port);

    if (!ev_http_send_all(sock, request_header, header_len))
    {
        closesocket(sock);
        return NULL;
    }

    raw_response = ev_http_recv_all(sock, &raw_len);
    closesocket(sock);

    if (!raw_response) return NULL;

    resp = ev_http_response_alloc();
    if (!resp) { free(raw_response); return NULL; }

    if (!ev_http_parse_response(raw_response, raw_len, resp))
    {
        free(raw_response);
        ev_http_response_free(resp);
        return NULL;
    }

    free(raw_response);
    return resp;
}
