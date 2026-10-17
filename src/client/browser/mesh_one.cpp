// M1: load one real .bmd with BMD__Open and draw mesh 0 through BMD__RenderMesh.
// Positions are the bind-pose vertices copied into the bone buffer the drawer reads.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"
#include "globals.h"

#include <stdio.h>
#include <string.h>
#include <new>

CWindow gWindow;

extern "C" void DbgLogPublic(const char* msg) {
    printf("DBG %s\n", msg ? msg : "");
}

FILE* __cdecl crt_fopen(const char* path, const void* mode) {
    return fopen(path, (const char*)mode);
}
void __cdecl crt_fclose(FILE* f) {
    if (f) fclose(f);
}
int __cdecl crt_sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsprintf(buf, fmt, args);
    va_end(args);
    return r;
}
void __cdecl operator_delete(void* ptr) {
    free(ptr);
}
void __cdecl putc(int ch, int* fp) {
    if (fp) fputc(ch, (FILE*)fp);
}
uint __cdecl crt_fwrite(char* buf, uint size, uint count, int* fp) {
    return (uint)fwrite(buf, size, count, (FILE*)fp);
}

// Static initializer in globals.cpp. The body is the CPhysicsManager ctor
// from Net/Crypto.cpp; that file is not in this link.
void* __fastcall FUN_00409ad0(void* param_1) {
    DWORD* This = (DWORD*)param_1;
    DWORD* head = (DWORD*)operator new(0xC);
    DWORD* tail = (DWORD*)operator new(0xC);
    head[0] = 0; head[1] = 0; head[2] = 0;
    tail[0] = 0; tail[1] = 0; tail[2] = 0;
    This[2] = (DWORD)(uintptr_t)head;
    This[3] = (DWORD)(uintptr_t)tail;
    head[2] = (DWORD)(uintptr_t)tail;
    tail[1] = (DWORD)(uintptr_t)head;
    This[1] = 0;
    return param_1;
}

void __cdecl BMD__Open(int param_1, int param_2, int param_3, int param_4);
void __cdecl BMD__RenderMesh(void* bmd_obj, float meshIdx, int flags, float alpha,
                             int blendMesh, float blendLight, float uvU, float uvV,
                             unsigned int texOverride);

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;
static unsigned char g_model[0x200];
static int g_ready = 0;
static int g_frames = 0;
static float g_min[3];
static float g_max[3];

static void copy_bind_pose(void) {
    short numMeshes = *(short*)(g_model + 0x24);
    int meshes = *(int*)(g_model + 0x28);
    int texIdx = *(int*)(g_model + 0x38);
    if (texIdx && numMeshes > 0) {
        memset((void*)texIdx, 0, (size_t)numMeshes * 2);
    }
    float minv[3] = { 1e30f, 1e30f, 1e30f };
    float maxv[3] = { -1e30f, -1e30f, -1e30f };
    int copied = 0;
    for (int m = 0; m < numMeshes; m++) {
        char* mesh = (char*)(meshes + m * 0x28);
        *(short*)(mesh + 2) = 0;
        short nV = *(short*)(mesh + 4);
        int vertBuf = *(int*)(mesh + 0x10);
        printf("MESH_ONE mesh=%d verts=%d normals=%d uvs=%d faces=%d\n",
               m, (int)nV, (int)*(short*)(mesh + 6),
               (int)*(short*)(mesh + 8), (int)*(short*)(mesh + 10));
        for (int i = 0; i < nV; i++) {
            float* src = (float*)(vertBuf + i * 0x10 + 4);
            float* dst = (float*)(g_BoneVertexBuf + ((i + m * 15000) * 12));
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            for (int k = 0; k < 3; k++) {
                if (src[k] < minv[k]) minv[k] = src[k];
                if (src[k] > maxv[k]) maxv[k] = src[k];
            }
            copied++;
        }
        if (m == 0) {
            char* faces = *(char**)(mesh + 0x1c);
            int nF = *(short*)(mesh + 10);
            printf("MESH_ONE face0 count=%d idx=%d,%d,%d\n",
                   faces ? (int)(unsigned char)faces[0] : -1,
                   faces ? (int)*(short*)(faces + 2) : -1,
                   faces ? (int)*(short*)(faces + 4) : -1,
                   faces ? (int)*(short*)(faces + 6) : -1);
            if (nF > 0 && faces) {
                short i0 = *(short*)(faces + 2);
                float* p = (float*)(g_BoneVertexBuf + i0 * 12);
                printf("MESH_ONE v0=(%.2f,%.2f,%.2f)\n", p[0], p[1], p[2]);
            }
        }
    }
    g_min[0] = minv[0]; g_min[1] = minv[1]; g_min[2] = minv[2];
    g_max[0] = maxv[0]; g_max[1] = maxv[1]; g_max[2] = maxv[2];
    printf("MESH_ONE copied=%d bbox=(%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f)\n",
           copied, minv[0], minv[1], minv[2], maxv[0], maxv[1], maxv[2]);
    fflush(stdout);
}

static int count_lit_pixels(void) {
    const int n = 640 * 480;
    unsigned char* px = (unsigned char*)malloc((size_t)n * 4);
    if (!px) return -1;
    memset(px, 0, (size_t)n * 4);
    glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, px);
    int lit = 0;
    for (int i = 0; i < n; i++) {
        unsigned char r = px[i * 4 + 0];
        unsigned char g = px[i * 4 + 1];
        unsigned char b = px[i * 4 + 2];
        if (g > 180 && r < 80 && b < 80) lit++;
    }
    printf("MESH_ONE center=%u,%u,%u,%u\n",
           px[(240 * 640 + 320) * 4 + 0],
           px[(240 * 640 + 320) * 4 + 1],
           px[(240 * 640 + 320) * 4 + 2],
           px[(240 * 640 + 320) * 4 + 3]);
    free(px);
    fflush(stdout);
    return lit;
}

static void frame(void) {
    emscripten_webgl_make_context_current(g_ctx);
    // emscripten_webgl_create_context does not run the legacy-GL context
    // callback, so the immediate-mode matrix stack stays uninitialized.
    EM_ASM({
        if (typeof GLImmediate !== 'undefined' && !GLImmediate.matrixVersion) {
            Browser.useWebGL = true;
            GLImmediate.init();
        }
    });
    if (!g_ready) {
        memset(g_model, 0, sizeof(g_model));
        const char* path = "Data/Item/";
        const char* name = "Axe01.bmd";
        BMD__Open((int)(intptr_t)g_model, (int)(intptr_t)path, (int)(intptr_t)name, 0);
        short numMeshes = *(short*)(g_model + 0x24);
        printf("MESH_ONE open meshes=%d name=%.32s\n", (int)numMeshes, (char*)g_model);
        if (numMeshes <= 0) {
            printf("MESH_ONE FAIL load\n");
            emscripten_cancel_main_loop();
            return;
        }
        *(char*)(g_model + 0x44) = 0;
        *(char*)(g_model + 0x88) = -1;
        *(float*)(g_model + 0x48) = 0.0f;
        *(float*)(g_model + 0x4c) = 1.0f;
        *(float*)(g_model + 0x50) = 0.0f;
        copy_bind_pose();
        g_ready = 1;
    }

    printf("MESH_ONE before_clear\n");
    fflush(stdout);
    glViewport(0, 0, 640, 480);
    glClearColor(0.6f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    printf("MESH_ONE after_clear err=%d\n", (int)glGetError());
    fflush(stdout);
    float mx = (g_max[0] - g_min[0]) * 0.08f;
    float my = (g_max[1] - g_min[1]) * 0.08f;
    if (mx < 1.0f) mx = 1.0f;
    if (my < 1.0f) my = 1.0f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(g_min[0] - mx, g_max[0] + mx, g_min[1] - my, g_max[1] + my, -500.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // The drawer turns culling on through its state cache. Leave both sides
    // visible so a winding mismatch cannot hide the only mesh.
    DAT_083a411c = 1;
    glDisable(GL_CULL_FACE);
    printf("MESH_ONE before_draw\n");
    fflush(stdout);

    BMD__RenderMesh(g_model, 0.0f, 1, 1.0f, -1, 1.0f, 0.0f, 0.0f, 0xffffffffu);
    printf("MESH_ONE after_draw err=%d\n", (int)glGetError());
    fflush(stdout);

    int lit = count_lit_pixels();
    printf("MESH_ONE frame=%d lit=%d\n", g_frames, lit);
    g_frames++;
    if (g_frames >= 3) {
        printf("MESH_ONE %s lit=%d\n", lit > 50 ? "VISIBLE" : "NOT_VISIBLE", lit);
        emscripten_cancel_main_loop();
    }
}

int main(void) {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.minorVersion = 0;
    attr.alpha = EM_FALSE;
    attr.depth = EM_TRUE;
    attr.stencil = EM_FALSE;
    attr.antialias = EM_FALSE;

    g_ctx = emscripten_webgl_create_context("#canvas", &attr);
    if (g_ctx <= 0) {
        printf("MESH_ONE context_failed=%d\n", (int)g_ctx);
        return 1;
    }
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
