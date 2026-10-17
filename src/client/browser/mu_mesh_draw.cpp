#include <emscripten/emscripten.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"
#include "mu_mesh_draw.h"

MuMeshW g_muMesh;
static unsigned char* g_muMeshBytes = nullptr;

static int g_open = 0;
static int g_layout = 0;
static int g_tex = 0;
static int g_blend = 0;
static int g_depth = 0;
static int g_cull = 0;
static int g_alphaTest = 0;
static int g_useColor = 0;
static float g_color[4] = { 1.f, 1.f, 1.f, 1.f };
static float g_matrix[16];
static unsigned char* g_markP = nullptr;
static int g_markN = 0;

static int mu_stride(int layout) {
    if (layout == MU_MESH_UV) return 24;
    if (layout == MU_MESH_POS) return 16;
    return 28;
}

static void mu_set_flag(int open, int drawing) {
    EM_ASM({
        Module._muBatchOpen = $0 ? 1 : 0;
        Module._muBatchDrawing = $1 ? 1 : 0;
    }, open, drawing);
}

static void mu_capture_matrix() {
    EM_ASM({
        var m = (typeof GLImmediate !== "undefined" && GLImmediate.matrix) ? GLImmediate.matrix[0] : null;
        var p = $0 >> 2;
        if (!m) {
            for (var i = 0; i < 16; i++) HEAPF32[p + i] = (i % 5 == 0) ? 1 : 0;
            return;
        }
        for (var i = 0; i < 16; i++) HEAPF32[p + i] = m[i];
    }, g_matrix);
}

static int mu_matrix_same() {
    return EM_ASM_INT({
        var m = (typeof GLImmediate !== "undefined" && GLImmediate.matrix) ? GLImmediate.matrix[0] : null;
        if (!m) return 1;
        var p = $0 >> 2;
        for (var i = 0; i < 16; i++) if (m[i] !== HEAPF32[p + i]) return 0;
        return 1;
    }, g_matrix);
}

static void mu_read_color(float* dst) {
    EM_ASM({
        var c = (typeof GLImmediate !== "undefined") ? GLImmediate.clientColor : null;
        var p = $0 >> 2;
        if (!c) { HEAPF32[p] = 1; HEAPF32[p+1] = 1; HEAPF32[p+2] = 1; HEAPF32[p+3] = 1; return; }
        HEAPF32[p] = c[0]; HEAPF32[p+1] = c[1]; HEAPF32[p+2] = c[2]; HEAPF32[p+3] = c[3];
    }, dst);
}

static void mu_capture_color() { mu_read_color(g_color); }

static int mu_color_same() {
    return EM_ASM_INT({
        var c = (typeof GLImmediate !== "undefined") ? GLImmediate.clientColor : null;
        if (!c) return 1;
        var p = $0 >> 2;
        for (var i = 0; i < 4; i++) if (c[i] !== HEAPF32[p + i]) return 0;
        return 1;
    }, g_color);
}

static void mu_apply_state(int blend, int depth, int cull, int alphaTest) {
    switch (blend) {
    case 1: GL_EnableLightMap(); break;
    case 2: GL_SetBlendSrcOver(1); break;
    case 3: GL_SetBlendAdditive(); break;
    case 4: GL_SetBlendSrcAlpha(); break;
    default: GL_ResetState(); break;
    }
    if (depth) GL_EnableDepthWrites();
    else GL_DisableDepthWrites();
    if (cull) GL_EnableCullFace();
    else GL_DisableCullFace();
    if ((int)DAT_083a411d != alphaTest) {
        DAT_083a411d = (char)alphaTest;
        if (alphaTest) glEnable(0xbc0);
        else glDisable(0xbc0);
    }
}

static int mu_state_same(int layout) {
    int useColor = (layout == MU_MESH_UV || layout == MU_MESH_POS);
    if (layout != g_layout) return 0;
    if ((int)DAT_00561574 != g_tex) return 0;
    if (DAT_083a412c != g_blend) return 0;
    if ((DAT_083a42e8 != 0) != g_depth) return 0;
    if ((DAT_083a411c != 0) != g_cull) return 0;
    if ((int)DAT_083a411d != g_alphaTest) return 0;
    if (useColor != g_useColor) return 0;
    if (useColor && !mu_color_same()) return 0;
    if ((g_muMesh.end - g_muMesh.p) < 256) return 0;
    return 1;
}

static void mu_remember(int layout) {
    g_layout = layout;
    g_tex = (int)DAT_00561574;
    g_blend = DAT_083a412c;
    g_depth = DAT_083a42e8 != 0;
    g_cull = DAT_083a411c != 0;
    g_alphaTest = (int)DAT_083a411d;
    g_useColor = (layout == MU_MESH_UV || layout == MU_MESH_POS);
    if (g_useColor) mu_capture_color();
    mu_capture_matrix();
}

static void mu_reset(int layout) {
    if (!g_muMeshBytes)
        g_muMeshBytes = (unsigned char*)malloc(MU_MESH_BYTES);
    g_muMesh.layout = layout;
    g_muMesh.n = 0;
    g_muMesh.phase = 0;
    g_muMesh.bad = g_muMeshBytes ? 0 : 1;
    g_muMesh.hasColor = 0;
    g_muMesh.base = g_muMeshBytes;
    g_muMesh.p = g_muMeshBytes;
    g_muMesh.end = g_muMeshBytes ? g_muMeshBytes + MU_MESH_BYTES : g_muMeshBytes;
    g_markP = g_muMesh.p;
    g_markN = 0;
}

extern "C" void mu_mesh_flush(void);

static void mu_open(int layout, int stable) {
    if (g_open && mu_state_same(layout) && (stable || mu_matrix_same())) {
        g_markP = g_muMesh.p;
        g_markN = g_muMesh.n;
        g_muMesh.layout = layout;
        g_muMesh.phase = 0;
        g_muMesh.bad = 0;
        return;
    }
    if (g_open) mu_mesh_flush();
    mu_reset(layout);
    mu_remember(layout);
    g_open = 1;
    mu_set_flag(1, 0);
    g_markP = g_muMesh.p;
    g_markN = 0;
}

void mu_mesh_start(int layout) { mu_open(layout, 0); }
void mu_mesh_start_stable(int layout) { mu_open(layout, 1); }

bool mu_mesh_finish() {
    if (!g_open) return false;
    if (g_muMesh.bad || g_muMesh.phase != 0) {
        g_muMesh.p = g_markP;
        g_muMesh.n = g_markN;
        g_muMesh.bad = 0;
        g_muMesh.phase = 0;
        return false;
    }
    return true;
}

struct TileRun {
    int tex, blend, depth, cull, alpha, layout, n, nbytes;
    unsigned char* data;
};

struct TileCell {
    int state;
    int hash;
    int nruns;
    TileRun* runs;
};

static TileCell* g_tileTab = nullptr;
static int g_tileMap = -1;
static TileCell** g_defer = nullptr;
static int g_deferN = 0;
static int g_deferCap = 0;
static int g_tileRec = 0;
static int g_tileAnim = 0;
static int g_tileHash = 0;
static int g_tileN = 0;
static TileRun g_tileRun[4];

static void mu_tile_free_cell(TileCell* cell) {
    if (!cell->runs) return;
    for (int i = 0; i < cell->nruns; i++) free(cell->runs[i].data);
    free(cell->runs);
    cell->runs = nullptr;
    cell->nruns = 0;
}

static void mu_tile_ensure(int map) {
    if (!g_tileTab) g_tileTab = (TileCell*)calloc(256 * 256, sizeof(TileCell));
    if (g_tileMap == map) return;
    g_deferN = 0;
    if (g_tileTab) {
        for (int i = 0; i < 256 * 256; i++) mu_tile_free_cell(&g_tileTab[i]);
        memset(g_tileTab, 0, 256 * 256 * sizeof(TileCell));
    }
    g_tileMap = map;
}

static int mu_tile_hash(int x, int y) {
    int i0 = y * 256 + x;
    int i2 = i0 + 256;
    int i3 = i2 + 1;
    if (i3 >= 256 * 256) return 0;
    unsigned h = 2166136261u;
    int idx[4] = { i0, i0 + 1, i2, i3 };
    for (int k = 0; k < 4; k++) {
        unsigned* p = (unsigned*)&DAT_081cb608[idx[k] * 3];
        h = (h ^ p[0]) * 16777619u;
        h = (h ^ p[1]) * 16777619u;
        h = (h ^ p[2]) * 16777619u;
    }
    return (int)h;
}

static void mu_tile_store(const unsigned char* bytes, int nbytes, int nverts) {
    if (!g_tileRec || g_tileAnim || nverts <= 0 || nbytes <= 0) return;
    if (g_tileN > 0) {
        TileRun* prev = &g_tileRun[g_tileN - 1];
        if (prev->tex == g_tex && prev->blend == g_blend && prev->depth == g_depth &&
            prev->cull == g_cull && prev->alpha == g_alphaTest && prev->layout == g_layout) {
            unsigned char* grown = (unsigned char*)realloc(prev->data, (size_t)prev->nbytes + (size_t)nbytes);
            if (!grown) { g_tileAnim = 1; return; }
            memcpy(grown + prev->nbytes, bytes, (size_t)nbytes);
            prev->data = grown;
            prev->nbytes += nbytes;
            prev->n += nverts;
            return;
        }
    }
    if (g_tileN >= 4) { g_tileAnim = 1; return; }
    unsigned char* copy = (unsigned char*)malloc((size_t)nbytes);
    if (!copy) { g_tileAnim = 1; return; }
    memcpy(copy, bytes, (size_t)nbytes);
    TileRun* run = &g_tileRun[g_tileN++];
    run->tex = g_tex;
    run->blend = g_blend;
    run->depth = g_depth;
    run->cull = g_cull;
    run->alpha = g_alphaTest;
    run->layout = g_layout;
    run->n = nverts;
    run->nbytes = nbytes;
    run->data = copy;
}

static void mu_tile_replay_run(const TileRun* run) {
    if (DAT_00561574 != (DWORD)run->tex)
        GL_BindTextureSlot(run->tex);
    DAT_083a412c = run->blend;
    DAT_083a42e8 = run->depth ? 1u : 0u;
    DAT_083a411c = (char)(run->cull ? 1 : 0);
    DAT_083a411d = (char)run->alpha;
    mu_open(run->layout, 1);
    if (!g_open || g_muMesh.p + run->nbytes > g_muMesh.end) {
        if (g_open) mu_mesh_flush();
        mu_open(run->layout, 1);
    }
    if (!g_open || g_muMesh.bad || g_muMesh.p + run->nbytes > g_muMesh.end) return;
    memcpy(g_muMesh.p, run->data, (size_t)run->nbytes);
    g_muMesh.p += run->nbytes;
    g_muMesh.n += run->n;
    if (run->layout == MU_MESH_UV_COLOR || run->layout == MU_MESH_COLOR_UV)
        g_muMesh.hasColor = 1;
}

int mu_tile_begin(int x, int y) {
    if ((unsigned)x > 255u || (unsigned)y > 255u) return 0;
    if ((y + 1) * 256 + x + 1 >= 256 * 256) return 0;
    int map = (int)gMapManager.GetCurrentMap();
    mu_tile_ensure(map);
    if (!g_tileTab) return 0;
    TileCell* cell = &g_tileTab[y * 256 + x];
    if (cell->state == 2) return 0;
    int hash = mu_tile_hash(x, y);
    if (cell->state == 1 && cell->hash == hash) {
        if (cell->nruns > 0) {
            if (g_deferN >= g_deferCap) {
                int cap = g_deferCap ? g_deferCap * 2 : 1024;
                TileCell** grown = (TileCell**)realloc(g_defer, sizeof(TileCell*) * (size_t)cap);
                if (!grown) {
                    for (int i = 0; i < cell->nruns; i++) mu_tile_replay_run(&cell->runs[i]);
                    return 1;
                }
                g_defer = grown;
                g_deferCap = cap;
            }
            g_defer[g_deferN++] = cell;
        }
        return 1;
    }
    g_tileRec = 1;
    g_tileAnim = 0;
    g_tileN = 0;
    g_tileHash = hash;
    return 2;
}

void mu_tile_anim(void) {
    if (g_tileRec) g_tileAnim = 1;
}

struct TileRef {
    int tex, blend, depth, cull, alpha, layout;
    const TileRun* run;
};

static int mu_tile_ref_less(const void* a, const void* b) {
    const TileRef* pa = (const TileRef*)a;
    const TileRef* pb = (const TileRef*)b;
    if (pa->blend != pb->blend) return pa->blend - pb->blend;
    if (pa->tex != pb->tex) return pa->tex - pb->tex;
    if (pa->layout != pb->layout) return pa->layout - pb->layout;
    if (pa->depth != pb->depth) return pa->depth - pb->depth;
    if (pa->cull != pb->cull) return pa->cull - pb->cull;
    return pa->alpha - pb->alpha;
}

void mu_tile_commit(void) {
    if (g_deferN <= 0) return;
    int nref = 0;
    for (int i = 0; i < g_deferN; i++) nref += g_defer[i]->nruns;
    TileRef* refs = (TileRef*)malloc(sizeof(TileRef) * (size_t)nref);
    if (!refs) {
        for (int i = 0; i < g_deferN; i++) {
            TileCell* cell = g_defer[i];
            for (int r = 0; r < cell->nruns; r++) mu_tile_replay_run(&cell->runs[r]);
        }
        g_deferN = 0;
        return;
    }
    int w = 0;
    for (int i = 0; i < g_deferN; i++) {
        TileCell* cell = g_defer[i];
        for (int r = 0; r < cell->nruns; r++) {
            TileRun* run = &cell->runs[r];
            refs[w].tex = run->tex;
            refs[w].blend = run->blend;
            refs[w].depth = run->depth;
            refs[w].cull = run->cull;
            refs[w].alpha = run->alpha;
            refs[w].layout = run->layout;
            refs[w].run = run;
            w++;
        }
    }
    qsort(refs, (size_t)w, sizeof(TileRef), mu_tile_ref_less);
    for (int i = 0; i < w; i++) mu_tile_replay_run(refs[i].run);
    free(refs);
    g_deferN = 0;
}

void mu_tile_end(int x, int y) {
    if (!g_tileRec) return;
    g_tileRec = 0;
    if ((unsigned)x > 255u || (unsigned)y > 255u || !g_tileTab) {
        for (int i = 0; i < g_tileN; i++) free(g_tileRun[i].data);
        g_tileN = 0;
        return;
    }
    TileCell* cell = &g_tileTab[y * 256 + x];
    mu_tile_free_cell(cell);
    if (g_tileAnim) {
        for (int i = 0; i < g_tileN; i++) free(g_tileRun[i].data);
        g_tileN = 0;
        cell->state = 2;
        cell->hash = 0;
        return;
    }
    cell->state = 1;
    cell->hash = g_tileHash;
    cell->nruns = g_tileN;
    cell->runs = nullptr;
    if (g_tileN > 0) {
        cell->runs = (TileRun*)malloc(sizeof(TileRun) * (size_t)g_tileN);
        if (!cell->runs) {
            for (int i = 0; i < g_tileN; i++) free(g_tileRun[i].data);
            cell->state = 2;
            cell->nruns = 0;
        } else {
            memcpy(cell->runs, g_tileRun, sizeof(TileRun) * (size_t)g_tileN);
        }
    }
    g_tileN = 0;
}

bool mu_mesh_emit4(int layout, int stable, const float* uv, const float* rgba, const float* xyz) {
    float baked[4] = { 1.f, 1.f, 1.f, 1.f };
    if (!rgba && layout != MU_MESH_POS) mu_read_color(baked);
    mu_open(layout, stable);
    static const int fan[6] = { 0, 1, 2, 0, 2, 3 };
    for (int k = 0; k < 6; k++) {
        int i = fan[k];
        if (layout == MU_MESH_COLOR_UV) {
            const float* c = rgba ? rgba + i * 4 : baked;
            mu_mesh_rgba(c[0], c[1], c[2], c[3]);
        }
        if (layout != MU_MESH_POS && uv)
            mu_mesh_tex(uv[i * 2], uv[i * 2 + 1]);
        if (layout == MU_MESH_UV_COLOR) {
            const float* c = rgba ? rgba + i * 4 : baked;
            mu_mesh_rgba(c[0], c[1], c[2], c[3]);
        }
        mu_mesh_xyz(xyz[i * 3], xyz[i * 3 + 1], xyz[i * 3 + 2]);
    }
    bool ok = mu_mesh_finish();
    if (g_tileRec) {
        if (!ok) g_tileAnim = 1;
        else mu_tile_store(g_markP, g_muMesh.p - g_markP, g_muMesh.n - g_markN);
    }
    return ok;
}

extern "C" EMSCRIPTEN_KEEPALIVE void mu_mesh_flush(void) {
    if (!g_open) return;
    int n = g_muMesh.n;
    int layout = g_muMesh.layout;
    int hasColor = g_muMesh.hasColor;
    int tex = g_tex, blend = g_blend, depth = g_depth, cull = g_cull, alphaTest = g_alphaTest;
    int useColor = g_useColor;
    g_open = 0;
    mu_set_flag(0, 1);
    if (n <= 0 || g_muMesh.bad) {
        mu_set_flag(0, 0);
        return;
    }
    int curTex = (int)DAT_00561574;
    int curBlend = DAT_083a412c;
    int curDepth = DAT_083a42e8 != 0;
    int curCull = DAT_083a411c != 0;
    int curAlpha = (int)DAT_083a411d;
    mu_apply_state(blend, depth, cull, alphaTest);
    GL_BindTextureSlot(tex);

    EM_ASM({
        var m = (typeof GLImmediate !== "undefined" && GLImmediate.matrix) ? GLImmediate.matrix[0] : null;
        if (!m) return;
        if (!Module._muMatSaved) Module._muMatSaved = new Float32Array(16);
        for (var s = 0; s < 16; s++) Module._muMatSaved[s] = m[s];
        var p = $0 >> 2;
        for (var i = 0; i < 16; i++) m[i] = HEAPF32[p + i];
        GLImmediate.matrixVersion[0] = (GLImmediate.matrixVersion[0] | 0) + 1;
        if ($1) {
            var c = GLImmediate.clientColor;
            if (!Module._muColSaved) Module._muColSaved = new Float32Array(4);
            Module._muColSaved[0] = c[0]; Module._muColSaved[1] = c[1];
            Module._muColSaved[2] = c[2]; Module._muColSaved[3] = c[3];
            var q = $2 >> 2;
            c[0] = HEAPF32[q]; c[1] = HEAPF32[q + 1]; c[2] = HEAPF32[q + 2]; c[3] = HEAPF32[q + 3];
        }
    }, g_matrix, useColor, g_color);

    glBegin(GL_TRIANGLES);
    int ok = EM_ASM_INT({
        var n = $1 | 0;
        var layout = $2 | 0;
        var stride = layout === 0 ? 24 : (layout === 3 ? 16 : 28);
        var bytes = (n * stride) | 0;
        if (typeof GLImmediate === "undefined" || GLImmediate.mode < 0) return 0;
        if (bytes <= 0 || bytes >= GL.MAX_TEMP_BUFFER_SIZE) return 0;
        var dst = GLImmediate.vertexDataU8;
        if (!dst || dst.length < bytes) return 0;
        dst.set(HEAPU8.subarray($0, ($0 + bytes) | 0));
        var F = GLctx.FLOAT;
        var U = GLctx.UNSIGNED_BYTE;
        if (layout === 0) {
            GLImmediate.addRendererComponent(GLImmediate.TEXTURE0, 2, F);
            GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, F);
        } else if (layout === 1) {
            GLImmediate.addRendererComponent(GLImmediate.TEXTURE0, 2, F);
            GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, U);
            GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, F);
        } else if (layout === 2) {
            GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, U);
            GLImmediate.addRendererComponent(GLImmediate.TEXTURE0, 2, F);
            GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, F);
        } else if (layout === 3) {
            GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, F);
        } else {
            return 0;
        }
        GLImmediate.vertexCounter = bytes >> 2;
        return 1;
    }, g_muMesh.base, n, layout);
    if (!ok) glEnd();
    else glEnd();

    EM_ASM({
        var m = (typeof GLImmediate !== "undefined" && GLImmediate.matrix) ? GLImmediate.matrix[0] : null;
        if (m && Module._muMatSaved) {
            for (var s = 0; s < 16; s++) m[s] = Module._muMatSaved[s];
            GLImmediate.matrixVersion[0] = (GLImmediate.matrixVersion[0] | 0) + 1;
        }
        if ($0 && GLImmediate.clientColor && Module._muColSaved) {
            var c = GLImmediate.clientColor;
            c[0] = Module._muColSaved[0]; c[1] = Module._muColSaved[1];
            c[2] = Module._muColSaved[2]; c[3] = Module._muColSaved[3];
        }
        if ($1 && GLImmediate.clientColor) {
            GLctx.disableVertexAttribArray(GLImmediate.COLOR);
            GLctx.vertexAttrib4fv(GLImmediate.COLOR, GLImmediate.clientColor);
        }
    }, useColor, hasColor);

    mu_apply_state(curBlend, curDepth, curCull, curAlpha);
    GL_BindTextureSlot(curTex);
    mu_set_flag(0, 0);
    (void)hasColor;
}
