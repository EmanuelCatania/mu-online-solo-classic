# Emscripten compatibility audit — MU Online 0.97k client

Source audited: [EmanuelCatania/Mu-97k-Client-Source](https://github.com/EmanuelCatania/Mu-97k-Client-Source), cloned at `reference/Mu-97k-Client-Source`. The clone was not modified. The native CMake/MSVC project was not modified.

Toolchain used for the proof: **Emscripten 6.0.12** (`emcc` 6.0.12, clang wasm32-unknown-emscripten), installed at `C:\Users\andvolodko\emsdk`. SDK alias `latest` resolved to `sdk-releases-6e9f6885cb17a3d6c8fd488a5cff730f50aba61d-64bit`. `emsdk.bat` on this machine calls Python 3.9 from `PATH` and exits; the install that succeeded was:

```text
C:\Python312\python.exe C:\Users\andvolodko\emsdk\emsdk.py install latest
C:\Python312\python.exe C:\Users\andvolodko\emsdk\emsdk.py activate latest
```

Activation was not written into the user environment. A later shell needs `emsdk_env.bat`.

## 1. Build system and compiler assumptions

| Item | What the tree requires |
|---|---|
| Project | `mu97k.sln`, `mu97k.vcxproj`, `CMakeLists.txt` |
| Compiler | MSVC only. `CMakeLists.txt` line 24: `FATAL_ERROR` unless `MSVC` |
| Toolset | v143 (Visual Studio 2022) |
| Platform | Win32. `CMAKE_SIZEOF_VOID_P` must be 4 (`CMakeLists.txt` lines 26–28). README: the port assumes 32-bit pointers, struct layouts, and pools |
| Language | C++17, MultiByte (`_MBCS`), no `UNICODE` |
| Runtime | `/MD` Release, `/MDd` Debug |
| PCH | `src/stdafx.cpp` creates `stdafx.h`; every other `.cpp` includes it |
| Output | `bin/Client/main.exe`, subsystem Windows, working directory `bin/Client` |
| Debug link | `/DYNAMICBASE:NO /FIXED` so DAT_ addresses match the original binary. Release does not pin the base. The game does not need those virtual addresses to run |
| Sources | 249 `.cpp` files listed explicitly in `CMakeLists.txt` (`MU_SOURCES`). CI compares that list to the `.vcxproj` |

Measured size of `src/`: **371** `.cpp`/`.h`/`.c` files, **122,619** non-blank lines.

Linked libraries (`CMakeLists.txt` `target_link_libraries` and the `.vcxproj`):

`jpeg`, `opengl32`, `glu32`, `winmm`, `ws2_32`, `version`, `dxguid`, `dsound`, `imm32`.

Bundled third parties: IJG **libjpeg 6b** (`lib/libjpeg`) and **miniaudio 0.11.25** (`lib/miniaudio/miniaudio.h`, dated 2026-03-04).

`src/stdafx.h` is included by every game translation unit and pulls in, in order: `winsock2.h`, `ws2tcpip.h`, `windows.h`, `mmsystem.h`, `dsound.h`, `<gl/gl.h>`, `<gl/glu.h>`, `<crtdbg.h>`, and `jpeglib.h` inside `extern "C"` with `XMD_H` set. It also `#define`s `send(...)` to `MuEmu_send_hook`.

## 2. Win32 APIs

The process entry is `WinMain` in `src/WinMain.cpp`. The window is `CWindow` in `src/Core/Window.cpp`.

`CWindow::Create` registers class `"Dialog"` (`CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS`) and calls `CreateWindowExA`. Fullscreen uses `EnumDisplaySettingsA` / `ChangeDisplaySettingsA`. The OpenGL context is created in `CWindow::InitOpenGL`:

- `GetDC`
- `ChoosePixelFormat` / `SetPixelFormat` with a 16-bit color, 16-bit depth, double-buffered `PIXELFORMATDESCRIPTOR`
- `wglCreateContext` / `wglMakeCurrent`
- `SwapBuffers` from the frame loop (`Game_MainLoop` takes an `HDC`)

Input is a Win32 message procedure plus polling:

- `WndProc` in `src/Core/WndProc.cpp` handles `WM_DESTROY`, `WM_TIMER`, `WM_USER` (socket), mouse, and IME
- Movement keys use `GetAsyncKeyState` from `src/UI/Chat_InputTick.cpp` and the game tick, not `WM_KEYDOWN`
- IME: `ImmGetContext` and related calls (6 hits under `src/`)
- Text metrics and fonts: `CreateFontA`, `SelectObject` (159 hits), `TextOutA`, `GetTextExtentPoint32A`, `CreateDIBSection`, `CreateCompatibleDC`

Other Win32 used by live code:

- Registry `RegQueryValueExA` and `GetPrivateProfileStringA` / `WritePrivateProfileStringA` in `src/Config/Config_Load.cpp` (`HKEY_CURRENT_USER\SOFTWARE\Webzen\Mu\Config` and `config.ini`)
- `GetVolumeInformationA("C:\\")`, `UuidCreateSequential` (`rpcrt4`), `GetSystemInfo` in `src/Net/HWID.cpp`
- `CreateFileA` / `WriteFile` / `ReadFile` for logs, minidumps (`src/Debug/MiniDump.cpp`), and a few legacy paths
- `MessageBoxA` at error sites in connect, data load, and exit
- `LoadLibraryA` / `GetProcAddress` (9 `GetProcAddress` hits), mostly dbghelp / optional modules
- `IsDebuggerPresent`, `OutputDebugStringA`, `SetUnhandledExceptionFilter`
- `MiniDumpWriteDump` (7 hits) in `src/Debug/MiniDump.cpp`
- GameGuard leftovers: pipe handles in `globals.h` (`lpTargetHandle_00563b5c`), `GameGuard_TickCheck` from `WM_TIMER` in `WndProc`

No `CreateThread`, `_beginthread`, or `CRITICAL_SECTION` usage was found under `src/`. The client is single-threaded. That matches WebAssembly's default single thread.

## 3. OpenGL

This is desktop OpenGL 1.1 fixed function. There are no `glCreateShader`, `glUseProgram`, `#version`, or `glDrawArrays` / `glDrawElements` call sites.

Call-site counts under `src/` (identifier scan):

| Call | Count | Role |
|---|---|---|
| `glColor3f` / `glColor4f` / `glColor3fv` | 288 / 97 / 36 | per-vertex color |
| `glTexCoord2f` | 94 | UVs |
| `glVertex3fv` / `glVertex2f` / `glVertex3f` | 77 / 28 / 11 | immediate vertices |
| `glBegin` / `glEnd` | 55 / 43 | immediate mode |
| `glPushMatrix` / `glPopMatrix` / `glMatrixMode` | 24 / 46 / 25 | matrix stack |
| `glBindTexture` / `glTexImage2D` / `glTexParameteri` | 22 / 15 / 21 | textures |
| `gluPerspective` / `gluOrtho2D` / `gluProject` | 5 / 3 / 1 | projection |
| `wglCreateContext` | 1 | `CWindow::InitOpenGL` |
| `wglUseFontBitmapsA` | 5 | UI glyphs |
| `glGenLists` / `glCallLists` / `glDeleteLists` / `glListBase` | 1 / 2 / 2 / 1 | display lists |
| `glBitmap` / `glRasterPos2f` | 4 / 3 | bitmap text |
| `glPushAttrib` / `glPopAttrib` | 2 / 2 | attrib stack |
| `glFogf` / `glFogfv` / `glFogi` | 3 / 2 / 2 | fog |
| `glAlphaFunc` | 2 | alpha test |

`src/Render/BMD_DrawMesh.cpp` draws a mesh with one `glBegin(GL_TRIANGLES)` and `glVertex3fv`. `src/Render/GL_State.cpp` sets blend, alpha test (`glEnable(0xbc0)` = `GL_ALPHA_TEST`), fog (`0xb60`), and texture 2D with numeric enums. `GL_BindTextureSlot` calls `glBindTexture(0xde1, ...)`.

Emscripten 6.0.12's legacy emulation (`src/lib/libglemu.js` in the SDK) implements `glBegin`/`glEnd`, the matrix stack, `glAlphaFunc` / `GL_ALPHA_TEST`, and partial `glFogf`/`glFogfv`. It does **not** define `glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f`, `glPushAttrib`, `glPopAttrib`, or any `wgl*` symbol. `system/include/GL/gl.h` still declares the desktop entry points, so those calls compile and then fail at link. `gluPerspective` and `gluProject` are present in `libsigs.js`.

`glTexEnvf` is a warn-once TODO in `libglemu.js`. `glDrawBuffer` / `glReadBuffer` abort. This client does not call those two.

WebGL 2 is OpenGL ES 3.0. It has no fixed-function pipeline. A browser build has to sit on `-sLEGACY_GL_EMULATION` (and `-sGL_FFP_ONLY`, since this client has no shaders) plus `-sMAX_WEBGL_VERSION=2`. That layer is a compatibility library, not desktop OpenGL.

The UI text path is the hard gap. `src/UI/UI_LegacyExterns.cpp` builds 256 display lists with `wglUseFontBitmapsA` and draws with `glRasterPos2f` + `glCallLists`. That cannot be emulated by the current Emscripten GL layer. World meshes can stay on immediate mode while text is replaced.

## 4. Assembly, x86 layout, and ABI

There is no compiled inline assembly. The only `_asm` hit is a comment in `src/Render/RenderLinkObject.cpp`.

Calling-convention keywords under `src/`: `__cdecl` 1925, `__fastcall` 478, `__stdcall` 87, `__thiscall` 44, `__int64` 136. On wasm32, clang keeps one ABI for the whole program, so `__cdecl` / `__fastcall` / `__stdcall` do not have to match the x86 register convention as long as every caller is also compiled by this toolchain. `__thiscall` and `__int64` need `-fms-extensions` or a typedef. `__try` does not become available that way: with `-fms-extensions`, `seh_probe.cpp` still fails with `SEH '__try' is not supported on this target`.

SEH is in eight files: `WinMain.cpp`, `UI/UI_LegacyExterns.cpp`, `Render/Effect_Tick.cpp`, `Render/HUD_Pass2.cpp`, `UI/RenderItemInfo.cpp`, `Render/Terrain_Render.cpp`, `Scene/Scene_ObjectInteraction.cpp`, `Scene/Scene_ObjectUpdate.cpp`. The blocks are crash guards (`EXCEPTION_EXECUTE_HANDLER` around minidumps, tooltip formatting, and a few render paths). They are not the packet or combat state machine. They have to be deleted or rewritten before those files compile. An access violation on wasm traps the module; the browser build will not have the "swallow this fault" behavior unless those sites get explicit checks.

`#pragma pack` is not used to define the client's own structs. `src/Net/Protocol/ProtocolBase.h` and `GameServerProtocol.h` document natural x86 alignment and lock it with `static_assert`. Those headers compiled and the asserts passed on wasm32.

Measured wasm32 ABI (`emscripten_port/browser/abi_probe.cpp`, node):

| Type | wasm32 (this SDK) | MSVC Win32, as this port relies on it |
|---|---|---|
| `void*` | 4 | 4 |
| `int` | 4 | 4 |
| `long` | 4 | 4 |
| `long long` | 8 | 8 |
| `float` / `double` | 4 / 8 | 4 / 8 |
| `long double` | **16** | **8** (`float10` comment in `functions.h`) |

`float10` is `typedef long double` in `types.h` and `functions.h`. It is used as a local and a return value (terrain light, entity reset, math), not as a field in `structs.h`. A 16-byte `long double` still changes the return ABI and turns hot `fsin` paths into software float128. The browser build should force `float10` to `double`, which is what MSVC actually does.

`DWORD`-sized object pointers (`*(DWORD*)(This + 8)` in `CWsctlc_Connect`) are valid on wasm32 because pointers are 4 bytes. They would break on wasm64. Stay on wasm32.

## 5. File I/O and assets

Assets live in `bin/Client/Data/` (about 209 MB: `.bmd`, `.ozj`, `.ozt`, maps, sounds). The executable's working directory is that `bin/Client` folder. Loaders use `fopen` / `crt_fopen` with paths such as the ones in `Terrain.cpp`, `Item_Data.cpp`, `Monster_Data.cpp`, `Music.cpp`. `Config_Load` builds `config.ini` via `GetCurrentDirectoryA`.

`fopen` itself is fine on Emscripten's MEMFS. The assumption that fails is "the files are already in the process working directory next to `main.exe`". A browser build has to fetch `Data/` into MEMFS or an IDBFS/worker filesystem before the existing loaders run. Preloading 209 MB in one wasm download is possible and slow; a fetch-on-demand map loader is the practical follow-up. The parsers can stay.

`GetPrivateProfileStringA` and the registry fallback in `Config_Load` need a file-backed or JS-backed adapter. `Config.ini` keys are the ones the README already documents.

## 6. Threads, timers, async

No threads and no critical sections. Synchronization is the Win32 message queue.

Timing:

- `GetTickCount` — 66 hits. `stdafx.h` even implements Ghidra's `__ftol()` as `(long long)GetTickCount()`
- `timeGetTime` — 17 (`winmm`)
- `QueryPerformanceCounter` / `Frequency` — 8 / 3
- `Sleep` — 6
- `timeBeginPeriod` / `timeEndPeriod` — the multimedia timer used by `CFrameLimiter` (`src/Game/FrameLimiter.h`, `HANDLE m_Timer`)
- `SetTimer` — 2, including the 20 s `WM_TIMER` keepalive / GameGuard tick in `WndProc`

`Game_MainLoop` (`src/Game/Game_MainLoop.cpp`) is the frame: poll `Net_Recv`, accumulate 40 ms logic steps, GL clear and draw, `SwapBuffers`, then a 25 fps limiter. The browser replacement is `emscripten_set_main_loop`. `Sleep` inside the limiter blocks the browser tab unless Asyncify is enabled. Asyncify is the wrong tool here; the limiter should yield to the browser frame callback.

`WSAAsyncSelect` is the other async mechanism. It is not a thread. It posts `WM_USER` to the window. See networking.

## 7. Audio

Two stacks:

- **Music.** `src/Sound/SoundManager.h` / `Music.cpp`. The original binary launched `MuPlayer.exe`. This reconstruction plays mp3/wav/flac with miniaudio inside the process. miniaudio 0.11 has an Emscripten backend. This is the part to keep.
- **Effects.** `src/Sound/Sound.h`, `Sound.cpp`, `Sound_DS3D.cpp`. DirectSound: `DirectSoundCreate` (3), `IDirectSound`, `IDirectSoundBuffer`, `DSBUFFERDESC`. Up to 420 slots, 4 channels, optional 3D position (`PlayBuffer`, `Sound_UpdatePos`). EAX `SetHall` is already a stub that returns 1. `dsound.h` / `dxguid` have no Emscripten equivalent. The effect path needs an adapter onto the same miniaudio engine, or onto the Web Audio API. The slot table and the call sites (`Sound_LoadAndPlay`, `PlayBuffer`) can stay if the buffer type behind them changes.

## 8. Sockets and packet framing

Transport is TCP only. `socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)` in `src/Net/Net_Connect.cpp` and `src/Render/SMD_Parser.cpp`. No `SOCK_DGRAM` usage. `WSAStartup` is called from `WinMain.cpp` and again inside `SMD_Parser.cpp`.

`CWsctlc::Connect` (`src/Net/Net_Connect.cpp`, `CWsctlc_Connect`) resolves with `inet_addr` then `gethostbyname`, calls non-blocking `connect`, and arms `WSAAsyncSelect(sock, hwnd, msg, 0x23)` for `FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE`. `WndProc` case `WM_USER` (`src/Core/WndProc.cpp`) dispatches:

- `wParam == 1` → `Net_Recv`
- `wParam == 2` → `Net_Send`
- `wParam == 0x20` → connect established

`Game_MainLoop` also polls `Net_Recv` every frame. Bytes are framed as Mu Online C1/C2 (plain + chain XOR) and C3/C4 (serial + chain XOR + `CSimpleModulus`). `src/Net/PacketFrame.h` (`PacketFrame_WantsEncrypt`) chooses the frame from the server's `HackPacketCheck` table. `stdafx.h` redirects `send()` to `MuEmu_send_hook`. Crypto is `src/Net/Crypto.cpp` and `src/Net/MuEmu.cpp`. Opcode handlers are `src/Net/Recv/Recv_*.cpp`.

Browsers cannot open a TCP socket to `ConnectServerPort` / `GameServerPort`, and they cannot deliver `WSAAsyncSelect` window messages. The bytes on the wire can stay. The socket and the `WM_USER` pump cannot.

A server-side TCP-to-WebSocket gateway is the transport that preserves framing: one binary WebSocket carries the same C1/C2/C3/C4 stream the GameServer already speaks. The client adapter reads and writes that stream and then calls the existing `Net_Recv` / `Net_Send` paths. Do not re-encode opcodes in JavaScript.

`src/Net/HWID.cpp` still has to send `F1/05` (36-character id, C3). The inputs today are the `C:` volume serial and a sequential UUID. Those Win32 calls need substitutes that still produce a stable 36-byte id. Skipping the packet would change login.

`src/Config/ServerConfig.h` compiles the server host, ports, `CustomerName`, `ServerSerial`, and `ClientVersion` into the binary. Those values are the encryption key. They must keep matching the GameServer. A gateway hostname can replace the TCP host without changing `MuEmu::InitKeys`.

## 9. External libraries

| Library | Where | Emscripten result |
|---|---|---|
| IJG libjpeg 6b | `lib/libjpeg/jpeg-6b`, sources listed in `lib/libjpeg/CMakeLists.txt` | Compiled and linked. Round-trip of an 8×8 RGB image returned pixel `200,9,40` from input `200,10,40` |
| miniaudio 0.11.25 | `lib/miniaudio/miniaudio.h` | Not compiled in this proof. The header targets Emscripten/WebAudio upstream. Music path is the consumer |
| opengl32 / glu32 | Windows SDK | Replaced by Emscripten's GL + legacy emulation. See section 3 |
| ws2_32 | Windows SDK | No equivalent. Adapter + gateway |
| dsound / dxguid / winmm | Windows SDK | No DirectSound. `timeGetTime` / `timeBeginPeriod` need a timer adapter |
| imm32 | Windows SDK | IME. Browser text input replaces it |
| version | Windows SDK | `FileVersion_Get` in `Config_Load.cpp`. Drop or stub for the browser; it reads a PE version resource |
| rpcrt4 | `HWID.cpp` `UuidCreateSequential` | Not in the link line of the vcxproj as a searched lib beyond `#pragma comment(lib, "rpcrt4.lib")`. Needs a substitute |
| dbghelp | `MiniDumpWriteDump` | Windows only. Crash dumps are not a browser feature |

## 10. What can stay

Already compiled to wasm without editing the reference sources:

- Every libjpeg 6b `.c` file named in `lib/libjpeg/CMakeLists.txt` (`jcapimin.c` through `jmemansi.c`)
- `src/Net/Protocol/ProtocolBase.h`, `GameServerProtocol.h`, `ConnectServerProtocol.h` (static asserts passed). The translation unit `Protocol_Check.cpp` does not compile yet, because it includes `stdafx.h`

Game `.cpp` files that are ordinary C++ once `stdafx.h` and the SEH sites are dealt with, and that do not call Win32 themselves: combat, item rules, entity motion, math, model tables, monster tables, party, pathfinding, cloth, trade, the `Recv_*` handlers, `Crypto.cpp`, `MuEmu.cpp`, `PacketFrame.cpp`, BMD load/animation math, terrain height and light math. Rendering call sites can stay if legacy GL emulation is accepted for `glBegin`/`glEnd`. They cannot stay unchanged for display-list text.

No game `.cpp` compiled in a clean Emscripten invocation. Each one stopped at `stdafx.h:4`.

## Reuse estimate

**75–85% of the 122,619 non-blank `src/` lines can remain unmodified**, if legacy GL emulation is good enough for the world renderer and the platform shell is replaced around it.

That range is "lines we do not have to rewrite", not "lines that compile today".

| Bucket | Approx. non-blank lines | Share | Evidence |
|---|---|---|---|
| Must be replaced or wrapped (window, input, DirectSound, socket pump, minidump, registry, HWID inputs, display-list text) | ~12,000–18,000 | 10–15% | Sections 2, 3, 7, 8. `Core` 2,511, `Input` 1,449, `Sound` 943, `Debug` 120, `Config` 855, `GameGuard` 103, Net transport on the order of 3–4k, UI text/IME a slice of `UI` 14,992 |
| World renderer, reusable only through legacy GL | `Render` 39,134 + `Terrain` 2,349 | ~34% | `glBegin` path exists in `libglemu.js`. Display lists and `wgl*` do not. This is the uncertainty |
| Gameplay, protocol, assets parsers | the rest | ~50% | Protocol headers already passed `static_assert` on wasm32. libjpeg round-trip passed |

**Floor, if immediate mode cannot be made correct or fast enough and `Render/` has to be rewritten: about 55–65%.** `Render/` alone is 32% of `src/`.

**Ceiling, if legacy GL plus a text replacement holds: about 85%.** Nothing in this audit supports a figure above that. `stdafx.h`, SEH, `long double`, and the socket pump are mandatory edits even in the best case.

Uncertainty is concentrated in two places: whether `-sLEGACY_GL_EMULATION` reproduces alpha-tested, fogged, immediate-mode meshes at a playable frame rate, and how large the UI text replacement is (`UI_LegacyExterns.cpp` versus a new glyph path). Both are measurable without rewriting combat or protocol.

## First 10 blocking errors

Clean compile of existing translation units (`Math_3D.cpp`, `GL_State.cpp`, `BMD_DrawMesh.cpp`, `PacketFrame.cpp`, `Protocol_Check.cpp`, `Window.cpp`), include path = `src` + libjpeg + miniaudio, no probe headers:

1. `fatal error: 'winsock2.h' file not found` at `src/stdafx.h:4`. This stops every game `.cpp`. The compiler does not reach the function body.

A diagnostic include directory (`emscripten_port/proof/include-probe/`) was then placed ahead of the system headers so the next diagnostics could be recorded. Those headers are not an adapter and are not sufficient to link. With them, `GL_State.cpp` got through `stdafx.h`'s GL and libjpeg includes and failed in the game's own headers:

2. `fatal error: 'crtdbg.h' file not found` at `src/stdafx.h:31` (before `crtdbg.h` was added to the probe).
3. `typedef unsigned __int64 undefined8` — `src/types.h:12`, `__int64` is not a type without MSVC extensions.
4. `unknown type name 'byte'` — `src/globals.h:734` and many other `extern byte` declarations. MSVC's `windows.h` typedefs lowercase `byte`. The probe only had `BYTE`.
5. `unknown type name '__forceinline'` — `src/globals.h:1266`, `ItemAttribute_Base`.
6. `unknown type name 'SIZE'` / `LPSIZE` — `src/globals.h:1423`, `:1424`, `:2833`.
7. `unknown type name 'LPBYTE'` — `src/globals.h:2127`.
8. `fatal error: 'intrin.h' file not found` via `#include_next <intrin.h>` — `src/functions.h:6` includes `<intrin.h>` for `_ReturnAddress`. Clang's wasm `intrin.h` forwards to a host header that does not exist.
9. `error: SEH '__try' is not supported on this target` — `emscripten_port/browser/seh_probe.cpp`, reproduced with `-fms-extensions`. This will hit the eight files listed in section 4.
10. Missing GL link symbols, confirmed absent from `libglemu.js`: `wglCreateContext`, `wglMakeCurrent`, `wglUseFontBitmapsA`, `glGenLists`, `glCallLists`, `glDeleteLists`, `glListBase`, `glBitmap`, `glRasterPos2f`, `glPushAttrib`, `glPopAttrib`. Declared in Emscripten's `GL/gl.h`, so they fail at link, not at parse.

`long double` being 16 bytes is not a diagnostic. It is a silent ABI mismatch (section 4) and belongs in the same list of things that have to be fixed before gameplay code is trusted.

## What this proof actually built

| Target | Result |
|---|---|
| `browser/abi_probe.cpp` | Ran under node. Pointers are 4 bytes. `long double` is 16 |
| `browser/protocol_check.cpp` including the three protocol headers | Exit 0. `sizeof(PBMSG_HEAD) == 3` and the header `static_assert`s passed |
| libjpeg 6b + `browser/jpeg_roundtrip.cpp` | `JPEG_ROUNDTRIP pixel=200,9,40` |
| `browser/webgl2_smoke.cpp` | Browser page at `out/webgl2_smoke.html`. Log: `context=1 version=OpenGL ES 3.0 (WebGL 2.0 (OpenGL ES 3.0 Chromium))`, `frames=3`. Canvas cleared to the requested blue |
| `browser/seh_probe.cpp` | Failed, with and without `-fms-extensions` |
| Six existing game `.cpp` files | Failed at `winsock2.h` |

The WebGL 2 page was loaded in a browser against `http://127.0.0.1:8765/webgl2_smoke.html`. The canvas is 640×480, `getContext('webgl2')` is non-null, and `gl.getParameter(gl.VERSION)` is `WebGL 2.0 (OpenGL ES 3.0 Chromium)`. The clear color is visible. That confirms the browser event loop and WebGL 2. It does not confirm the game's fixed-function renderer.

## Go / no-go

**Conditional go.** Compile the existing client with Emscripten, keep the gameplay and protocol code, and replace the platform shell. Do not start by rewriting the renderer, and do not expect `emcc` on the current `CMakeLists.txt` to produce a client.

**No-go** for "point Emscripten at the Win32 tree and ship". Desktop OpenGL is not in the browser, TCP is not in the browser, SEH is rejected by the wasm target, and `stdafx.h` does not parse.

The first implementation milestone after this audit is a browser `stdafx` preamble plus `-sLEGACY_GL_EMULATION` on `GL_State.cpp` and `BMD_DrawMesh.cpp`, not a new scene graph and not a Rust port.
