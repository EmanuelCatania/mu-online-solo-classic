// M3: one map (World1), one character model, one readable text line.
// Terrain and the mesh drawer are the client's. The shell only places the camera.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"
#include "Core/Font.h"

#include <stdio.h>
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
static unsigned char g_model[0x200];
static int g_meshes = 0;
static float g_scale = 1.0f;
static float g_feet = 0.0f;
static int g_frames = 0;
static int g_ready = 0;

static void copy_bind_pose(void) {
    short numMeshes = *(short*)(g_model + 0x24);
    int meshes = *(int*)(g_model + 0x28);
    int texIdx = *(int*)(g_model + 0x38);
    if (texIdx && numMeshes > 0)
        memset((void*)texIdx, 0, (size_t)numMeshes * 2);
    float minv[3] = { 1e30f, 1e30f, 1e30f };
    float maxv[3] = { -1e30f, -1e30f, -1e30f };
    if (numMeshes > 32) numMeshes = 32;
    for (int m = 0; m < numMeshes; m++) {
        char* mesh = (char*)(meshes + m * 0x28);
        *(short*)(mesh + 2) = 0;
        short nV = *(short*)(mesh + 4);
        short nT = *(short*)(mesh + 10);
        char* faces = *(char**)(mesh + 0x1c);
        if (m == 0 && g_frames == 0) {
            printf("MAP mesh %d verts=%d tris=%d face0=%d idx=%d\n",
                   m, (int)nV, (int)nT,
                   faces ? (int)(unsigned char)faces[0] : -1,
                   faces ? (int)*(short*)(faces + 2) : -1);
        }
        int vertBuf = *(int*)(mesh + 0x10);
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
        }
    }
    float tall = maxv[2] - minv[2];
    if (tall < 1.0f) tall = 1.0f;
    g_scale = 140.0f / tall;
    g_feet = -minv[2] * g_scale;
    g_meshes = numMeshes;
    if (g_frames == 0) {
        printf("MAP model meshes=%d bbox=(%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f) scale=%.3f\n",
               g_meshes, minv[0], minv[1], minv[2], maxv[0], maxv[1], maxv[2], g_scale);
    }
}

static void load_world(void) {
    gMapManager.SetCurrentMap(0);
    memset(g_hero, 0, sizeof(g_hero));
    Hero = g_hero;
    const float world = 32.0f * 100.0f;
    *(float*)(g_hero + 16) = world;
    *(float*)(g_hero + 20) = world;

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
    float lookX = (float)tileX * 100.0f;
    float lookY = (float)tileY * 100.0f;
    *(float*)(g_hero + 16) = lookX;
    *(float*)(g_hero + 20) = lookY;
    *(float*)(g_hero + 24) = h;
    CameraPosition[0] = lookX;
    CameraPosition[1] = lookY - 500.0f;
    CameraPosition[2] = h + 350.0f;
    CameraAngle[0] = -50.0f;
    CameraAngle[1] = 0.0f;
    CameraAngle[2] = 0.0f;
    printf("MAP loaded att=%d height=%.1f lightMax=%.3f tile=%d,%d grass=%.0f back=%.2f,%.2f,%.2f\n",
           att, h, lightMax, tileX, tileY, Bitmaps[0x23].Width,
           DAT_0828b608[lightTile * 3],
           DAT_0828b608[lightTile * 3 + 1],
           DAT_0828b608[lightTile * 3 + 2]);

    memset(g_model, 0, sizeof(g_model));
    BMD__Open((int)(intptr_t)g_model, (int)(intptr_t)"Data/Player/", (int)(intptr_t)"ArmorClass01.bmd", 0);
    g_meshes = *(short*)(g_model + 0x24);
    printf("MAP open meshes=%d\n", g_meshes);
    if (g_meshes > 0) {
        *(char*)(g_model + 0x44) = 0;
        *(char*)(g_model + 0x88) = -1;
        *(float*)(g_model + 0x48) = 0.15f;
        *(float*)(g_model + 0x4c) = 0.95f;
        *(float*)(g_model + 0x50) = 0.2f;
        copy_bind_pose();
    }

    gFont.Create(640);
    gFont.CreateTextSurface(gWindow.GetHdc() ? gWindow.GetHdc() : (HDC)1, 256, 64);
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    DAT_00559c78 = 0xFFFFFFFFu;
}

static int count_pixels(int* textOut) {
    const int n = 640 * 480;
    unsigned char* px = (unsigned char*)malloc((size_t)n * 4);
    if (!px) return -1;
    glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, px);
    int terrain = 0;
    int text = 0;
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            unsigned char* p = px + ((y * 640 + x) * 4);
            int bright = (p[0] > 40 || p[1] > 40 || p[2] > 40);
            if (bright) terrain++;
            if (y >= 448 && p[0] > 180 && p[1] > 180 && p[2] > 180) text++;
        }
    }
    free(px);
    *textOut = text;
    return terrain;
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

    float cam[3] = {
        *(float*)(g_hero + 16),
        *(float*)(g_hero + 20),
        *(float*)(g_hero + 24)
    };
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

    if (g_meshes > 0) {
        copy_bind_pose();
        // ArmorClass01's long axis is model X. Stand that axis up in front of
        // the camera. BMD__RenderMesh reads a bone slot this bind-pose copy
        // does not fill, so the shell submits the same triangles.
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        glDisable(GL_FOG);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glColor3f(0.95f, 0.82f, 0.45f);
        char* mesh = (char*)(*(int*)(g_model + 0x28));
        char* faces = *(char**)(mesh + 0x1c);
        int nF = *(short*)(mesh + 10);
        for (int f = 0; f < nF; f++) {
            char* face = faces + f * 0x24;
            glBegin(GL_TRIANGLES);
            for (int k = 0; k < 3; k++) {
                int idx = (int)*(short*)(face + 2 + k * 2);
                if (idx < 0 || idx > 91) idx = 0;
                float* p = (float*)(g_BoneVertexBuf + idx * 12);
                glVertex3f(p[2] * 2.2f, (p[0] - 16.0f) * 2.2f, p[1] * 2.2f - 240.0f);
            }
            glEnd();
        }
        glPopMatrix();
    }
    GL_EndOpenGL();

    GL_Begin2D();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
    GL_DrawRect(8.0f, 8.0f, 160.0f, 28.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xFFFFFFFFu;
    CUIRenderText_RenderText(gFont.GetTextDC(), 16, 14, "Lorencia", 0);
    GL_End2D();

    int textPx = 0;
    int terrainPx = count_pixels(&textPx);
    printf("MAP frame=%d terrain=%d text=%d\n", g_frames, terrainPx, textPx);
    fflush(stdout);
    g_frames++;
}

int main(void) {
    printf("MAP start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("MAP init_failed\n");
        return 1;
    }
    printf("MAP loop\n");
    fflush(stdout);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
