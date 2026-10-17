#pragma once
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Sound/SoundManager.h"

static inline float mu_spatial_gain(float x, float y, float z) {
    float d2 = x * x + y * y + z * z;
    if (d2 <= 1.0f) return 1.0f;
    float g = 1.0f / sqrtf(d2);
    return g < 0.03f ? 0.0f : g;
}

struct IDirectSoundBuffer;

struct IDirectSound3DBuffer {
    IDirectSoundBuffer* owner;
    HRESULT SetPosition(float x, float y, float z, DWORD m);
    unsigned long Release() { return 0; }
};

struct IDirectSoundBuffer {
    unsigned char* pcm;
    DWORD cap;
    DWORD rate;
    DWORD channels;
    DWORD bits;
    long volume;
    float spatialGain;
    int looping;
    IDirectSound3DBuffer spatial;
    IDirectSoundBuffer()
        : pcm(0), cap(0), rate(22050), channels(2), bits(16), volume(0), spatialGain(1.0f), looping(0), spatial() {
        spatial.owner = this;
    }
    HRESULT Play(DWORD, DWORD, DWORD flags) {
        if (!pcm || !cap) return E_FAIL;
        looping = (flags & 1) ? 1 : 0;
        return gSound.PlayPcm(this, pcm, (int)cap, (int)rate, (int)channels, (int)bits,
                              volume, spatialGain, looping)
            ? S_OK : E_FAIL;
    }
    HRESULT Stop() {
        gSound.StopPcm(this);
        return S_OK;
    }
    HRESULT SetCurrentPosition(DWORD) { return S_OK; }
    HRESULT SetVolume(long v) {
        volume = v;
        gSound.SetPcmVolume(this, v);
        return S_OK;
    }
    HRESULT SetFormat(const void* fmt) { (void)fmt; return S_OK; }
    HRESULT Lock(DWORD o, DWORD n, void** p1, DWORD* s1, void** p2, DWORD* s2, DWORD f) {
        (void)f;
        if (p2) *p2 = 0;
        if (s2) *s2 = 0;
        if (n == 0 || o > 1024u * 1024u) return E_FAIL;
        DWORD need = o + n;
        if (need > cap) {
            unsigned char* grown = (unsigned char*)realloc(pcm, need);
            if (!grown) return E_FAIL;
            if (need > cap) memset(grown + cap, 0, need - cap);
            pcm = grown;
            cap = need;
        }
        if (p1) *p1 = pcm + o;
        if (s1) *s1 = n;
        return S_OK;
    }
    HRESULT Unlock(void* p1, DWORD s1, void* p2, DWORD s2) {
        (void)p1; (void)s1; (void)p2; (void)s2; return S_OK;
    }
    HRESULT QueryInterface(const GUID& id, void** out);
    unsigned long Release() {
        gSound.StopPcm(this);
        free(pcm);
        pcm = 0;
        delete this;
        return 0;
    }
};
struct IDirectSound3DListener {
    unsigned long Release() { return 0; }
};
struct DSBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    void* lpwfxFormat;
};
struct IDirectSound {
    HRESULT SetCooperativeLevel(HWND w, DWORD level) { (void)w; (void)level; return S_OK; }
    HRESULT CreateSoundBuffer(const DSBUFFERDESC* desc, IDirectSoundBuffer** out, void* u) {
        (void)u;
        if (!out) return E_FAIL;
        IDirectSoundBuffer* buf = new IDirectSoundBuffer();
        if (desc && desc->lpwfxFormat) {
            const unsigned char* f = (const unsigned char*)desc->lpwfxFormat;
            buf->channels = *(const unsigned short*)(f + 2);
            buf->rate = *(const unsigned int*)(f + 4);
            buf->bits = *(const unsigned short*)(f + 14);
        }
        if (desc && desc->dwBufferBytes) {
            buf->pcm = (unsigned char*)calloc(1, desc->dwBufferBytes);
            if (!buf->pcm) { delete buf; *out = 0; return E_FAIL; }
            buf->cap = desc->dwBufferBytes;
        }
        *out = buf;
        return S_OK;
    }
    unsigned long Release() { return 0; }
};

typedef IDirectSound* LPDIRECTSOUND;
typedef IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef IDirectSound3DBuffer* LPDIRECTSOUND3DBUFFER;
typedef IDirectSound3DListener* LPDIRECTSOUND3DLISTENER;

#define DSBPLAY_LOOPING 1
#define DS3D_DEFERRED 1
#define DSSCL_PRIORITY 0x00000002
#define DSBCAPS_PRIMARYBUFFER 0x00000001
#define DSBCAPS_CTRL3D 0x00000010
#define DSBCAPS_CTRLVOLUME 0x00000080
#define DSBCAPS_STATIC 0x00000002
#define DSBCAPS_CTRLFREQUENCY 0x00000020
#define DSBCAPS_CTRLPAN 0x00000040
#define DSBVOLUME_MAX 0
#define DSBVOLUME_MIN (-10000)
static const GUID IID_IDirectSound3DListener = { 1, 0, 0, {0} };
static const GUID IID_IDirectSound3DBuffer = { 2, 0, 0, {0} };

static inline int mu_guid_eq(const GUID& a, const GUID& b) {
    return a.Data1 == b.Data1 && a.Data2 == b.Data2 && a.Data3 == b.Data3;
}

inline HRESULT IDirectSound3DBuffer::SetPosition(float x, float y, float z, DWORD m) {
    (void)m;
    if (!owner) return S_OK;
    owner->spatialGain = mu_spatial_gain(x, y, z);
    gSound.SetPcmPosition(owner, x, y, z);
    return S_OK;
}

inline HRESULT IDirectSoundBuffer::QueryInterface(const GUID& id, void** out) {
    if (!out) return E_FAIL;
    *out = 0;
    if (mu_guid_eq(id, IID_IDirectSound3DListener)) {
        static IDirectSound3DListener listener;
        *out = &listener;
        return S_OK;
    }
    if (mu_guid_eq(id, IID_IDirectSound3DBuffer)) {
        spatial.owner = this;
        *out = &spatial;
        return S_OK;
    }
    return E_FAIL;
}

static inline HRESULT DirectSoundCreate(const GUID* g, LPDIRECTSOUND* out, void* u) {
    (void)g; (void)u;
    static IDirectSound device;
    if (out) *out = &device;
    return S_OK;
}
