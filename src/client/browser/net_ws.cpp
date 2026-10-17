// Browser socket for M4. Game packets stay in the C++ client.
// This file only moves the bytes the client already sends and receives.

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

#include "stdafx.h"
#include "Net/Network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static EMSCRIPTEN_WEBSOCKET_T g_ws = 0;
static int g_open = 0;
static int g_err = 0;
static char g_host[128];
// The content catalog arrives as one TCP burst before F1/01. The game
// reads at most 8 KB per FD_READ, so a single 64 KB socket buffer dropped
// the tail, including the login result, and the panel waited forever.
static unsigned char* g_in = 0;
static int g_in_cap = 0;
static int g_in_len = 0;
static int g_sends = 0;

static void note_host(const char* s) {
    if (!s || !s[0]) return;
    strncpy(g_host, s, sizeof(g_host) - 1);
    g_host[sizeof(g_host) - 1] = 0;
}

static void post_event(unsigned evt) {
    gNetwork.OnSocketEvent((WORD)evt, 0);
}

static bool on_open(int, const EmscriptenWebSocketOpenEvent*, void*) {
    g_open = 1;
    g_err = 0;
    printf("M4 ws open host=%s\n", g_host);
    fflush(stdout);
    post_event(FD_WRITE);
    return true;
}

static int append_in(const unsigned char* data, int n) {
    if (n <= 0) return 1;
    if (g_in_len > 0x4000000 || n > 0x4000000) {
        printf("M4 recv overflow\n");
        fflush(stdout);
        return 0;
    }
    if (g_in_len + n > g_in_cap) {
        int cap = g_in_cap > 0 ? g_in_cap : 65536;
        while (cap < g_in_len + n) {
            if (cap > 0x2000000) {
                cap = g_in_len + n;
                break;
            }
            cap *= 2;
        }
        unsigned char* grown = (unsigned char*)realloc(g_in, (size_t)cap);
        if (!grown) {
            printf("M4 recv overflow\n");
            fflush(stdout);
            return 0;
        }
        g_in = grown;
        g_in_cap = cap;
    }
    memcpy(g_in + g_in_len, data, (size_t)n);
    g_in_len += n;
    return 1;
}

// Windows posts FD_READ again while bytes remain. One call here reads 8 KB
// and returns, so keep going until this message is in the client buffer.
static void drain_in() {
    for (int i = 0; i < 8192 && g_in_len > 0; ++i) {
        int before = g_in_len;
        post_event(FD_READ);
        if (g_in_len >= before) break;
    }
}

static bool on_message(int, const EmscriptenWebSocketMessageEvent* ev, void*) {
    if (!ev || ev->socket != g_ws || ev->isText || !ev->data || ev->numBytes == 0) return true;
    if (!append_in(ev->data, (int)ev->numBytes)) return true;
    drain_in();
    return true;
}

static bool on_close(int, const EmscriptenWebSocketCloseEvent* ev, void*) {
    if (!ev || ev->socket != g_ws) return true;
    g_open = 0;
    g_ws = 0;
    printf("M4 ws close\n");
    fflush(stdout);
    post_event(FD_CLOSE);
    return true;
}

static bool on_error(int, const EmscriptenWebSocketErrorEvent*, void*) {
    g_err = 10061;
    printf("M4 ws error\n");
    fflush(stdout);
    return true;
}

extern "C" int mu_net_last_error(void) { return g_err; }

extern "C" SOCKET mu_net_socket(int, int, int) {
    g_err = 0;
    return (SOCKET)1;
}

extern "C" uint32_t mu_net_inet_addr(const char* s) {
    note_host(s);
    if (!s) return INADDR_NONE;
    unsigned parts[4];
    int n = 0;
    const char* p = s;
    while (*p && n < 4) {
        if (*p < '0' || *p > '9') return INADDR_NONE;
        unsigned v = 0;
        while (*p >= '0' && *p <= '9') {
            v = v * 10u + (unsigned)(*p - '0');
            if (v > 255u) return INADDR_NONE;
            p++;
        }
        parts[n++] = v;
        if (n < 4) {
            if (*p != '.') return INADDR_NONE;
            p++;
        }
    }
    if (n != 4 || *p) return INADDR_NONE;
    return parts[0] | (parts[1] << 8) | (parts[2] << 16) | (parts[3] << 24);
}

extern "C" struct hostent* mu_net_gethostbyname(const char* name) {
    note_host(name);
    static char addr[4] = { 127, 0, 0, 1 };
    static char* list[2] = { addr, 0 };
    static struct hostent h;
    h.h_name = g_host;
    h.h_aliases = 0;
    h.h_addrtype = AF_INET;
    h.h_length = 4;
    h.h_addr_list = list;
    g_err = 0;
    return &h;
}

extern "C" int mu_net_async_select(SOCKET, HWND, unsigned, long) {
    g_err = 0;
    return 0;
}

extern "C" int mu_net_connect(SOCKET, const void* addr, int len) {
    if (!addr || len < (int)sizeof(sockaddr_in)) {
        g_err = 10022;
        return -1;
    }
    const sockaddr_in* sa = (const sockaddr_in*)addr;
    unsigned short port = (unsigned short)((sa->sin_port << 8) | (sa->sin_port >> 8));
    if (!g_host[0]) strncpy(g_host, "127.0.0.1", sizeof(g_host) - 1);

    if (g_ws) {
        EMSCRIPTEN_WEBSOCKET_T old = g_ws;
        g_ws = 0;
        g_open = 0;
        emscripten_websocket_close(old, 1000, "switch");
        emscripten_websocket_delete(old);
    }
    g_open = 0;
    g_in_len = 0;

    char url[256];
    char pageHost[128] = {};
    // The browser opens the gateway on whatever host served the page, so a
    // phone on the LAN reaches this machine. The query host stays 127.0.0.1:
    // the gateway dials the game servers on this PC, not the public address.
    EM_ASM({
        var h = (typeof location !== "undefined" && location.hostname) ? location.hostname : "";
        if (!h || h.length > 120) h = "127.0.0.1";
        stringToUTF8(h, $0, 128);
    }, pageHost);
    if (!pageHost[0]) strncpy(pageHost, "127.0.0.1", sizeof(pageHost) - 1);
    snprintf(url, sizeof(url), "ws://%s:8091/?host=127.0.0.1&port=%u", pageHost, (unsigned)port);
    printf("M4 connect %s asked=%s\n", url, g_host);
    fflush(stdout);

    EmscriptenWebSocketCreateAttributes attr;
    emscripten_websocket_init_create_attributes(&attr);
    attr.url = url;
    attr.createOnMainThread = true;
    g_ws = emscripten_websocket_new(&attr);
    if (g_ws <= 0) {
        g_err = 10061;
        g_ws = 0;
        return -1;
    }
    emscripten_websocket_set_onopen_callback(g_ws, 0, on_open);
    emscripten_websocket_set_onmessage_callback(g_ws, 0, on_message);
    emscripten_websocket_set_onclose_callback(g_ws, 0, on_close);
    emscripten_websocket_set_onerror_callback(g_ws, 0, on_error);
    g_err = WSAEWOULDBLOCK;
    return -1;
}

extern "C" int mu_net_closesocket(SOCKET) {
    if (g_ws) {
        EMSCRIPTEN_WEBSOCKET_T old = g_ws;
        g_ws = 0;
        emscripten_websocket_close(old, 1000, "close");
        emscripten_websocket_delete(old);
    }
    g_open = 0;
    g_in_len = 0;
    g_err = 0;
    return 0;
}

extern "C" int mu_net_send(SOCKET, const char* buf, int len, int) {
    if (!g_open || g_ws <= 0) {
        g_err = WSAEWOULDBLOCK;
        return -1;
    }
    if (len <= 0) return 0;
    if (emscripten_websocket_send_binary(g_ws, (void*)buf, (uint32_t)len) != EMSCRIPTEN_RESULT_SUCCESS) {
        g_err = 10054;
        return -1;
    }
    if (g_sends < 8) {
        printf("M4 send %d\n", len);
        fflush(stdout);
    }
    g_sends++;
    g_err = 0;
    return len;
}

extern "C" int mu_net_recv(SOCKET, char* buf, int len, int) {
    if (g_in_len <= 0) {
        g_err = WSAEWOULDBLOCK;
        return -1;
    }
    if (!buf || len <= 0) return 0;
    int n = g_in_len < len ? g_in_len : len;
    memcpy(buf, g_in, (size_t)n);
    memmove(g_in, g_in + n, (size_t)(g_in_len - n));
    g_in_len -= n;
    g_err = 0;
    return n;
}
