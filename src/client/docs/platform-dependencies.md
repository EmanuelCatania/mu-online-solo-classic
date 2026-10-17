# Platform dependencies — MU Online 0.97k client

Classification of the Win32 reconstruction at `reference/Mu-97k-Client-Source`. Line counts are non-blank lines under `src/`. "Reusable without changes" means the file compiled to wasm in this audit, or it is a header that did. Everything else that is ordinary C++ still includes `src/stdafx.h`, so it needs the browser preamble before it will compile even when its body can stay.

## Directory summary

| Directory | Files | Non-blank lines | Classification |
|---|---:|---:|---|
| `src/Combat` | 12 | 7,137 | Reusable with small changes |
| `src/Config` | 6 | 855 | Requires an adapter |
| `src/Core` | 12 | 2,511 | Requires an adapter |
| `src/Debug` | 2 | 120 | Requires substantial rewriting (or drop) |
| `src/Entity` | 13 | 1,355 | Reusable with small changes |
| `src/Game` | 22 | 5,638 | Reusable with small changes |
| `src/GameGuard` | 3 | 103 | Unknown; likely an adapter or a confirmed no-op |
| `src/Input` | 6 | 1,449 | Requires an adapter |
| `src/Item` | 35 | 7,501 | Reusable with small changes |
| `src/Local` | 3 | 352 | Reusable with small changes |
| `src/Math` | 3 | 314 | Reusable with small changes |
| `src/Model` | 7 | 1,171 | Reusable with small changes |
| `src/Monster` | 3 | 1,792 | Reusable with small changes |
| `src/Net` | 45 | 16,444 | Split: protocol reusable, transport adapter |
| `src/Party` | 2 | 442 | Reusable with small changes |
| `src/Path` | 2 | 657 | Reusable with small changes |
| `src/Physics` | 1 | 940 | Reusable with small changes |
| `src/Render` | 95 | 39,134 | Reusable with small changes if legacy GL holds; text path does not |
| `src/Scene` | 27 | 7,639 | Reusable with small changes |
| `src/Sound` | 7 | 943 | Requires an adapter |
| `src/Terrain` | 10 | 2,349 | Reusable with small changes |
| `src/Trade` | 2 | 514 | Reusable with small changes |
| `src/UI` | 41 | 14,992 | Mixed: widgets reusable, GDI/IME/font lists need an adapter |
| `src/Util` | 2 | 210 | Requires an adapter (process exit, a few Win32 helpers) |
| `src/*.cpp`, `src/*.h` at the root | — | ~8,057 | Mixed. `stdafx.h` is the preamble. `WinMain.cpp` is the shell |

Root files were included in the 122,619 total and are the difference between that total and the directory sum (114,562).

## Libraries and system APIs

| Dependency | Symbols / files | Class |
|---|---|---|
| IJG libjpeg 6b | `lib/libjpeg/CMakeLists.txt` `JPEG_SOURCES`; included from `stdafx.h` as `jpeglib.h` | **Reusable without changes.** Compiled and round-tripped in this audit |
| Protocol structs | `src/Net/Protocol/ProtocolBase.h`, `GameServerProtocol.h`, `ConnectServerProtocol.h` | **Reusable without changes.** `static_assert`s passed on wasm32. `Protocol_Check.cpp` is not in this class because it includes `stdafx.h` |
| miniaudio 0.11.25 | `lib/miniaudio/miniaudio.h`; `CSound::PlayMusic` in `src/Sound/SoundManager.h` | **Reusable with small changes.** Music only. Needs the Emscripten/WebAudio backend switch inside miniaudio, which the header already contains |
| MSVC C++17 language | `__cdecl`, `__fastcall`, `__stdcall`, `__thiscall`, `__int64` across `src/` | **Reusable with small changes.** One ABI on wasm32. `__int64` needs `-fms-extensions` or a typedef in the browser preamble |
| `float10` / `long double` | `src/types.h:14`, `src/functions.h:9`, locals in `Terrain_Light.cpp`, `Entity_Reset.cpp`, `Math_3D.cpp` | **Reusable with small changes.** Force `float10` to `double`. wasm `long double` is 16 bytes; MSVC's is 8 |
| Fixed-function GL used by meshes | `glBegin`, `glVertex3fv`, `glColor3f`, `glTexCoord2f`, `glBindTexture`, matrix stack, `glAlphaFunc`, `glFogf` in `src/Render/BMD_DrawMesh.cpp`, `GL_State.cpp`, `Terrain_*.cpp` | **Implemented.** Emulated via `-sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2` and post-link batching in `patch-login-page.ps1`. |
| GLU | `gluPerspective`, `gluOrtho2D`, `gluProject` | **Implemented.** Supported via Emscripten legacy GL layer (`libsigs.js`). |
| WGL context | `wglCreateContext`, `wglMakeCurrent`, `wglDeleteContext`, `ChoosePixelFormat`, `SetPixelFormat`, `SwapBuffers` in `src/Core/Window.cpp` `CWindow::InitOpenGL` | **Implemented.** Handled via Emscripten WebGL 2 context creation on `#canvas` in `browser/shell_login.cpp`. |
| Display-list text | `wglUseFontBitmapsA`, `glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f` in `src/UI/UI_LegacyExterns.cpp` | **Adapted.** Modern font and UI rendering path in WebGL canvas. |
| `glPushAttrib` / `glPopAttrib` | 2 call sites | **Adapted.** State tracking adapter in browser shell. |
| `glTexEnvf` | 1 call site | **Resolved.** Texture environment handled by legacy GL pipeline. |
| Win32 window | `CreateWindowExA`, `RegisterClassA`, `WndProc`, `PeekMessage` / `GetMessage`, `ChangeDisplaySettingsA` — `src/Core/Window.cpp`, `src/Core/WndProc.cpp`, `src/WinMain.cpp` | **Implemented.** Replaced by HTML5 Canvas + `emscripten_set_main_loop` in `browser/shell_login.cpp`. |
| Keyboard / mouse | `GetAsyncKeyState` (`src/UI/Chat_InputTick.cpp`), mouse in `WndProc` / `src/Input/Input_WndProc.cpp` | **Implemented.** HTML5 keyboard, mouse, and touch events mapped to `g_MouseX`/`g_MouseY` (640×480 viewport). |
| IME | `ImmGetContext` and `WM_IME_COMPOSITION` in `WndProc` | **Adapted.** Input handled through DOM elements when needed. |
| GDI fonts | `CreateFontA`, `SelectObject`, `GetTextExtentPoint32A`, `CreateDIBSection` — `src/Core/Font.cpp`, UI text | **Adapted.** Bitmapped/canvas font metric integration. |
| Timers | `GetTickCount`, `timeGetTime`, `QueryPerformanceCounter`, `CFrameLimiter` (`src/Game/FrameLimiter.h`), `SetTimer` id 1000 in `WndProc` | **Implemented.** High-resolution timing backed by `emscripten_get_now()` / `performance.now()`. |
| `fopen` asset loaders | `Terrain.cpp`, `Item_Data.cpp`, `Monster_Data.cpp`, `Skill_Data.cpp`, `Music.cpp`, `BMD` loaders | **Implemented.** Assets preloaded into MEMFS virtual filesystem (`login-data@/Data`). |
| `GetPrivateProfile*` / registry | `src/Config/Config_Load.cpp`, `src/Config/UserSettings.cpp` | **Implemented.** In-memory configuration defaults with browser persistence. |
| Winsock TCP | `WSAStartup`, `socket`, `connect`, `gethostbyname`, `closesocket`, `recv` — `src/Net/Net_Connect.cpp`, `src/Net/Network.cpp`, `src/Render/SMD_Parser.cpp` | **Implemented.** Zero-network in-memory bridge (`browser/net_ws.cpp` + `window.soloBridge`), communicating directly with in-browser GameServer Web Worker. |
| `WSAAsyncSelect` + `WM_USER` | `CWsctlc_Connect`, `WndProc` case `WM_USER` | **Implemented.** Dispatches `FD_READ`, `FD_WRITE`, and `FD_CLOSE` via `post_event()` into `gNetwork.OnSocketEvent`. |
| Packet framing and crypto | `PacketFrame_WantsEncrypt` (`src/Net/PacketFrame.cpp`), `MuEmu_send_hook` (`src/Net/MuEmu.cpp`), `src/Net/Crypto.cpp`, `src/Net/Recv/Recv_*.cpp` (~6,074 lines of handlers), `SecondPassword.cpp` (4,505) | **Implemented & Reusable.** Core packet encoders/decoders fully operational on WebAssembly. |
| HWID | `HWID_Init`, `HWID_Send` in `src/Net/HWID.cpp`. `GetVolumeInformationA`, `UuidCreateSequential`, `GetSystemInfo`. Packet `F1/05` | **Adapted.** Virtual machine/browser identifier for packet `F1/05`. |
| DirectSound | `DirectSoundCreate`, `PlayBuffer`, `Sound_UpdatePos` in `src/Sound/Sound.cpp`, `src/Sound/Sound_DS3D.cpp`. `CWaveFile` in `src/Sound/Sound.h` | **Adapted.** `miniaudio` WebAudio backend for sound effects and Lorencia BGM. |
| SEH | `__try` / `__except` in the eight files listed in the audit | **Cleaned.** SEH blocks replaced with standard C++ exception / error handling. |
| `intrin.h` / `_ReturnAddress` | `src/functions.h:6` | **Cleaned.** Inline intrinsic shims provided in `mu_emscripten.h`. |
| MiniDump / dbghelp | `src/Debug/MiniDump.cpp`, `SetUnhandledExceptionFilter` in `WinMain.cpp` | **Cleaned.** Excluded from browser build target. |
| GameGuard | `GameGuard_Init`, `GameGuard_TickCheck`, `src/GameGuard/*`, pipe handles in `globals.h` | **Cleaned.** Stubbed out as no-ops in single-player standalone mode. |
| Named pipes | `src/Net/Pipe_Legacy.cpp`, `src/Net/Net_PipeQuery.cpp` | **Cleaned.** Unused in standalone browser mode. |
| `version.dll` `FileVersion_Get` | `src/Config/Config_Load.cpp` | **Cleaned.** Uses static version string `0.97.04`. |
| Threads | none found | **Single-threaded.** Client main loop runs on browser main thread; GameServer runs in Web Worker. |
| Inline assembly | none in compiled code | **Cleaned / Reusable.** Fully portable C++17. |

## Preamble symbols provided by `src/stdafx.h` & `mu_emscripten.h`

The browser target includes `<mu_emscripten.h>` within `#ifdef __EMSCRIPTEN__` in `src/client/src/stdafx.h`:

- `DWORD`, `WORD`, `BYTE`, `byte`, `LPBYTE`, `BOOL`, `CHAR`, `LPCSTR`, `LPSTR`
- `HANDLE`, `HWND`, `HDC`, `HGLRC`, `HINSTANCE`, `HFONT`, `HGDIOBJ`
- `UINT`, `WPARAM`, `LPARAM`, `LRESULT`, `WNDPROC`
- `RECT`, `POINT`, `SIZE`, `LPSIZE`
- `SOCKET`, `sockaddr_in`, `hostent`, `WSADATA`
- `HRESULT`, `S_OK`, `E_FAIL`, `FAILED`
- `__forceinline` → `inline`
- Calling conventions `__cdecl`, `__fastcall`, `__stdcall`, `__thiscall` mapped to default on wasm32.
- `GetTickCount()` mapped to `emscripten_get_now()`.
- `glTexImage2D` wrapper handling legacy unsized internal formats for WebGL 2.

## Build targets

1. **Browser Target (`wasm32`)**:
   - Built via `src/client/link-login.bat`.
   - Links Emscripten `libGL`, compiled IJG `libjpeg` objects, and `miniaudio`.
   - Produces `web/shell_login.wasm`, `web/shell_login.js`, and `web/index.html`.

2. **Native Target (`Win32 x86`)**:
   - Project file: `src/client/mu97k.vcxproj` / `src/client/CMakeLists.native.txt`.
   - Links native Windows libraries: `jpeg.lib opengl32.lib glu32.lib winmm.lib ws2_32.lib version.lib dxguid.lib dsound.lib imm32.lib`.

