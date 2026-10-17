// M7: a terrain click walks ArmorClass01 through the client's path.
// Path_FindRoute builds the tiles, the walker steps them, MoveCharacter
// advances the animation, and MoveMainCamera follows the body.

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

static unsigned char g_ent[0x394];
static int g_frames = 0;
static int g_ready = 0;
static int g_meshes = 0;
static int g_placed = 0;
static int g_click_x = -1;
static int g_click_y = -1;
static int g_click_pending = 0;
static int g_walks = 0;
static int g_moving = 0;
static int g_last_tx = -1;
static int g_last_ty = -1;
static char* g_model = 0;
static float g_shift[3];
static char g_packet[240];
static char g_crossed[240];

static EM_BOOL on_click(int, const EmscriptenMouseEvent* e, void*) {
    if (!e || e->button != 0) return EM_FALSE;
    g_click_x = (int)e->targetX;
    g_click_y = (int)e->targetY;
    g_click_pending = 1;
    return EM_TRUE;
}

static int file_opens(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void load_world(void) {
    gMapManager.SetCurrentMap(0);
    memset(g_ent, 0, sizeof(g_ent));
    Hero = (char*)g_ent;
    DAT_07abf5d8 = (char*)g_ent;

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
    g_ent[0] = 1;
    *(short*)(g_ent + 2) = 1;
    *(float*)(g_ent + 0x0c) = 0.9f;
    *(float*)(g_ent + 16) = ((float)tileX + 0.5f) * 100.0f;
    *(float*)(g_ent + 20) = ((float)tileY + 0.5f) * 100.0f;
    *(float*)(g_ent + 24) = h;
    *(float*)(g_ent + 0x168) = 1.0f;
    *(int*)(g_ent + 0x388) = tileX;
    *(int*)(g_ent + 0x38c) = tileY;
    *(unsigned short*)(g_ent + 762) = 12;
    *(short*)(g_ent + 624) = -1;
    *(short*)(g_ent + 648) = -1;
    *(short*)(g_ent + 672) = -1;
    *(short*)(g_ent + 696) = -1;
    g_last_tx = tileX;
    g_last_ty = tileY;

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
    printf("M7 files bmd=%d skin=%d openSkin=%d att=%d tile=%d,%d h=%.1f\n",
           file_opens("Data/Player/ArmorClass01.bmd"),
           file_opens("Data/Player/skin_barbarian_01.OZJ"),
           skin, att, tileX, tileY, h);

    const int slot = 1;
    AccessModel(slot, "Data/Player/", "ArmorClass01", -1);
    TextureCurrent = 0x140;
    OpenTexture(slot, "Data/Player/", 0x2600, 1);

    g_model = (char*)(uintptr_t)(DAT_05828d58 + slot * 0xbc);
    g_meshes = *(short*)(g_model + 0x24);
    *(float*)(g_model + 0x48) = 1.0f;
    *(float*)(g_model + 0x4c) = 0.75f;
    *(float*)(g_model + 0x50) = 0.55f;
    *(char*)(g_model + 0x98) = 0;
    *(char*)(g_model + 0x99) = 0;
    printf("M7 model meshes=%d bones=%d actions=%d\n",
           g_meshes, (int)*(short*)(g_model + 0x26), (int)*(short*)(g_model + 0x22));
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
    *(float*)(g_model + 0x6c) = *(float*)(g_ent + 16) + g_shift[0];
    *(float*)(g_model + 0x70) = *(float*)(g_ent + 20) + g_shift[1];
    *(float*)(g_model + 0x74) = *(float*)(g_ent + 24) + g_shift[2];
    *(float*)(g_model + 0x8c) = 0.0f;
    *(float*)(g_model + 0x90) = 0.0f;
    *(float*)(g_model + 0x94) = *(float*)(g_ent + 36);
    *(unsigned char*)(g_model + 0xa0) = 0;
    float angles[3] = { 0.0f, 0.0f, *(float*)(g_ent + 36) };
    float head[3] = { 0.0f, 0.0f, 0.0f };
    float frame = *(float*)(g_ent + 0x108);
    if (frame != frame || frame < 0.0f || frame > 64.0f) frame = 0.0f;
    _DAT_005597c8 = 1.0f;
    BMD_Animation(g_model, (int)(intptr_t)g_BoneScratch, frame, 0, 0,
                  (unsigned int*)angles, head, 0, 1);
    float aabbMin[3] = {};
    float aabbMax[3] = {};
    float aabbOut[12] = {};
    Skeleton_Transform(g_model, (int)(intptr_t)g_BoneScratch, aabbMin, aabbMax, aabbOut, 0);
}

static void place_on_tile(void) {
    pose_body();
    float minv[3], maxv[3];
    measure_pose(minv, maxv);
    float cx = (minv[0] + maxv[0]) * 0.5f;
    float cy = (minv[1] + maxv[1]) * 0.5f;
    g_shift[0] = *(float*)(g_ent + 16) - cx;
    g_shift[1] = *(float*)(g_ent + 20) - cy;
    g_shift[2] = *(float*)(g_ent + 24) - minv[2];
    pose_body();
    g_placed = 1;
    printf("M7 placed at %.0f,%.0f\n", *(float*)(g_ent + 16), *(float*)(g_ent + 20));
    fflush(stdout);
}

static void note_tile(void) {
    int tx = (int)(*(float*)(g_ent + 16) / 100.0f);
    int ty = (int)(*(float*)(g_ent + 20) / 100.0f);
    if (tx == g_last_tx && ty == g_last_ty) return;
    g_last_tx = tx;
    g_last_ty = ty;
    char bit[16];
    snprintf(bit, sizeof(bit), "%d,%d ", tx, ty);
    if (strlen(g_crossed) + strlen(bit) < sizeof(g_crossed) - 1)
        strcat(g_crossed, bit);
    printf("M7 tile %d,%d\n", tx, ty);
    fflush(stdout);
}

static int unproject_tile(int mx, int my, int* tx, int* ty) {
    GLfloat model[16], proj[16];
    GLint vp[4];
    glGetFloatv(GL_MODELVIEW_MATRIX, model);
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, vp);
    if (vp[3] <= 0) return 0;
    GLdouble md[16], pr[16];
    for (int i = 0; i < 16; i++) { md[i] = model[i]; pr[i] = proj[i]; }
    double gy = (double)vp[3] - (double)my;
    double x0, y0, z0, x1, y1, z1;
    if (!gluUnProject((double)mx, gy, 0.0, md, pr, vp, &x0, &y0, &z0)) return 0;
    if (!gluUnProject((double)mx, gy, 1.0, md, pr, vp, &x1, &y1, &z1)) return 0;
    for (int i = 1; i <= 48; i++) {
        float t = (float)i / 48.0f;
        float x = (float)(x0 + (x1 - x0) * t);
        float y = (float)(y0 + (y1 - y0) * t);
        float z = (float)(z0 + (z1 - z0) * t);
        if (x < 0.0f || y < 0.0f || x >= 25600.0f || y >= 25600.0f) continue;
        if (z <= RequestTerrainHeight(x, y) + 30.0f) {
            *tx = (int)(x / 100.0f);
            *ty = (int)(y / 100.0f);
            return 1;
        }
    }
    return 0;
}

static void begin_walk(void) {
    int sx = (int)(*(float*)(g_ent + 16) / 100.0f);
    int sy = (int)(*(float*)(g_ent + 20) / 100.0f);
    *(int*)(g_ent + 0x388) = sx;
    *(int*)(g_ent + 0x38c) = sy;
    DAT_083a427c = g_click_x;
    DAT_083a4278 = g_click_y;
    int tx = 0, ty = 0, picked = 0;
    if (unproject_tile(g_click_x, g_click_y, &tx, &ty)) picked = 1;
    Camera_BuildMouseRay(g_click_x, g_click_y, (float*)&DAT_083a4110);
    Map_InitRayCast();
    DAT_07eab1fc = 0;
    RenderTerrain(1);
    if (DAT_07eab1fc) {
        int cx = (int)*(float*)&DAT_080ab288;
        int cy = (int)*(float*)&DAT_080ab28c;
        if (abs(cx - sx) < 40 && abs(cy - sy) < 40) {
            tx = cx;
            ty = cy;
            picked = 1;
        }
    }
    if (!picked) {
        printf("M7 click %d,%d missed the terrain\n", g_click_x, g_click_y);
        fflush(stdout);
        return;
    }
    unsigned int ok = Path_FindRoute(sx, sy, tx, ty, g_ent + 0x354, 0.0f);
    int count = g_ent[0x356];
    if (!ok || count < 2) {
        printf("M7 click %d,%d tile %d,%d route=0\n", g_click_x, g_click_y, tx, ty);
        fflush(stdout);
        return;
    }
    g_packet[0] = 0;
    for (int i = 0; i < count && i < 15; i++) {
        char bit[16];
        snprintf(bit, sizeof(bit), "%d,%d ", g_ent[0x357 + i], g_ent[0x366 + i]);
        if (strlen(g_packet) + strlen(bit) < sizeof(g_packet) - 1)
            strcat(g_packet, bit);
    }
    int bytes = 5 + ((count >> 1) + 1);
    Combat_SendMovePathPacket((int)(intptr_t)g_ent, (int)(intptr_t)g_ent);
    g_walks++;
    g_moving = g_ent[748] ? 1 : 0;
    g_crossed[0] = 0;
    snprintf(g_crossed, sizeof(g_crossed), "%d,%d ", sx, sy);
    printf("M7 path packet %d bytes walks=%d moving=%d tiles=%s\n",
           bytes, g_walks, g_moving, g_packet);
    fflush(stdout);
}

static void step_walk(void) {
    if (!g_ent[748]) {
        if (g_moving) {
            g_moving = 0;
            printf("M7 arrived crossed=%s\n", g_crossed);
            fflush(stdout);
        }
        return;
    }
    SetPlayerWalk((int)(intptr_t)g_ent);
    unsigned int moveOk = Entity_AdvancePath(g_ent, 1);
    if ((char)moveOk == 0)
        MoveCharacterPosition((int)(intptr_t)g_ent);
    else {
        g_ent[0x354] = 0;
        g_ent[0x355] = 0;
        g_ent[0x356] = 0;
        g_ent[748] = 0;
        SetPlayerStop((int)(intptr_t)g_ent);
        g_moving = 0;
        note_tile();
        printf("M7 arrived crossed=%s\n", g_crossed);
        fflush(stdout);
    }
}

static void install_draw_fix(void) {
    EM_ASM({
        if (Module._wrapDraws) return;
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
    });
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
    if (g_meshes > 0 && !g_placed) place_on_tile();

    if (g_placed) {
        step_walk();
        MoveCharacter((int)(intptr_t)g_ent);
        note_tile();
    }
    MoveMainCamera();

    float look[3] = {
        *(float*)(g_ent + 16),
        *(float*)(g_ent + 20),
        *(float*)(g_ent + 24)
    };
    GL_BeginViewport(0, 0, 640, 480);
    Camera_SetupFrustum(640.0f * _DAT_0055283c, look);
    if (g_click_pending && g_placed && !g_ent[748]) {
        g_click_pending = 0;
        begin_walk();
    }

    glClearColor(0.039f, 0.078f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Terrain_Water();
    memcpy(DAT_081cb608, DAT_0828b608, sizeof(DAT_0828b608));
    for (int i = 0; i < 256 * 256 * 3; i++) {
        if (DAT_081cb608[i] < 0.65f) DAT_081cb608[i] = 0.65f;
    }
    GL_BeginViewport(0, 0, 640, 480);
    glColor3f(1.0f, 1.0f, 1.0f);
    RenderTerrain(0);

    if (g_meshes > 0) {
        pose_body();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        DAT_083a411c = 1;
        glDisable(GL_CULL_FACE);
        install_draw_fix();
        glColor3f(1.0f, 0.75f, 0.55f);
        *(float*)(g_model + 0x48) = 1.0f;
        *(float*)(g_model + 0x4c) = 0.75f;
        *(float*)(g_model + 0x50) = 0.55f;
        BMD__RenderMesh(g_model, 0.0f, 2, 1.0f, -1, 1.0f, 0.0f, 0.0f, 0xffffffffu);
    }

    if (g_frames < 3 || (g_frames % 30) == 0) {
        printf("M7 frame=%d pos=%.0f,%.0f action=%d framef=%.2f\n",
               g_frames, *(float*)(g_ent + 16), *(float*)(g_ent + 20),
               (int)g_ent[0x105], *(float*)(g_ent + 0x108));
        fflush(stdout);
    }
    g_frames++;
}

int main(void) {
    printf("M7 start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("M7 init_failed\n");
        return 1;
    }
    emscripten_set_click_callback("#canvas", 0, 1, on_click);
    printf("M7 loop\n");
    fflush(stdout);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
