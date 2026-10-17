// HWID.cpp — hardware fingerprint + F1/05 send path.
//
// See HWID.h for the format / why this is required. This is a 1:1 port of
// Mu-linux-97K/Source/Client/Main/HWID.cpp's computation, with the string
// obfuscation inlined (the companion client XORs the two format strings
// against 0xB0; we just use the plain literals since the bytes are public
// knowledge and match what the companion client produces at runtime).
//
// The packet on the wire is CSimpleModulus-encrypted (C3 frame), same as
// login — matches `PMSG_SET_HWID_SEND::header.setE(0xF1, 0x05, sizeof(...))`
// in the companion client.

#include "stdafx.h"
#include "Net/HWID.h"
#include "Net/Net.h"
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")

extern "C" void DbgLogPublic(const char* msg);

static char s_Hwid[36] = { 0 };  // "XXXXXXXX-XXXXXXXX-XXXXXXXX-XXXXXXXX"
static bool s_Ready   = false;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
bool HWID_Init(void)
{
    if (s_Ready) return true;

    char id[40];
    memset(id, 0, sizeof(id));
    EM_ASM({
        var key = "mu97k-hwid";
        var v = "";
        try { v = localStorage.getItem(key) || ""; } catch (e) { v = ""; }
        if (v.length != 35) {
            var hex = "0123456789ABCDEF";
            var a = "";
            var b = "";
            var c = "";
            var d = "";
            var p = 0;
            while (p < 4) {
                var s = "";
                var buf = new Uint8Array(4);
                crypto.getRandomValues(buf);
                var i = 0;
                while (i < 4) {
                    s = s + hex[buf[i] >> 4] + hex[buf[i] & 15];
                    i = i + 1;
                }
                if (p == 0) a = s;
                if (p == 1) b = s;
                if (p == 2) c = s;
                if (p == 3) d = s;
                p = p + 1;
            }
            v = a + "-" + b + "-" + c + "-" + d;
            try { localStorage.setItem(key, v); } catch (e2) {}
        }
        var n = v.length;
        if (n > 35) n = 35;
        var j = 0;
        while (j < n) {
            setValue($0 + j, v.charCodeAt(j), "i8");
            j = j + 1;
        }
        setValue($0 + n, 0, "i8");
    }, id);
    memcpy(s_Hwid, id, 36);
    s_Ready = true;
    {
        char line[128];
        wsprintfA(line, "HWID_Init: hwid='%s' storage=1", s_Hwid);
        DbgLogPublic(line);
    }
    return true;
}
#else
bool HWID_Init(void)
{
    if (s_Ready) return true;

    DWORD VolumeSerialNumber = 0;
    BOOL ok = GetVolumeInformationA("C:\\", nullptr, 0, &VolumeSerialNumber,
                                    nullptr, nullptr, nullptr, 0);

    // Fallback: if the volume read fails (e.g. C: not available), synthesize
    // a deterministic non-zero value so we still pass the empty-string check.
    if (!ok) VolumeSerialNumber = 0xDEADBEEF;

    UUID uuid = { 0 };
    UuidCreateSequential(&uuid);

    SYSTEM_INFO SystemInfo = { 0 };
    GetSystemInfo(&SystemInfo);

    DWORD id1 = VolumeSerialNumber ^ 0x12B586FE;
    DWORD id2 = *(DWORD*)(&uuid.Data4[2]) ^ 0x5D78A569;
    DWORD id3 = ((*(WORD*)(&uuid.Data4[6]) & 0xFFFF)
                 | (SystemInfo.wProcessorArchitecture << 16)) ^ 0xF45BC123;
    DWORD id4 = ((SystemInfo.wProcessorLevel & 0xFFFF)
                 | (SystemInfo.wProcessorRevision << 16)) ^ 0xB542D8E1;

    wsprintfA(s_Hwid, "%08X-%08X-%08X-%08X", id1, id2, id3, id4);
    s_Ready = true;

    char line[128];
    wsprintfA(line, "HWID_Init: hwid='%s' ok=%d", s_Hwid, (int)ok);
    DbgLogPublic(line);
    return ok ? true : false;
}
#endif

const char* HWID_Get(void)
{
    if (!s_Ready) HWID_Init();
    return s_Hwid;
}

// Packet layout on the wire matches PMSG_SET_HWID_SEND from the companion
// client (Protocol.h): [C3 framing] around 40 plaintext bytes:
//   pkt[0] = placeholder (gNetwork.Send replaces with C3)
//   pkt[1] = size (40, plaintext size incl. header)
//   pkt[2] = 0xF1 (main opcode)
//   pkt[3] = 0x05 (sub opcode)
//   pkt[4..39] = 36-byte HardwareId string
void HWID_Send(void)
{
    if (!s_Ready) HWID_Init();

    BYTE pkt[40] = { 0 };
    pkt[0] = 0xC1;     // placeholder, gNetwork.Send overwrites
    pkt[1] = 40;       // plaintext size
    pkt[2] = 0xF1;
    pkt[3] = 0x05;
    // 36 bytes of HardwareId (char[36] including the trailing NUL +padding)
    memcpy(pkt + 4, s_Hwid, 36);

    DbgLogPublic("HWID_Send: sending F1/05 packet (encrypted, 40 bytes plain)");
    gNetwork.Send(pkt, sizeof(pkt));
}

// F1/04 SetLanguage — companion (Protocol.cpp:771) calls SendLanguage() inside
// GCConnectClientRecv before SendHwid().  Without this packet the MuEmu server
// silently drops F1/01 (it expects the language to have been registered first).
// Layout per Protocol.h:382  PMSG_SET_LANG_SEND:
//   pkt[0] = 0xC1 placeholder (gNetwork.Send overwrites with C3)
//   pkt[1] = size (8, plaintext incl. header)
//   pkt[2] = 0xF1 (head)
//   pkt[3] = 0x04 (subh)
//   pkt[4..7] = int lang (LE DWORD)  ← 0=English, 1=Spanish, 2=Portuguese
void Lang_Send(int langNum)
{
    BYTE pkt[8] = { 0 };
    pkt[0] = 0xC1;     // placeholder (replaced with C3 in send wrapper)
    pkt[1] = 8;        // plaintext size (incl. 4-byte header)
    pkt[2] = 0xF1;
    pkt[3] = 0x04;
    memcpy(pkt + 4, &langNum, 4);

    char line[64];
    wsprintfA(line, "Lang_Send: sending F1/04 packet (lang=%d)", langNum);
    DbgLogPublic(line);
    gNetwork.Send(pkt, sizeof(pkt));
}
