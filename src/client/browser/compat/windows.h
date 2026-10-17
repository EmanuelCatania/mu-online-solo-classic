#pragma once
// Browser stand-ins for the Win32 headers stdafx.h and the game headers include.
// Present only on the Emscripten include path. The native build never sees this file.

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

double emscripten_get_now(void);

typedef uint8_t BYTE;
typedef uint8_t byte;
typedef uint16_t WORD;
typedef short SHORT;
typedef uint32_t DWORD;
typedef uintptr_t DWORD_PTR;
typedef int32_t LONG;
typedef int32_t INT;
typedef uint32_t UINT;
typedef uint32_t ULONG;
typedef int BOOL;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t INT_PTR;
typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LRESULT;
typedef uintptr_t SOCKET;
typedef long HRESULT;
typedef long LSTATUS;

typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HFONT;
typedef void* HGDIOBJ;
typedef void* HBITMAP;
typedef void* HMODULE;
typedef void* HMENU;
typedef void* HKEY;
typedef HKEY* PHKEY;
typedef BYTE* LPBYTE;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef const char* LPCTSTR;
typedef char* LPTSTR;
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef DWORD* LPDWORD;
typedef WORD ATOM;
typedef UINT MMRESULT;
typedef unsigned short u_short;
typedef unsigned long u_long;

struct tagRECT { LONG left, top, right, bottom; };
typedef struct tagRECT RECT;
typedef RECT* LPRECT;
struct tagPOINT { LONG x, y; };
typedef struct tagPOINT POINT;
struct tagSIZE { LONG cx, cy; };
typedef struct tagSIZE SIZE;
typedef SIZE* LPSIZE;

struct tagWNDCLASSA {
    UINT style;
    LRESULT (*lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
};
typedef struct tagWNDCLASSA WNDCLASSA;
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

struct _LARGE_INTEGER { long long QuadPart; };
typedef struct _LARGE_INTEGER LARGE_INTEGER;

struct _GUID { DWORD Data1; WORD Data2; WORD Data3; BYTE Data4[8]; };
typedef struct _GUID GUID;
typedef GUID UUID;

struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
};
typedef struct WSAData WSADATA;

struct sockaddr { unsigned short sa_family; char sa_data[14]; };
struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct {
        union {
            struct { uint32_t S_addr; } S_un;
            uint32_t s_addr;
        };
    } sin_addr;
    char sin_zero[8];
};
typedef struct hostent HOSTENT;
struct hostent {
    char* h_name;
    char** h_aliases;
    short h_addrtype;
    short h_length;
    char** h_addr_list;
};

#define WINAPI
#define CALLBACK
#define APIENTRY
#define WINAPIV
#define FAR
#define NEAR
#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL 0
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define MAX_PATH 260
#define _MAX_PATH 260
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define GENERIC_READ 0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 0x00000001u
#define FILE_SHARE_WRITE 0x00000002u
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_ATTRIBUTE_NORMAL 0x00000080u
#define FILE_ATTRIBUTE_READONLY 0x00000001u
#define NORMAL_PRIORITY_CLASS 0x00000020
#define REALTIME_PRIORITY_CLASS 0x00000100
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_TIME_CRITICAL 15
#define INVALID_SOCKET ((SOCKET)(~(SOCKET)0))
#define SOCKET_ERROR (-1)
#define S_OK 0L
#define E_FAIL 0x80004005L
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
/* LOWORD/HIWORD come from ghidra_compat.h as lvalues. */
#define AF_INET 2
#define SOCK_STREAM 1
#define IPPROTO_TCP 6
#define INADDR_NONE 0xFFFFFFFFu
#define WSAEWOULDBLOCK 10035
#define WM_USER 0x0400
#define WM_DESTROY 2
#define WM_CLOSE 0x10
#define WM_TIMER 0x113
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MOUSEWHEEL 0x020A
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define DEFAULT_CHARSET 1
#define ANSI_CHARSET 0
#define NONANTIALIASED_QUALITY 3
#define ANTIALIASED_QUALITY 4
#define CLEARTYPE_QUALITY 5
#define FW_NORMAL 400
#define FW_BOLD 700
#define ERROR_ALREADY_EXISTS 183L
#define TIMERR_NOERROR 0
#define TIMER_MODIFY_STATE 0x0002
#define SYNCHRONIZE 0x00100000L
#define WAIT_OBJECT_0 0

typedef void* LPSECURITY_ATTRIBUTES;
typedef const wchar_t* LPCWSTR;
typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay;
    WORD wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME;

#define __forceinline inline
#define __declspec(x)

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

extern short Mu_AsyncKeyState[256];
static inline DWORD GetTickCount(void) { return (DWORD)emscripten_get_now(); }
static inline DWORD timeGetTime(void) { return GetTickCount(); }
static inline void Sleep(DWORD ms) { (void)ms; }
static inline BOOL QueryPerformanceCounter(LARGE_INTEGER* c) {
    if (c) c->QuadPart = (long long)(emscripten_get_now() * 1000.0);
    return 1;
}
static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* c) {
    if (c) c->QuadPart = 1000000;
    return 1;
}
static inline short GetAsyncKeyState(int key) {
    if ((unsigned)key >= 256) return 0;
    return Mu_AsyncKeyState[key];
}
static inline void GetLocalTime(SYSTEMTIME* st) { if (st) memset(st, 0, sizeof(*st)); }
static inline BOOL CreateDirectoryA(LPCSTR path, void* sec) { (void)path; (void)sec; return 1; }
static inline DWORD GetLastError(void) { return 0; }
struct MuFileHandle { unsigned magic; FILE* fp; };
static inline HANDLE CreateFileA(const char* name, DWORD access, DWORD share, void* sec,
                                 DWORD creation, DWORD attr, HANDLE templ) {
    (void)share; (void)sec; (void)attr; (void)templ;
    const char* mode = "rb";
    if (creation == CREATE_ALWAYS || creation == CREATE_NEW) mode = "wb";
    else if (access & GENERIC_WRITE) mode = "r+b";
    char fixed[512];
    const char* openName = name;
    if (name) {
        size_t i = 0;
        for (; name[i] && i + 1 < sizeof(fixed); ++i)
            fixed[i] = (name[i] == '\\') ? '/' : name[i];
        fixed[i] = 0;
        openName = fixed;
    }
    FILE* fp = openName ? fopen(openName, mode) : 0;
    if (!fp) return INVALID_HANDLE_VALUE;
    MuFileHandle* h = (MuFileHandle*)malloc(sizeof(MuFileHandle));
    if (!h) { fclose(fp); return INVALID_HANDLE_VALUE; }
    h->magic = 0x4D554649u;
    h->fp = fp;
    return h;
}
static inline BOOL ReadFile(HANDLE h, void* buf, DWORD n, DWORD* got, void* ov) {
    (void)ov;
    MuFileHandle* f = (MuFileHandle*)h;
    if (!f || f->magic != 0x4D554649u || !buf) return 0;
    size_t r = fread(buf, 1, n, f->fp);
    if (got) *got = (DWORD)r;
    return 1;
}
static inline BOOL WriteFile(HANDLE h, const void* buf, DWORD n, DWORD* got, void* ov) {
    (void)ov;
    MuFileHandle* f = (MuFileHandle*)h;
    if (!f || f->magic != 0x4D554649u || !buf) return 0;
    size_t r = fwrite(buf, 1, n, f->fp);
    if (got) *got = (DWORD)r;
    return r == n;
}
static inline BOOL DeleteFileA(const char* path) { return path && remove(path) == 0; }
static inline BOOL CloseHandle(HANDLE h) {
    MuFileHandle* f = (MuFileHandle*)h;
    if (f && f->magic == 0x4D554649u) {
        if (f->fp) fclose(f->fp);
        f->magic = 0;
        free(f);
    }
    return 1;
}
static inline int lstrlenA(const char* s) { return s ? (int)strlen(s) : 0; }
#ifdef __EMSCRIPTEN__
extern "C" int Mu_GlyphAdvanceSum(const char* s, int n);
extern "C" void* mu_font_create(int height, int weight, int italic, const char* face);
extern "C" void* mu_gdi_select_font(void* obj);
extern "C" void* mu_gdi_current_font(void);
extern "C" int mu_font_query(void* obj, int* height, int* weight, int* italic, char* face, int faceCap);
#endif
static inline BOOL GetTextExtentPointA(HDC dc, const char* s, int n, LPSIZE sz) {
    (void)dc;
    if (sz) {
#ifdef __EMSCRIPTEN__
        sz->cx = Mu_GlyphAdvanceSum(s, n);
        int height = 0;
        if (!mu_font_query(mu_gdi_current_font(), &height, 0, 0, 0, 0) || height < 8)
            height = 16;
        sz->cy = height + 3;
#else
        (void)s;
        sz->cx = (n > 0 ? n : 0) * 8;
        sz->cy = 16;
#endif
    }
    return 1;
}
static inline BOOL GetTextExtentPoint32A(HDC dc, const char* s, int n, LPSIZE sz) {
    return GetTextExtentPointA(dc, s, n, sz);
}
static inline int _vsnprintf_s(char* buf, size_t bufsz, size_t count, const char* fmt, va_list ap) {
    (void)count;
    if (!buf || bufsz == 0) return -1;
    return vsnprintf(buf, bufsz, fmt, ap);
}
static inline MMRESULT timeBeginPeriod(UINT p) { (void)p; return 0; }
static inline MMRESULT timeEndPeriod(UINT p) { (void)p; return 0; }
typedef void* FARPROC;
static inline FARPROC GetProcAddress(HMODULE m, const char* n) { (void)m; (void)n; return 0; }
static inline HMODULE GetModuleHandleW(const wchar_t* n) { (void)n; return 0; }
static inline BOOL SetWaitableTimer(HANDLE h, const LARGE_INTEGER* d, LONG per, void* cb, void* arg, BOOL resume) {
    (void)h; (void)d; (void)per; (void)cb; (void)arg; (void)resume; return 0;
}
static inline DWORD WaitForSingleObject(HANDLE h, DWORD ms) { (void)h; (void)ms; return WAIT_OBJECT_0; }
static inline int wsprintfA(char* buf, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vsprintf(buf, fmt, ap);
    va_end(ap);
    return n;
}
static inline int MessageBoxA(HWND w, LPCSTR t, LPCSTR c, UINT y) { (void)w; (void)t; (void)c; (void)y; return 0; }
SOCKET mu_net_socket(int, int, int);
int mu_net_connect(SOCKET, const void*, int);
int mu_net_closesocket(SOCKET);
int mu_net_send(SOCKET, const char*, int, int);
int mu_net_recv(SOCKET, char*, int, int);
int mu_net_async_select(SOCKET, HWND, unsigned, long);
int mu_net_last_error(void);
uint32_t mu_net_inet_addr(const char*);
struct hostent* mu_net_gethostbyname(const char*);

static inline int WSAStartup(WORD v, WSADATA* d) { (void)v; (void)d; return 0; }
static inline int WSACleanup(void) { return 0; }
static inline int WSAGetLastError(void) { return mu_net_last_error(); }
static inline SOCKET socket(int a, int b, int c) { return mu_net_socket(a, b, c); }
static inline int connect(SOCKET s, const void* n, int l) { return mu_net_connect(s, n, l); }
static inline int closesocket(SOCKET s) { return mu_net_closesocket(s); }
static inline int shutdown(SOCKET s, int h) { (void)s; (void)h; return 0; }
static inline unsigned short htons(unsigned short x) { return (unsigned short)((x << 8) | (x >> 8)); }
static inline uint32_t inet_addr(const char* s) { return mu_net_inet_addr(s); }
static inline struct hostent* gethostbyname(const char* n) { return mu_net_gethostbyname(n); }
static inline int WSAAsyncSelect(SOCKET s, HWND w, unsigned m, long e) { return mu_net_async_select(s, w, m, e); }
static inline int recv(SOCKET s, char* b, int l, int f) { return mu_net_recv(s, b, l, f); }
static inline int send(SOCKET s, const char* b, int l, int f) { return mu_net_send(s, b, l, f); }
static inline DWORD GetCurrentDirectoryA(DWORD n, LPSTR b) { (void)n; if (b && n) b[0] = 0; return 0; }
static inline BOOL SwapBuffers(HDC dc) { (void)dc; return 1; }
#ifdef __cplusplus
extern "C" {
#endif
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef __cplusplus
}
#endif

static inline LRESULT SendMessageA(HWND w, UINT m, WPARAM wp, LPARAM lp) {
    return WndProc(w, m, wp, lp);
}
static inline HGDIOBJ SelectObject(HDC dc, HGDIOBJ obj) {
    (void)dc;
#ifdef __EMSCRIPTEN__
    return (HGDIOBJ)mu_gdi_select_font(obj);
#else
    (void)obj;
    return 0;
#endif
}
static inline HWND SetCapture(HWND w) { (void)w; return 0; }
static inline BOOL ReleaseCapture(void) { return 1; }
static inline char* lstrcpynA(char* dst, const char* src, int n) {
    if (!dst || n <= 0) return dst;
    if (!src) { dst[0] = 0; return dst; }
    int i = 0;
    for (; i < n - 1 && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
    return dst;
}
typedef void* HIMC;
typedef void* HKL;
static inline HIMC ImmGetContext(HWND w) { (void)w; return 0; }
static inline BOOL ImmReleaseContext(HWND w, HIMC h) { (void)w; (void)h; return 0; }
static inline HKL GetKeyboardLayout(DWORD id) { (void)id; return 0; }
static inline UINT ImmGetDescriptionA(HKL h, LPSTR b, UINT n) { (void)h; if (b && n) b[0] = 0; return 0; }
static inline UINT ImmGetIMEFileNameA(HKL h, LPSTR b, UINT n) { (void)h; if (b && n) b[0] = 0; return 0; }
static inline BOOL GetKeyboardLayoutNameA(LPSTR b) { if (b) b[0] = 0; return 0; }
static inline void __cpuid(int regs[4], int leaf) {
    (void)leaf;
    if (regs) { regs[0] = 0; regs[1] = 0; regs[2] = 0; regs[3] = 0; }
}
static inline unsigned long long __rdtsc(void) { return 0; }
static inline HANDLE GetCurrentProcess(void) { return (HANDLE)1; }
static inline HANDLE GetCurrentThread(void) { return (HANDLE)2; }
static inline DWORD GetPriorityClass(HANDLE h) { (void)h; return 0x20; }
static inline int GetThreadPriority(HANDLE h) { (void)h; return 0; }
static inline BOOL GetProcessAffinityMask(HANDLE h, DWORD_PTR* p, DWORD_PTR* s) {
    (void)h; if (p) *p = 1; if (s) *s = 1; return 1;
}
static inline BOOL SetPriorityClass(HANDLE h, DWORD c) { (void)h; (void)c; return 1; }
static inline BOOL SetThreadPriority(HANDLE h, int p) { (void)h; (void)p; return 1; }
static inline BOOL SetProcessAffinityMask(HANDLE h, DWORD_PTR m) { (void)h; (void)m; return 1; }
typedef struct _CONTEXT {
    DWORD ContextFlags;
    DWORD Eax, Ebx, Ecx, Edx, Esi, Edi, Ebp, Eip, Esp;
} CONTEXT;
#define CONTEXT_FULL 0x10007
static inline BOOL GetThreadContext(HANDLE h, CONTEXT* c) { (void)h; (void)c; return 0; }
static inline BOOL IsBadReadPtr(const void* p, UINT n) { (void)p; (void)n; return 1; }
static inline int ShowCursor(BOOL show) { (void)show; return 0; }
static inline BOOL IsDebuggerPresent(void) { return 0; }
static inline void OutputDebugStringA(const char* s) { (void)s; }

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    DWORD NumberParameters;
    DWORD_PTR ExceptionInformation[15];
} EXCEPTION_RECORD;
typedef struct _EXCEPTION_POINTERS {
    EXCEPTION_RECORD* ExceptionRecord;
    CONTEXT* ContextRecord;
} EXCEPTION_POINTERS;
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_EXECUTION (-1)
static inline BOOL SetCurrentDirectoryA(const char* path) { (void)path; return 1; }
typedef void (__cdecl *MuInvalidParamHandler)(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t);
static inline MuInvalidParamHandler _set_invalid_parameter_handler(MuInvalidParamHandler handler) {
    (void)handler; return 0;
}
#define SEM_FAILCRITICALERRORS 0x0001
static inline UINT SetErrorMode(UINT mode) { (void)mode; return 0; }
typedef LONG (WINAPI *LPTOP_LEVEL_EXCEPTION_FILTER)(EXCEPTION_POINTERS*);
static inline LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER filter) {
    (void)filter; return 0;
}
static inline UINT_PTR SetTimer(HWND hwnd, UINT_PTR id, UINT elapse, void* proc) {
    (void)hwnd; (void)id; (void)elapse; (void)proc; return id;
}
#define PM_NOREMOVE 0
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105

typedef unsigned short USHORT;
typedef uintptr_t HANDLE_PTR;
typedef void* LPOVERLAPPED;
typedef uintptr_t HCRYPTPROV;
typedef void* HKEY;
#define HKEY_CURRENT_USER ((HKEY)(uintptr_t)1)
#define KEY_ALL_ACCESS 0xF003F
#define ERROR_SUCCESS 0
#define REG_SZ 1
#define CLIP_DEFAULT_PRECIS 0
#define OUT_DEFAULT_PRECIS 0
#define DEFAULT_PITCH 0
#define FF_DONTCARE 0
#define WM_ACTIVATE 0x0006
#define WM_SETCURSOR 0x0020
#define WM_DESTROY 0x0002
#define WM_CLOSE 0x0010
#define WM_TIMER 0x0113
#define WM_USER 0x0400
#define VK_TAB 0x09
#define VK_SPACE 0x20
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define CLEARTYPE_NATURAL_QUALITY 6
#define BI_RGB 0
#define DIB_RGB_COLORS 0
#define TRANSPARENT 1
#define OPAQUE 2
#define MB_OK 0x00000000
#define MB_ICONERROR 0x00000010
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define PM_REMOVE 0x0001
#define FD_READ 0x01
#define FD_WRITE 0x02
#define FD_OOB 0x04
#define FD_ACCEPT 0x08
#define FD_CONNECT 0x10
#define FD_CLOSE 0x20
#define OBJ_FONT 6
#define RGB(r, g, b) ((DWORD)(((BYTE)(r) | ((WORD)(BYTE)(g) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define _countof(a) (sizeof(a) / sizeof((a)[0]))
#define FindText FindTextA
#define MS_DEF_PROV_A "Microsoft Base Cryptographic Provider v1.0"

typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG;
typedef struct tagTEXTMETRICA {
    LONG tmHeight, tmAscent, tmDescent, tmInternalLeading, tmExternalLeading;
    LONG tmAveCharWidth, tmMaxCharWidth, tmWeight, tmOverhang;
    LONG tmDigitizedAspectX, tmDigitizedAspectY;
    BYTE tmFirstChar, tmLastChar, tmDefaultChar, tmBreakChar;
    BYTE tmItalic, tmUnderlined, tmStruckOut, tmPitchAndFamily, tmCharSet;
} TEXTMETRICA;

extern "C" unsigned mu_profile_string(const char* sec, const char* key, const char* def,
                                      char* out, unsigned n, const char* file);
extern "C" unsigned mu_profile_section(const char* sec, char* out, unsigned n, const char* file);
extern "C" int mu_profile_write(const char* sec, const char* key, const char* val, const char* file);
extern "C" int mu_file_exists(const char* path);

static inline DWORD GetPrivateProfileStringA(const char* sec, const char* key, const char* def,
                                             char* out, DWORD n, const char* file) {
    return mu_profile_string(sec, key, def, out, n, file);
}
static inline char* GetCommandLineA(void) { return (char*)""; }
static inline BOOL TextOutA(HDC dc, int x, int y, const char* s, int n) {
    (void)dc; (void)x; (void)y; (void)s; (void)n; return 1;
}
static inline DWORD SetTextColor(HDC dc, DWORD c) { (void)dc; (void)c; return 0; }
static inline BOOL ImmGetConversionStatus(HIMC h, DWORD* conv, DWORD* sent) {
    (void)h; if (conv) *conv = 0; if (sent) *sent = 0; return 0;
}
static inline BOOL ImmSetConversionStatus(HIMC h, DWORD conv, DWORD sent) {
    (void)h; (void)conv; (void)sent; return 0;
}
static inline HWND ImmGetDefaultIMEWnd(HWND w) { (void)w; return 0; }
static inline BOOL SetRect(LPRECT r, int l, int t, int ri, int b) {
    if (r) { r->left = l; r->top = t; r->right = ri; r->bottom = b; }
    return 1;
}
static inline BOOL DestroyWindow(HWND w) {
    SendMessageA(w, 2 /* WM_DESTROY */, 0, 0);
    return 1;
}
static inline void PostQuitMessage(int c) {
    (void)c;
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    EM_ASM({
        if (typeof window !== 'undefined' && window.soloExitGame) {
            window.soloExitGame();
        } else if (typeof window !== 'undefined') {
            window.location.reload();
        }
    });
#endif
}
static inline HCURSOR SetCursor(HCURSOR c) { (void)c; return 0; }
static inline DWORD GetFileAttributesA(const char* p) {
    return mu_file_exists(p) ? FILE_ATTRIBUTE_NORMAL : INVALID_FILE_ATTRIBUTES;
}
static inline char* lstrcatA(char* d, const char* s) { return d && s ? strcat(d, s) : d; }
static inline int wvsprintfA(char* b, const char* f, va_list ap) { return vsprintf(b, f, ap); }
static inline BOOL IsDBCSLeadByteEx(UINT cp, BYTE c) { (void)cp; (void)c; return 0; }
static inline DWORD GetModuleFileNameA(HMODULE m, char* b, DWORD n) {
    (void)m;
    const char* s = "main.exe";
    if (!b || n == 0) return 0;
    DWORD i = 0;
    for (; i + 1 < n && s[i]; ++i) b[i] = s[i];
    b[i] = 0;
    return i;
}
static inline BOOL wglUseFontBitmapsA(HDC dc, DWORD first, DWORD count, DWORD list) {
    (void)dc; (void)first; (void)count; (void)list; return 0;
}
static inline HGLRC wglGetCurrentContext(void) { return 0; }
static inline HRESULT CoInitialize(void* u) { (void)u; return S_OK; }
static inline void CoUninitialize(void) {}
static inline char* _itoa(int v, char* buf, int radix) {
    if (!buf) return buf;
    if (radix == 16) sprintf(buf, "%x", v);
    else sprintf(buf, "%d", v);
    return buf;
}
static inline LONG RegOpenKeyExA(HKEY k, const char* sub, DWORD opt, DWORD sam, HKEY* out) {
    (void)k; (void)sub; (void)opt; (void)sam; if (out) *out = 0; return 2;
}
static inline LONG RegQueryValueExA(HKEY k, const char* name, DWORD* res, DWORD* type, BYTE* data, DWORD* cb) {
    (void)k; (void)name; (void)res; (void)type; (void)data; (void)cb; return 2;
}
static inline LONG RegCloseKey(HKEY k) { (void)k; return 0; }
static inline BOOL PeekMessageA(MSG* m, HWND w, UINT a, UINT b, UINT r) {
    (void)m; (void)w; (void)a; (void)b; (void)r; return 0;
}
static inline BOOL GetMessageA(MSG* m, HWND w, UINT a, UINT b) {
    (void)m; (void)w; (void)a; (void)b; return 0;
}
static inline BOOL TranslateMessage(const MSG* m) { (void)m; return 0; }
static inline LRESULT DispatchMessageA(const MSG* m) { (void)m; return 0; }
static inline LRESULT DefWindowProcA(HWND w, UINT m, WPARAM wp, LPARAM lp) {
    (void)w; (void)m; (void)wp; (void)lp; return 0;
}
static inline HFONT CreateFontA(int h, int w, int esc, int ori, int weight, DWORD ital, DWORD under,
                                DWORD strike, DWORD charset, DWORD outPrec, DWORD clip, DWORD quality,
                                DWORD pitch, const char* face) {
    (void)w; (void)esc; (void)ori; (void)under;
    (void)strike; (void)charset; (void)outPrec; (void)clip; (void)quality; (void)pitch;
#ifdef __EMSCRIPTEN__
    return (HFONT)mu_font_create(h, weight, ital ? 1 : 0, face);
#else
    (void)h; (void)weight; (void)ital; (void)face;
    return (HFONT)1;
#endif
}
static inline BOOL GetTextMetricsA(HDC dc, TEXTMETRICA* tm) {
    (void)dc;
    if (!tm) return 0;
    memset(tm, 0, sizeof(*tm));
    int height = 0;
    int weight = 400;
    int italic = 0;
#ifdef __EMSCRIPTEN__
    if (!mu_font_query(mu_gdi_current_font(), &height, &weight, &italic, nullptr, 0) || height < 8)
        height = 16;
#else
    height = 16;
#endif
    tm->tmHeight = height + 3;
    tm->tmAscent = height > 1 ? height - 1 : height;
    tm->tmDescent = 4;
    tm->tmAveCharWidth = height / 2;
    tm->tmMaxCharWidth = height;
    tm->tmWeight = weight;
    tm->tmItalic = (BYTE)italic;
    return 1;
}
static inline DWORD GetPrivateProfileSectionA(const char* sec, char* out, DWORD n, const char* file) {
    return mu_profile_section(sec, out, n, file);
}
static inline LONG RegCreateKeyExA(HKEY k, const char* sub, DWORD r, char* cls, DWORD opt, DWORD sam,
                                   void* sec, HKEY* out, DWORD* disp) {
    (void)k; (void)sub; (void)r; (void)cls; (void)opt; (void)sam; (void)sec;
    if (out) *out = 0; if (disp) *disp = 0; return 2;
}
static inline BOOL GetVolumeInformationA(const char* root, char* name, DWORD nameN, DWORD* serial,
                                         DWORD* compLen, DWORD* flags, char* fs, DWORD fsN) {
    (void)root; (void)name; (void)nameN; (void)serial; (void)compLen; (void)flags; (void)fs; (void)fsN;
    return 0;
}
typedef struct _SYSTEM_INFO {
    WORD wProcessorArchitecture;
    WORD wReserved;
    DWORD dwPageSize;
    void* lpMinimumApplicationAddress;
    void* lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO;
static inline void GetSystemInfo(SYSTEM_INFO* s) { if (s) memset(s, 0, sizeof(*s)); }
static inline HMODULE LoadLibraryA(const char* name) { (void)name; return 0; }
static inline DWORD GetCurrentThreadId(void) { return 1; }
static inline DWORD GetCurrentProcessId(void) { return 1; }
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define PROV_RSA_FULL 1
#define CRYPT_VERIFYCONTEXT 0xF0000000u
#define CRYPT_NEWKEYSET 0x00000008u
static inline BOOL CryptAcquireContextA(HCRYPTPROV* p, const char* c, const char* prov, DWORD type, DWORD flags) {
    (void)c; (void)prov; (void)type; (void)flags; if (p) *p = 0; return 0;
}
typedef uintptr_t HCRYPTHASH;
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    DWORD bmiColors[1];
} BITMAPINFO;
static inline HBITMAP CreateDIBSection(HDC dc, const BITMAPINFO* bmi, UINT usage, void** bits, HANDLE section, DWORD offset) {
    (void)dc; (void)bmi; (void)usage; (void)section; (void)offset;
    if (bits) *bits = 0;
    return 0;
}
static inline HDC CreateCompatibleDC(HDC dc) { (void)dc; return (HDC)1; }
static inline int SetBkMode(HDC dc, int mode) { (void)dc; (void)mode; return 0; }
static inline DWORD GetFileVersionInfoSizeA(const char* file, DWORD* handle) {
    (void)file; if (handle) *handle = 0; return 0;
}
static inline BOOL GetFileVersionInfoA(const char* file, DWORD handle, DWORD size, void* data) {
    (void)file; (void)handle; (void)size; (void)data; return 0;
}
static inline BOOL WritePrivateProfileStringA(const char* sec, const char* key, const char* val, const char* file) {
    return mu_profile_write(sec, key, val, file) ? 1 : 0;
}
static inline BOOL CopyFileA(const char* src, const char* dst, BOOL failIfExists) {
    (void)src; (void)dst; (void)failIfExists; return 0;
}
static inline HGDIOBJ GetCurrentObject(HDC dc, UINT kind) {
    (void)dc;
#ifdef __EMSCRIPTEN__
    if (kind == OBJ_FONT) return (HGDIOBJ)mu_gdi_current_font();
#else
    (void)kind;
#endif
    return 0;
}
static inline BOOL DeleteObject(HGDIOBJ obj) { (void)obj; return 1; }
static inline BOOL DeleteDC(HDC dc) { (void)dc; return 1; }
typedef struct {
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;
static inline BOOL VerQueryValueA(const void* block, const char* sub, void** buf, UINT* len) {
    (void)block; (void)sub; if (buf) *buf = 0; if (len) *len = 0; return 0;
}
typedef uintptr_t HCRYPTKEY;
static inline BOOL CryptDestroyHash(HCRYPTHASH h) { (void)h; return 1; }
static inline BOOL CryptDestroyKey(HCRYPTKEY h) { (void)h; return 1; }
static inline BOOL CryptReleaseContext(HCRYPTPROV h, DWORD flags) { (void)h; (void)flags; return 1; }
typedef struct tagLOGFONTA {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[32];
} LOGFONTA;
typedef int (*FONTENUMPROCA)(const LOGFONTA*, const TEXTMETRICA*, DWORD, LPARAM);
static inline HDC GetDC(HWND w) { (void)w; return (HDC)1; }
static inline int ReleaseDC(HWND w, HDC dc) { (void)w; (void)dc; return 1; }
static inline int EnumFontFamiliesExA(HDC dc, LOGFONTA* lf, FONTENUMPROCA proc, LPARAM param, DWORD flags) {
    (void)dc; (void)lf; (void)proc; (void)param; (void)flags; return 1;
}
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
static inline DWORD SetFilePointer(HANDLE h, LONG dist, LONG* high, DWORD method) {
    (void)high; (void)method;
    MuFileHandle* f = (MuFileHandle*)h;
    if (!f || f->magic != 0x4D554649u) return 0;
    int origin = method == FILE_END ? SEEK_END : method == FILE_CURRENT ? SEEK_CUR : SEEK_SET;
    if (fseek(f->fp, dist, origin) != 0) return (DWORD)-1;
    return (DWORD)ftell(f->fp);
}
typedef void* PEXCEPTION_POINTERS;

#ifdef __cplusplus
}
#endif

#define _snprintf_s(buf, size, count, ...) snprintf((buf), (size), __VA_ARGS__)
#define PeekMessage PeekMessageA
#define DispatchMessage DispatchMessageA
#ifdef __cplusplus
template <size_t N>
inline void strcpy_s(char (&dst)[N], const char* src) {
    snprintf(dst, N, "%s", src ? src : "");
}
inline void strcpy_s(char* dst, size_t n, const char* src) {
    if (!dst || n == 0) return;
    snprintf(dst, n, "%s", src ? src : "");
}
template <size_t N, typename... Args>
inline int sprintf_s(char (&buf)[N], const char* fmt, Args... args) {
    return snprintf(buf, N, fmt, args...);
}
template <typename... Args>
inline int sprintf_s(char* buf, size_t n, const char* fmt, Args... args) {
    return snprintf(buf, n, fmt, args...);
}
#endif
#define strcat_s(dst, n, src) strncat((dst), (src), (n) - strlen(dst) - 1)
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define strtok_s strtok_r
#define _access access
#define _strdup strdup
static inline void Mu_strncpy_s(char* dst, size_t dstsz, const char* src, size_t count) {
    size_t i = 0;
    if (!dst || dstsz == 0) return;
    if (!src) { dst[0] = 0; return; }
    if (count >= dstsz) count = dstsz - 1;
    for (; i < count && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}
#ifdef __cplusplus
template <size_t N>
inline void strncpy_s(char (&dst)[N], const char* src, size_t count) {
    Mu_strncpy_s(dst, N, src, count);
}
inline void strncpy_s(char* dst, size_t dstsz, const char* src, size_t count) {
    Mu_strncpy_s(dst, dstsz, src, count);
}
template <typename T>
inline T min(T a, T b) { return a < b ? a : b; }
template <typename T>
inline T max(T a, T b) { return a > b ? a : b; }
#endif
