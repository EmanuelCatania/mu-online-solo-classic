// Browser integration smoke test. This is not game code.
// It creates a WebGL 2 context on #canvas and pumps emscripten's main loop.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include <stdio.h>

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;
static int g_frames = 0;

static void frame(void) {
    emscripten_webgl_make_context_current(g_ctx);
    glClearColor(0.05f, 0.35f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (g_frames == 0) {
        const GLubyte* version = glGetString(GL_VERSION);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        printf("WEBGL2_SMOKE context=%d version=%s renderer=%s\n",
               (int)g_ctx,
               version ? (const char*)version : "(null)",
               renderer ? (const char*)renderer : "(null)");
    }

    ++g_frames;
    if (g_frames >= 3) {
        printf("WEBGL2_SMOKE frames=%d\n", g_frames);
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
        printf("WEBGL2_SMOKE context_failed=%d\n", (int)g_ctx);
        return 1;
    }

    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
