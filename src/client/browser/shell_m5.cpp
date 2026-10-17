// M5: walk, attack, inventory, chat, and one music track on World1.
// The shell calls the client's own move, attack, inventory, and chat paths.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"
#include "Core/Font.h"
#include "globals.h"
#include "Net/MuEmu.h"
#include "Net/Network.h"
#include "Config/ServerConfig.h"
#include "Sound/SoundManager.h"

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
    fflush(stdout);
}

extern void __cdecl CreateSocket(const char* ip, unsigned int port);
extern void __cdecl SendChat(char* text);
extern void __cdecl Action(DWORD c, DWORD o);
extern void __cdecl Combat_SendMovePathPacket(int entity, int object);
extern "C" void __cdecl RenderInventoryWindow(void);
extern "C" int __cdecl GetScreenWidth(void);
extern HRESULT __cdecl InitDirectSound(HWND hwnd);
extern void __cdecl LoadWaveFile(int buffer, const char* file, int maxChannel, bool enable3d);
extern HRESULT __cdecl PlayBuffer(int buffer, DWORD object, BOOL looped);

static unsigned char g_pool[2 * 0x394];
static unsigned char g_model[0x200];
static int g_meshes = 0;
static int g_frames = 0;
static int g_ready = 0;
static int g_steps = 0;
static int g_attacks = 0;
static int g_attackDraw = 0;
static int g_chatSent = 0;
static int g_music = 0;
static int g_sfx = 0;
static int g_tileX = 32;
static int g_tileY = 32;
static double g_frameMs = 0.0;
static double g_nextMove = 0.0;
static int g_prevSpace = 0;
static int g_prevI = 0;
static int g_prevEnter = 0;

static void copy_bind_pose(void) {
    short numMeshes = *(short*)(g_model + 0x24);
    int meshes = *(int*)(g_model + 0x28);
    int texIdx = *(int*)(g_model + 0x38);
    if (texIdx && numMeshes > 0)
        memset((void*)texIdx, 0, (size_t)numMeshes * 2);
    if (numMeshes > 32) numMeshes = 32;
    for (int m = 0; m < numMeshes; m++) {
        char* mesh = (char*)(meshes + m * 0x28);
        *(short*)(mesh + 2) = 0;
        short nV = *(short*)(mesh + 4);
        int vertBuf = *(int*)(mesh + 0x10);
        for (int i = 0; i < nV; i++) {
            float* src = (float*)(vertBuf + i * 0x10 + 4);
            float* dst = (float*)(g_BoneVertexBuf + ((i + m * 15000) * 12));
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
    }
    g_meshes = numMeshes;
}

static void place_entity(char* entity, int gx, int gy) {
    int idx = (gy & 255) * 256 + (gx & 255);
    *(float*)(entity + 16) = (float)gx * 100.0f + 50.0f;
    *(float*)(entity + 20) = (float)gy * 100.0f + 50.0f;
    *(float*)(entity + 24) = DAT_080cb2cc[idx];
    *(int*)(entity + 904) = gx;
    *(int*)(entity + 908) = gy;
}

static void follow_camera(void) {
    float x = *(float*)(Hero + 16);
    float y = *(float*)(Hero + 20);
    float z = *(float*)(Hero + 24);
    CameraPosition[0] = x;
    CameraPosition[1] = y - 500.0f;
    CameraPosition[2] = z + 350.0f;
    CameraAngle[0] = -50.0f;
    CameraAngle[1] = 0.0f;
    CameraAngle[2] = 0.0f;
}

static void start_audio(void) {
    if (!g_music) {
        g_music = gSound.PlayMusic("Data/Music/Lorencia.mp3") ? 1 : 0;
        printf("M5 music=%d\n", g_music);
        fflush(stdout);
    }
    if (!g_sfx) {
        LoadWaveFile(1, "Data/Sound/aMouse.WAV", 1, false);
        HRESULT hr = PlayBuffer(1, 0, 0);
        g_sfx = SUCCEEDED(hr) ? 1 : 0;
        printf("M5 effect=%d\n", g_sfx);
        fflush(stdout);
    }
    EM_ASM({
        if (window.miniaudio && window.miniaudio.devices) {
            for (var i = 0; i < window.miniaudio.devices.length; i++) {
                var d = window.miniaudio.devices[i];
                if (d && d.webaudio && d.webaudio.state !== 'running') d.webaudio.resume();
            }
        }
    });
}

static void try_move(int dx, int dy) {
    double now = emscripten_get_now();
    if (now < g_nextMove) return;
    g_nextMove = now + 180.0;
    int gx = *(int*)(Hero + 904) + dx;
    int gy = *(int*)(Hero + 908) + dy;
    if (gx < 1 || gy < 2 || gx > 254 || gy > 254) return;
    unsigned char* e = (unsigned char*)Hero;
    e[0x356] = 1;
    e[0x306] = (unsigned char)(gx - dx);
    e[0x307] = (unsigned char)(gy - dy);
    e[0x357] = (unsigned char)gx;
    e[0x366] = (unsigned char)gy;
    Combat_SendMovePathPacket((int)(intptr_t)Hero, (int)(intptr_t)Hero);
    place_entity(Hero, gx, gy);
    place_entity(Hero + 0x394, gx, gy - 1);
    follow_camera();
    g_steps++;
    g_tileX = gx;
    g_tileY = gy;
    printf("M5 move %d,%d steps=%d\n", gx, gy, g_steps);
    fflush(stdout);
}

static void load_world(void) {
    gMapManager.SetCurrentMap(0);
    memset(g_pool, 0, sizeof(g_pool));
    CharactersClient = (DWORD)(uintptr_t)g_pool;
    Hero = (char*)g_pool;
    memset(OffsetInventoryItems, 0xFF, 0x1100);

    OpenTerrainMapping("Data/World1/EncTerrain1.map");
    OpenTerrainAttribute("Data/World1/Terrain1.att");
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
    g_tileX = lightTile & 255;
    g_tileY = lightTile >> 8;
    if (g_tileY < 2) g_tileY = 2;

    *(short*)(Hero + 2) = 390;
    *(short*)(Hero + 624) = -1;
    *(short*)(Hero + 648) = -1;
    *(short*)(Hero + 696) = -1;
    memcpy(Hero + 0x1C1, "DarkWizard", 10);
    *(unsigned char*)(Hero + 749) = 3;
    *(short*)(Hero + 0x394 + 2) = 390;
    *(short*)(Hero + 0x394 + 476) = 1;
    place_entity(Hero, g_tileX, g_tileY);
    place_entity(Hero + 0x394, g_tileX, g_tileY - 1);
    DAT_00559ce8 = 1;
    follow_camera();

    memset(g_model, 0, sizeof(g_model));
    BMD__Open((int)(intptr_t)g_model, (int)(intptr_t)"Data/Player/", (int)(intptr_t)"ArmorClass01.bmd", 0);
    g_meshes = *(short*)(g_model + 0x24);
    if (g_meshes > 0) {
        *(char*)(g_model + 0x44) = 0;
        *(char*)(g_model + 0x88) = -1;
        copy_bind_pose();
    }

    gFont.Create(640);
    gFont.CreateTextSurface(gWindow.GetHdc() ? gWindow.GetHdc() : (HDC)1, 256, 64);
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    DAT_00559c78 = 0xFFFFFFFFu;

    InitDirectSound((HWND)(uintptr_t)gWindow.GetHwnd());
    extern BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD* self, const char* fn);
    extern BOOL __cdecl CSimpleModulus_LoadDecryptionKey(DWORD* self, const char* fn);
    extern DWORD g_SimpleModulusCS[17];
    extern DWORD g_SimpleModulusSC[17];
    int okEnc = CSimpleModulus_LoadEncryptionKey(g_SimpleModulusCS, "Data/Enc1.dat");
    int okDec = CSimpleModulus_LoadDecryptionKey(g_SimpleModulusSC, "Data/Dec2.dat");
    printf("M5 keys enc=%d dec=%d\n", okEnc, okDec);
    fflush(stdout);
    MuEmu::InitKeys(ServerConfig::CustomerName, ServerConfig::ServerSerial);
    MuEmu::SetActive(true);
    CreateSocket("127.0.0.1", 55901);
    printf("M5 world tile=%d,%d sound=%d\n", g_tileX, g_tileY, (int)g_EnableSound);
    fflush(stdout);
}

static void poll_input(void) {
    int left = GetAsyncKeyState(37) & 0x8000;
    int up = GetAsyncKeyState(38) & 0x8000;
    int right = GetAsyncKeyState(39) & 0x8000;
    int down = GetAsyncKeyState(40) & 0x8000;
    if (left) try_move(-1, 0);
    else if (right) try_move(1, 0);
    else if (up) try_move(0, -1);
    else if (down) try_move(0, 1);

    int space = (GetAsyncKeyState(32) & 0x8000) ? 1 : 0;
    if (space && !g_prevSpace) {
        *(unsigned char*)(Hero + 749) = 3;
        DAT_00559ce8 = 1;
        Action((DWORD)(uintptr_t)Hero, (DWORD)(uintptr_t)Hero);
        g_attacks++;
        g_attackDraw = 45;
        printf("M5 attack n=%d\n", g_attacks);
        fflush(stdout);
    }
    g_prevSpace = space;

    int keyI = (GetAsyncKeyState(73) & 0x8000) ? 1 : 0;
    if (keyI && !g_prevI) {
        InventoryOpened = InventoryOpened ? 0 : 1;
        printf("M5 inventory open=%d screen=%d\n", (int)InventoryOpened, GetScreenWidth());
        fflush(stdout);
    }
    g_prevI = keyI;

    int enter = (GetAsyncKeyState(13) & 0x8000) ? 1 : 0;
    if (enter && !g_prevEnter && !g_chatSent) {
        SendChat((char*)"hello");
        g_chatSent = 1;
        printf("M5 chat hello\n");
        fflush(stdout);
    }
    g_prevEnter = enter;
}

static void frame(void) {
    double t0 = emscripten_get_now();
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
    if (g_frames == 5) {
        try_move(1, 0);
        *(unsigned char*)(Hero + 749) = 3;
        DAT_00559ce8 = 1;
        Action((DWORD)(uintptr_t)Hero, (DWORD)(uintptr_t)Hero);
        g_attacks++;
        g_attackDraw = 90;
        InventoryOpened = 1;
        SendChat((char*)"hello");
        g_chatSent = 1;
        printf("M5 burst move=%d attack=%d inventory=%d screen=%d\n",
               g_steps, g_attacks, (int)InventoryOpened, GetScreenWidth());
        fflush(stdout);
    }
    poll_input();

    glClearColor(0.039f, 0.078f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float cam[3] = { *(float*)(Hero + 16), *(float*)(Hero + 20), *(float*)(Hero + 24) };
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
        float bob = (g_steps & 1) ? 18.0f : 0.0f;
        for (int f = 0; f < nF; f++) {
            char* face = faces + f * 0x24;
            glBegin(GL_TRIANGLES);
            for (int k = 0; k < 3; k++) {
                int idx = (int)*(short*)(face + 2 + k * 2);
                if (idx < 0 || idx > 91) idx = 0;
                float* p = (float*)(g_BoneVertexBuf + idx * 12);
                glVertex3f(p[2] * 2.2f, (p[0] - 16.0f) * 2.2f + bob, p[1] * 2.2f - 240.0f);
            }
            glEnd();
        }
        glPopMatrix();
    }
    GL_EndOpenGL();

    GL_Begin2D();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xFFFFFFFFu;
    char line[160];
    sprintf(line, "Lorencia %d,%d", g_tileX, g_tileY);
    CUIRenderText_RenderText(gFont.GetTextDC(), 16, 16, line, 0);
    sprintf(line, "walk %d  attack %d  %.1f ms", g_steps, g_attacks, g_frameMs);
    CUIRenderText_RenderText(gFont.GetTextDC(), 16, 36, line, 0);
    if (g_attackDraw > 0) {
        g_attackDraw--;
        GL_DrawRect(16.0f, 58.0f, 120.0f, 22.0f);
        CUIRenderText_RenderText(gFont.GetTextDC(), 24, 62, "Attack", 0);
    }
    if (g_chatSent) {
        CUIRenderText_RenderText(gFont.GetTextDC(), 16, 430, "DarkWizard: hello", 0);
    }
    sprintf(line, "music %d  effect %d", g_music, g_sfx);
    CUIRenderText_RenderText(gFont.GetTextDC(), 16, 450, line, 0);
    if (InventoryOpened) {
        RenderInventoryWindow();
        GL_DrawRect(450.0f, 8.0f, 170.0f, 36.0f);
        CUIRenderText_RenderText(gFont.GetTextDC(), 460, 16, "Inventory", 0);
    }
    GL_End2D();

    g_frameMs = emscripten_get_now() - t0;
    g_frames++;
    if (g_frames == 1 || g_frames == 30 || g_frames == 120) {
        int audio = EM_ASM_INT({
            if (!window.miniaudio || !window.miniaudio.devices || !window.miniaudio.devices.length) return 0;
            var d = window.miniaudio.devices[0];
            if (!d || !d.webaudio) return 1;
            if (d.webaudio.state === 'running') return 2;
            return 3;
        });
        printf("M5 frame=%d ms=%.2f audio=%d tile=%d,%d\n",
               g_frames, g_frameMs, audio, g_tileX, g_tileY);
        fflush(stdout);
    }
}

static EM_BOOL on_mouse(int type, const EmscriptenMouseEvent* e, void*) {
    if (type == EMSCRIPTEN_EVENT_MOUSEDOWN && e->button == 0) start_audio();
    return EM_TRUE;
}

static EM_BOOL on_key(int type, const EmscriptenKeyboardEvent* e, void*) {
    unsigned vk = (unsigned)e->keyCode;
    if (vk < 256)
        Mu_AsyncKeyState[vk] = (type == EMSCRIPTEN_EVENT_KEYDOWN) ? (short)0x8000 : 0;
    if (type == EMSCRIPTEN_EVENT_KEYDOWN) start_audio();
    return EM_TRUE;
}

int main(void) {
    printf("M5 start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("M5 init_failed\n");
        return 1;
    }
    emscripten_set_mousedown_callback("#canvas", 0, EM_TRUE, on_mouse);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, EM_TRUE, on_key);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, EM_TRUE, on_key);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
