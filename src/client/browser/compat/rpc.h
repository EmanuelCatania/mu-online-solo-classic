#pragma once
#include <windows.h>
static inline int UuidCreateSequential(UUID* u) {
    if (!u) return 0;
    memset(u, 0, sizeof(*u));
    u->Data4[2] = 1; u->Data4[3] = 2; u->Data4[4] = 3; u->Data4[5] = 4;
    u->Data4[6] = 5; u->Data4[7] = 6;
    return 0;
}
