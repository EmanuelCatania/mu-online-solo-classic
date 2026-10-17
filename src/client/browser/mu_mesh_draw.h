#pragma once

// One immediate-mode batch per mesh. The vertex bytes match what
// glTexCoord / glColor / glVertex would have written, so the picture stays
// the same and the browser gets one copy instead of a call per vertex.

enum {
    MU_MESH_UV = 0,
    MU_MESH_UV_COLOR = 1,
    MU_MESH_COLOR_UV = 2,
    MU_MESH_POS = 3
};

enum { MU_MESH_BYTES = 2 * 1024 * 1024 - 64 };

struct MuMeshW {
    int layout;
    int n;
    int phase;
    int bad;
    int hasColor;
    float lastR, lastG, lastB, lastA;
    unsigned char* p;
    unsigned char* end;
    unsigned char* base;
};

extern MuMeshW g_muMesh;

void mu_mesh_start(int layout);
// Terrain and other runs whose matrix stays put. A matrix change still
// closes the run, because glPushMatrix flushes before it applies.
void mu_mesh_start_stable(int layout);
bool mu_mesh_finish();
extern "C" void mu_mesh_flush();

// Four corners become two triangles. uv is 8 floats, xyz is 12, rgba is
// 16 (r,g,b,a per corner) or null to bake the current color into each one.
bool mu_mesh_emit4(int layout, int stable, const float* uv, const float* rgba, const float* xyz);

// Floor tiles keep their world-space vertices. mu_tile_begin returns 1 when
// the tile was replayed, 2 when this draw should be recorded, 0 to draw live.
int mu_tile_begin(int x, int y);
void mu_tile_anim(void);
void mu_tile_end(int x, int y);
void mu_tile_commit(void);

#include <string.h>

static inline float mu_mesh_clamp(float c) {
    if (c < 0.f) return 0.f;
    if (c > 1.f) return 1.f;
    return c;
}

static inline void mu_mesh_tex(float u, float v) {
    MuMeshW* g = &g_muMesh;
    if (g->bad) return;
    int expect = (g->layout == MU_MESH_COLOR_UV) ? 1 : 0;
    if (g->layout == MU_MESH_POS || g->phase != expect) { g->bad = 1; return; }
    if (g->p + 8 > g->end) { g->bad = 1; return; }
    memcpy(g->p, &u, 4);
    memcpy(g->p + 4, &v, 4);
    g->p += 8;
    g->phase++;
}

static inline void mu_mesh_rgba(float r, float g, float b, float a) {
    MuMeshW* s = &g_muMesh;
    if (s->bad) return;
    int expect = (s->layout == MU_MESH_UV_COLOR) ? 1 : 0;
    if (s->layout == MU_MESH_UV || s->layout == MU_MESH_POS || s->phase != expect) {
        s->bad = 1;
        return;
    }
    if (s->p + 4 > s->end) { s->bad = 1; return; }
    r = mu_mesh_clamp(r);
    g = mu_mesh_clamp(g);
    b = mu_mesh_clamp(b);
    a = mu_mesh_clamp(a);
    s->p[0] = (unsigned char)(r * 255.f);
    s->p[1] = (unsigned char)(g * 255.f);
    s->p[2] = (unsigned char)(b * 255.f);
    s->p[3] = (unsigned char)(a * 255.f);
    s->p += 4;
    s->lastR = r;
    s->lastG = g;
    s->lastB = b;
    s->lastA = a;
    s->hasColor = 1;
    s->phase++;
}

static inline void mu_mesh_xyz(float x, float y, float z) {
    MuMeshW* g = &g_muMesh;
    if (g->bad) return;
    int expect = 2;
    if (g->layout == MU_MESH_POS) expect = 0;
    else if (g->layout == MU_MESH_UV) expect = 1;
    if (g->phase != expect) { g->bad = 1; return; }
    if (g->p + 16 > g->end) { g->bad = 1; return; }
    float w = 1.f;
    memcpy(g->p, &x, 4);
    memcpy(g->p + 4, &y, 4);
    memcpy(g->p + 8, &z, 4);
    memcpy(g->p + 12, &w, 4);
    g->p += 16;
    g->n++;
    g->phase = 0;
}
