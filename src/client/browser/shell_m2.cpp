// M2: browser shell. The frame callback is Game_MainLoop.
// Mouse events go through Input_OnWindowMessage. Keys fill GetAsyncKeyState.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "stdafx.h"
#include "Core/Window.h"
#include "Game/Game_MainLoop.h"

#include <stdio.h>

extern "C" short Mu_AsyncKeyState[256] = {};
int SceneFlag = 0;

// Legacy GL emulation does not implement display lists or the attribute stack.
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
}
void Input_OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

static int g_frames = 0;

static LPARAM pack_lparam(int x, int y) {
    return (LPARAM)(((unsigned)y & 0xffffu) << 16) | ((unsigned)x & 0xffffu);
}

static EM_BOOL on_mouse(int type, const EmscriptenMouseEvent* e, void*) {
    LPARAM lp = pack_lparam(e->targetX, e->targetY);
    UINT msg = WM_MOUSEMOVE;
    if (type == EMSCRIPTEN_EVENT_MOUSEDOWN && e->button == 0) msg = WM_LBUTTONDOWN;
    else if (type == EMSCRIPTEN_EVENT_MOUSEUP && e->button == 0) msg = WM_LBUTTONUP;
    else if (type == EMSCRIPTEN_EVENT_MOUSEMOVE) msg = WM_MOUSEMOVE;
    else return EM_FALSE;
    Input_OnWindowMessage(msg, 0, lp);
    return EM_TRUE;
}

static EM_BOOL on_key(int type, const EmscriptenKeyboardEvent* e, void*) {
    unsigned vk = (unsigned)e->keyCode;
    if (vk < 256)
        Mu_AsyncKeyState[vk] = (type == EMSCRIPTEN_EVENT_KEYDOWN) ? (short)0x8000 : 0;
    return EM_TRUE;
}

static void frame(void) {
    emscripten_webgl_make_context_current(
        (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(intptr_t)gWindow.GetGLContext());
    Game_MainLoop(gWindow.GetHdc());
    ++g_frames;
    printf("SHELL frame=%d mouse=%u,%u click=%u button=%u shift=%d\n",
           g_frames,
           (unsigned)DAT_083a427c,
           (unsigned)DAT_083a4278,
           (unsigned)DAT_083a4124,
           (unsigned)DAT_083a42c4,
           (int)GetAsyncKeyState(VK_SHIFT));
    fflush(stdout);
}

int main(void) {
    printf("SHELL start\n");
    fflush(stdout);
    if (!gWindow.InitOpenGL()) {
        printf("SHELL init_failed\n");
        return 1;
    }
    emscripten_set_mousemove_callback("#canvas", 0, EM_TRUE, on_mouse);
    emscripten_set_mousedown_callback("#canvas", 0, EM_TRUE, on_mouse);
    emscripten_set_mouseup_callback("#canvas", 0, EM_TRUE, on_mouse);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, EM_TRUE, on_key);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, EM_TRUE, on_key);
    printf("SHELL loop\n");
    fflush(stdout);
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
