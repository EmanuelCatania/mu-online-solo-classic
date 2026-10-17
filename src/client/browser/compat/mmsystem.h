#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char* HPSTR;
typedef DWORD FOURCC;
typedef struct {
    FOURCC ckid;
    DWORD cksize;
    FOURCC fccType;
    DWORD dwDataOffset;
    DWORD dwFlags;
} MMCKINFO;
typedef struct {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX;

#define WAVE_FORMAT_PCM 1
#define MMIO_READ 0x00000000
#define MMIO_ALLOCBUF 0x00010000
#define MMIO_FINDCHUNK 0x00000010
#define mmioFOURCC(a, b, c, d) \
    ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))

struct MuMmio {
    unsigned char* data;
    DWORD size;
    DWORD pos;
};
typedef MuMmio* HMMIO;

static inline int mu_mmio_chunk(MuMmio* m, DWORD at, MMCKINFO* ck) {
    if (!m || !ck || at + 8 > m->size) return 0;
    ck->ckid = *(DWORD*)(m->data + at);
    ck->cksize = *(DWORD*)(m->data + at + 4);
    ck->fccType = 0;
    ck->dwFlags = 0;
    ck->dwDataOffset = at + 8;
    if (ck->ckid == mmioFOURCC('R','I','F','F') || ck->ckid == mmioFOURCC('L','I','S','T')) {
        if (at + 12 > m->size) return 0;
        ck->fccType = *(DWORD*)(m->data + at + 8);
        ck->dwDataOffset = at + 12;
    }
    return 1;
}

static inline HMMIO mmioOpenA(const char* name, void* info, DWORD flags) {
    (void)info; (void)flags;
    if (!name) return 0;
    FILE* fp = fopen(name, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    long n = ftell(fp);
    if (n <= 0 || n > 8 * 1024 * 1024) { fclose(fp); return 0; }
    rewind(fp);
    unsigned char* data = (unsigned char*)malloc((size_t)n);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1, (size_t)n, fp) != (size_t)n) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    MuMmio* m = (MuMmio*)malloc(sizeof(MuMmio));
    if (!m) { free(data); return 0; }
    m->data = data;
    m->size = (DWORD)n;
    m->pos = 0;
    return m;
}
static inline MMRESULT mmioDescend(HMMIO h, MMCKINFO* ck, const MMCKINFO* parent, UINT flags) {
    (void)parent;
    if (!h || !ck) return 1;
    if (flags & MMIO_FINDCHUNK) {
        DWORD want = ck->ckid;
        DWORD at = h->pos;
        while (at + 8 <= h->size) {
            MMCKINFO cur;
            if (!mu_mmio_chunk(h, at, &cur)) return 1;
            if (cur.ckid == want) {
                *ck = cur;
                h->pos = cur.dwDataOffset;
                return 0;
            }
            DWORD next = cur.dwDataOffset + cur.cksize;
            if (next & 1u) next++;
            if (next <= at) return 1;
            at = next;
        }
        return 1;
    }
    if (!mu_mmio_chunk(h, h->pos, ck)) return 1;
    h->pos = ck->dwDataOffset;
    return 0;
}
static inline MMRESULT mmioAscend(HMMIO h, MMCKINFO* ck, UINT flags) {
    (void)flags;
    if (!h || !ck) return 0;
    DWORD next = ck->dwDataOffset + ck->cksize;
    if (next & 1u) next++;
    if (next > h->size) next = h->size;
    h->pos = next;
    return 0;
}
static inline LONG mmioRead(HMMIO h, HPSTR buf, LONG n) {
    if (!h || !buf || n < 0) return -1;
    if (h->pos >= h->size) return 0;
    DWORD have = h->size - h->pos;
    DWORD take = (DWORD)n < have ? (DWORD)n : have;
    memcpy(buf, h->data + h->pos, take);
    h->pos += take;
    return (LONG)take;
}
static inline MMRESULT mmioClose(HMMIO h, UINT flags) {
    (void)flags;
    if (!h) return 0;
    free(h->data);
    free(h);
    return 0;
}
