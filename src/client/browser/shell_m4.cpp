// M4: the client's socket speaks WebSocket. A captured login is replayed
// by the gateway. Recv_Account and Recv_Character decode it.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"
#include "Core/Font.h"
#include "Net/Network.h"
#include "Net/MuEmu.h"
#include "Net/HWID.h"
#include "Config/ServerConfig.h"

#include <stdio.h>
#include <string.h>

extern "C" short Mu_AsyncKeyState[256] = {};
int SceneFlag = 2;

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

extern "C" void DbgLogPublic(const char* msg) {
    printf("DBG %s\n", msg ? msg : "");
    fflush(stdout);
}

extern void __cdecl CreateSocket(const char* ip, unsigned int port);

static unsigned char g_pool[8 * 0x394];
static int g_frames = 0;
static int g_drew = 0;

static void frame(void) {
    emscripten_webgl_make_context_current(
        (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(intptr_t)gWindow.GetGLContext());
    EM_ASM({
        if (typeof GLImmediate !== 'undefined' && !GLImmediate.matrixVersion) {
            Browser.useWebGL = true;
            GLImmediate.init();
        }
    });

    glClearColor(0.05f, 0.07f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char* hero = (char*)(uintptr_t)CharactersClient;
    char name[11];
    name[0] = 0;
    unsigned level = 0;
    if (hero && hero[0x1C1]) {
        memcpy(name, hero + 0x1C1, 10);
        name[10] = 0;
        level = *(unsigned short*)(hero + 0x1BE);
    }

    GL_Begin2D();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.15f, 0.2f, 0.28f, 1.0f);
    GL_DrawRect(16.0f, 16.0f, 280.0f, 72.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xFFFFFFFFu;
    CUIRenderText_RenderText(gFont.GetTextDC(), 28, 24, DAT_07d52c34, 0);
    if (name[0]) {
        char line[32];
        snprintf(line, sizeof(line), "%s  lv %u", name, level);
        CUIRenderText_RenderText(gFont.GetTextDC(), 28, 48, line, 0);
        if (!g_drew) {
            printf("M4 list server=%s name=%s level=%u\n", DAT_07d52c34, name, level);
            fflush(stdout);
            g_drew = 1;
        }
    }
    GL_End2D();

    if (g_frames == 1 || g_frames == 30 || g_frames == 120) {
        printf("M4 frame=%d\n", g_frames);
        fflush(stdout);
    }
    g_frames++;
}

int main(void) {
    printf("M4 start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("M4 init_failed\n");
        return 1;
    }
    gFont.Create(640);
    gFont.CreateTextSurface(gWindow.GetHdc() ? gWindow.GetHdc() : (HDC)1, 256, 64);
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    DAT_00559c78 = 0xFFFFFFFFu;

    memset(g_pool, 0, sizeof(g_pool));
    CharactersClient = (DWORD)(uintptr_t)g_pool;
    extern BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD* self, const char* fn);
    extern BOOL __cdecl CSimpleModulus_LoadDecryptionKey(DWORD* self, const char* fn);
    extern DWORD g_SimpleModulusCS[17];
    extern DWORD g_SimpleModulusSC[17];
    int okEnc = CSimpleModulus_LoadEncryptionKey(g_SimpleModulusCS, "Data/Enc1.dat");
    int okDec = CSimpleModulus_LoadDecryptionKey(g_SimpleModulusSC, "Data/Dec2.dat");
    printf("M4 keys enc=%d dec=%d\n", okEnc, okDec);
    fflush(stdout);
    MuEmu::InitKeys(ServerConfig::CustomerName, ServerConfig::ServerSerial);
    HWID_Init();
    gNetwork.BeginConnectServerSession();
    MuEmu::SetActive(false);
    CreateSocket(ServerConfig::ConnectServerIP, ServerConfig::ConnectServerPort);
    printf("M4 loop\n");
    fflush(stdout);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
