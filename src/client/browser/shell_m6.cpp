// M6: ArmorClass01 idle frame on World1 through the client's drawer.
// The shell loads the model, the skin, and the bone pose. It does not submit triangles.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" short Mu_AsyncKeyState[256] = {};
int SceneFlag = 5;

extern "C" {
void glPushAttrib(unsigned int) {}
void glPopAttrib(void) {}
void glDeleteLists(unsigned int, int) {}
unsigned int glGenLists(int) { return 0; }
void glListBase(unsigned int) {}
void glRasterPos2f(float, float) {}
void glCallLists(int, unsigned int, const void*) {}
void glBitmap(int, int, float, float, float, float, const unsigned char*) {}
}

void __cdecl Camera_SetupFrustum(float param_1, float* param_2);

extern "C" void DbgLogPublic(const char* msg) {
    printf("DBG %s\n", msg ? msg : "");
}

static char g_hero[512];
static int g_frames = 0;
static int g_ready = 0;
static int g_meshes = 0;
static int g_placed = 0;
static char* g_model = 0;
static float g_look[3];
static float g_shift[3];
static float g_cam[3];

static int file_opens(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void load_world(void) {
    gMapManager.SetCurrentMap(0);
    memset(g_hero, 0, sizeof(g_hero));
    Hero = g_hero;

    OpenTerrainMapping("Data/World1/EncTerrain1.map");
    int att = OpenTerrainAttribute("Data/World1/Terrain1.att");
    CreateTerrain("World1/TerrainHeight.OZB");
    OpenTerrainLight("World1/TerrainLight.OZJ");

    const char* tiles[] = {
        "World1/TileGrass01.OZJ", "World1/TileGrass02.OZJ",
        "World1/TileGround01.OZJ", "World1/TileGround02.OZJ", "World1/TileGround03.OZJ",
        "World1/TileWater01.OZJ", "World1/TileWood01.OZJ",
        "World1/TileRock01.OZJ", "World1/TileRock02.OZJ",
    };
    const int ids[] = { 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b };
    for (int i = 0; i < 9; i++)
        OpenJPG(tiles[i], ids[i], 0x2600, 0x2901, 0, 0);

    float lightMax = 0.0f;
    int lightTile = 32 * 256 + 32;
    for (int i = 0; i < 256 * 256; i++) {
        float s = DAT_07eeb238[i * 3] + DAT_07eeb238[i * 3 + 1] + DAT_07eeb238[i * 3 + 2];
        if (s > lightMax) { lightMax = s; lightTile = i; }
    }
    int tileX = lightTile & 255;
    int tileY = lightTile >> 8;
    float h = DAT_080cb2cc[lightTile];
    g_look[0] = (float)tileX * 100.0f;
    g_look[1] = (float)tileY * 100.0f;
    g_look[2] = h;
    *(float*)(g_hero + 16) = g_look[0];
    *(float*)(g_hero + 20) = g_look[1];
    *(float*)(g_hero + 24) = g_look[2];
    CameraPosition[0] = g_look[0];
    CameraPosition[1] = g_look[1] - 500.0f;
    CameraPosition[2] = g_look[2] + 350.0f;
    CameraAngle[0] = -50.0f;
    CameraAngle[1] = 0.0f;
    CameraAngle[2] = 0.0f;

    if (DAT_05828d58 == 0) {
        size_t modelPoolSz = (size_t)(963 + 512 + 0x400) * 0xbc;
        void* pool = malloc(modelPoolSz);
        if (pool && (uintptr_t)pool < 0x100000u) {
            malloc(0x100000);
            pool = malloc(modelPoolSz);
        }
        memset(pool, 0, modelPoolSz);
        DAT_06f42a58 = pool;
        DAT_05828d58 = (DWORD)(uintptr_t)pool;
    }

    int skin = OpenJPG("Player/skin_barbarian_01.OZJ", 0x12d, 0x2600, 0x2900, 0, 1);
    printf("M6 files bmd=%d skin=%d openSkin=%d skinW=%.0f pool=%08x att=%d tile=%d,%d h=%.1f\n",
           file_opens("Data/Player/ArmorClass01.bmd"),
           file_opens("Data/Player/skin_barbarian_01.OZJ"),
           skin, Bitmaps[0x12d].Width, (unsigned)DAT_05828d58, att, tileX, tileY, h);

    const int slot = 1;
    AccessModel(slot, "Data/Player/", "ArmorClass01", -1);
    TextureCurrent = 0x140;
    OpenTexture(slot, "Data/Player/", 0x2600, 1);

    g_model = (char*)(uintptr_t)(DAT_05828d58 + slot * 0xbc);
    g_meshes = *(short*)(g_model + 0x24);
    short* indexTex = *(short**)(g_model + 0x38);
    char* texNames = *(char**)(g_model + 0x34);
    char name0[33];
    memset(name0, 0, sizeof(name0));
    if (texNames) memcpy(name0, texNames, 32);
    char* actions = *(char**)(g_model + 0x2c);
    int keys0 = actions ? (int)*(short*)(actions + 0x20) : -1;
    printf("M6 model=%p meshes=%d bones=%d actions=%d keys0=%d light=%d stream=%d skin=%d hide=%d tex0=%d name=%s\n",
           (void*)g_model, g_meshes,
           (int)*(short*)(g_model + 0x26),
           (int)*(short*)(g_model + 0x22),
           keys0,
           (int)*(unsigned char*)(g_model + 0x44),
           (int)*(signed char*)(g_model + 0x88),
           (int)*(signed char*)(g_model + 0x98),
           (int)*(signed char*)(g_model + 0x99),
           indexTex ? (int)indexTex[0] : -1,
           name0);

    *(float*)(g_model + 0x48) = 1.0f;
    *(float*)(g_model + 0x4c) = 1.0f;
    *(float*)(g_model + 0x50) = 1.0f;
    *(char*)(g_model + 0x98) = 0;
    *(char*)(g_model + 0x99) = 0;
    fflush(stdout);
}

static int measure_pose(float minv[3], float maxv[3]) {
    char* mesh = (char*)(*(int*)(g_model + 0x28));
    short nV = *(short*)(mesh + 4);
    minv[0] = minv[1] = minv[2] = 1e30f;
    maxv[0] = maxv[1] = maxv[2] = -1e30f;
    int finite = 0;
    for (int i = 0; i < nV; i++) {
        float* p = (float*)(g_BoneVertexBuf + i * 12);
        int ok = 1;
        for (int k = 0; k < 3; k++) {
            if (p[k] != p[k] || p[k] > 1e8f || p[k] < -1e8f) ok = 0;
        }
        if (!ok) continue;
        finite++;
        for (int k = 0; k < 3; k++) {
            if (p[k] < minv[k]) minv[k] = p[k];
            if (p[k] > maxv[k]) maxv[k] = p[k];
        }
    }
    return finite;
}

static void pose_body(void) {
    *(float*)(g_model + 0x68) = 0.9f;
    *(float*)(g_model + 0x6c) = g_look[0] + g_shift[0];
    *(float*)(g_model + 0x70) = g_look[1] + g_shift[1];
    *(float*)(g_model + 0x74) = g_look[2] + g_shift[2];
    *(float*)(g_model + 0x8c) = 0.0f;
    *(float*)(g_model + 0x90) = 0.0f;
    *(float*)(g_model + 0x94) = 0.0f;
    float angles[3] = { 0.0f, 0.0f, 0.0f };
    float head[3] = { 0.0f, 0.0f, 0.0f };
    _DAT_005597c8 = 1.0f;
    BMD_Animation(g_model, (int)(intptr_t)g_BoneScratch, 0.0f, 0, 0,
                  (unsigned int*)angles, head, 0, 1);
    float aabbMin[3] = {};
    float aabbMax[3] = {};
    float aabbOut[12] = {};
    Skeleton_Transform(g_model, (int)(intptr_t)g_BoneScratch, aabbMin, aabbMax, aabbOut, 0);
}

static void place_on_tile(void) {
    pose_body();
    float minv[3], maxv[3];
    int finite = measure_pose(minv, maxv);
    float cx = (minv[0] + maxv[0]) * 0.5f;
    float cy = (minv[1] + maxv[1]) * 0.5f;
    g_shift[0] = g_look[0] - cx;
    g_shift[1] = g_look[1] - cy;
    g_shift[2] = g_look[2] - minv[2];
    pose_body();
    finite = measure_pose(minv, maxv);
    g_cam[0] = (minv[0] + maxv[0]) * 0.5f;
    g_cam[1] = (minv[1] + maxv[1]) * 0.5f;
    g_cam[2] = (minv[2] + maxv[2]) * 0.5f;
    char* mesh = (char*)(*(int*)(g_model + 0x28));
    char* script = *(char**)(mesh + 0x24);
    printf("M6 placed verts=%d tris=%d script=%d,%d bbox=(%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f) cam=(%.1f,%.1f,%.1f)\n",
           finite, (int)*(short*)(mesh + 10),
           script ? (int)(unsigned char)script[0] : -1,
           script ? (int)(unsigned char)script[1] : -1,
           minv[0], minv[1], minv[2], maxv[0], maxv[1], maxv[2],
           g_cam[0], g_cam[1], g_cam[2]);
    g_placed = 1;
}

static int count_changed(const unsigned char* before, const unsigned char* after) {
    int changed = 0;
    const int n = 640 * 480;
    for (int i = 0; i < n; i++) {
        int d = abs((int)before[i * 4] - (int)after[i * 4])
              + abs((int)before[i * 4 + 1] - (int)after[i * 4 + 1])
              + abs((int)before[i * 4 + 2] - (int)after[i * 4 + 2]);
        if (d > 8) changed++;
    }
    return changed;
}

static void frame(void) {
    emscripten_webgl_make_context_current(
        (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(intptr_t)gWindow.GetGLContext());
    EM_ASM({
        if (typeof GLImmediate !== 'undefined' && !GLImmediate.matrixVersion) {
            Browser.useWebGL = true;
            GLImmediate.init();
        }
    });
    if (!g_ready) {
        load_world();
        g_ready = 1;
    }

    glClearColor(0.039f, 0.078f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_meshes > 0 && !g_placed) place_on_tile();
    float cam[3] = {
        g_placed ? g_cam[0] : g_look[0],
        g_placed ? g_cam[1] : g_look[1],
        g_placed ? g_cam[2] : g_look[2]
    };
    CameraPosition[0] = cam[0];
    CameraPosition[1] = cam[1] - 120.0f;
    CameraPosition[2] = cam[2] + 80.0f;
    GL_BeginViewport(0, 0, 640, 0x30);
    Camera_SetupFrustum(640.0f * _DAT_0055283c, cam);
    GL_EndOpenGL();
    Terrain_Water();
    memcpy(DAT_081cb608, DAT_0828b608, sizeof(DAT_0828b608));
    for (int i = 0; i < 256 * 256 * 3; i++) {
        if (DAT_081cb608[i] < 0.65f) DAT_081cb608[i] = 0.65f;
    }

    GL_BeginViewport(0, 0, 640, 480);
    RenderTerrain(0);

    const int pixels = 640 * 480 * 4;
    unsigned char* before = (unsigned char*)malloc((size_t)pixels);
    unsigned char* after = (unsigned char*)malloc((size_t)pixels);
    if (before) glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, before);

    int fragments = 0;
    if (g_meshes > 0) {
        pose_body();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        DAT_083a411c = 1;
        glDisable(GL_CULL_FACE);
        EM_ASM({
            if (!Module._wrapDraws) {
                Module._wrapDraws = 1;
                var a = GLctx.drawArrays;
                GLctx.drawArrays = function(mode, first, count) {
                    var maxA = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
                    var prev = GLctx.getParameter(GLctx.ARRAY_BUFFER_BINDING);
                    for (var i = 0; i < maxA; i++) {
                        if (!GLctx.getVertexAttrib(i, GLctx.VERTEX_ATTRIB_ARRAY_ENABLED)) continue;
                        var buf = GLctx.getVertexAttrib(i, GLctx.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);
                        var stride = GLctx.getVertexAttrib(i, GLctx.VERTEX_ATTRIB_ARRAY_STRIDE) | 0;
                        var off = GLctx.getVertexAttribOffset(i, GLctx.VERTEX_ATTRIB_ARRAY_POINTER) | 0;
                        var bsize = 0;
                        if (buf) {
                            GLctx.bindBuffer(GLctx.ARRAY_BUFFER, buf);
                            bsize = GLctx.getBufferParameter(GLctx.ARRAY_BUFFER, GLctx.BUFFER_SIZE) | 0;
                        }
                        var step = stride > 0 ? stride : 16;
                        var need = off + (count > 0 ? (count - 1) * step : 0) + 4;
                        if (!buf || need > bsize) GLctx.disableVertexAttribArray(i);
                    }
                    if (prev) GLctx.bindBuffer(GLctx.ARRAY_BUFFER, prev);
                    return a.apply(this, arguments);
                };
            }
        });
        *(float*)(g_model + 0x48) = 1.0f;
        *(float*)(g_model + 0x4c) = 0.75f;
        *(float*)(g_model + 0x50) = 0.55f;
        BMD__RenderMesh(g_model, 0.0f, 2, 1.0f, -1, 1.0f, 0.0f, 0.0f, 0xffffffffu);
    }
    if (after) glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, after);
    if (before && after) fragments = count_changed(before, after);
    free(before);
    free(after);

    if (g_frames < 3) {
        printf("M6 frame=%d fragments=%d\n", g_frames, fragments);
        fflush(stdout);
    }
    g_frames++;
}

int main(void) {
    printf("M6 start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("M6 init_failed\n");
        return 1;
    }
    printf("M6 loop\n");
    fflush(stdout);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
