#pragma once
#include <windows.h>
typedef enum {
    MiniDumpNormal = 0,
    MiniDumpScanMemory = 0x00000010,
    MiniDumpWithIndirectlyReferencedMemory = 0x00000040
} MINIDUMP_TYPE;
typedef struct {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION;
typedef MINIDUMP_EXCEPTION_INFORMATION* PMINIDUMP_EXCEPTION_INFORMATION;
typedef void* PMINIDUMP_USER_STREAM_INFORMATION;
typedef void* PMINIDUMP_CALLBACK_INFORMATION;
