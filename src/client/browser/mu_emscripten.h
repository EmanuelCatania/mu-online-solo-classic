#pragma once
// Included first on the Emscripten path, before the game headers.
#define __int64 long long
#define __int16 short
#define __int32 int
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#define __try if (1)
#define __except(filter) if (0)

// The client builds paths with '\\'. MEMFS separates on '/'.
#include <cstdio>
#include <string.h>
inline FILE* mu_fopen_slash(const char* path, const char* mode) {
    if (!path) return NULL;
    char fixed[512];
    size_t n = 0;
    for (; path[n] && n + 1 < sizeof(fixed); ++n)
        fixed[n] = (path[n] == '\\') ? '/' : path[n];
    fixed[n] = 0;
    FILE* f = std::fopen(fixed, mode ? mode : "rb");
    if (f || !mode || (mode[0] != 'r' && mode[0] != 'R')) return f;
    char* dot = strrchr(fixed, '.');
    if (!dot || !dot[1]) return NULL;
    for (char* p = dot + 1; *p; ++p) {
        if (*p >= 'a' && *p <= 'z') *p = (char)(*p - 32);
        else if (*p >= 'A' && *p <= 'Z') *p = (char)(*p + 32);
    }
    return std::fopen(fixed, mode);
}
#define fopen(path, mode) mu_fopen_slash(path, mode)
#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#endif
