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
| Fixed-function GL used by meshes | `glBegin`, `glVertex3fv`, `glColor3f`, `glTexCoord2f`, `glBindTexture`, matrix stack, `glAlphaFunc`, `glFogf` in `src/Render/BMD_DrawMesh.cpp`, `GL_State.cpp`, `Terrain_*.cpp` | **Requires an adapter:** `-sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMAX_WEBGL_VERSION=2`. The call sites can stay. Correctness and speed are unproven |
| GLU | `gluPerspective`, `gluOrtho2D`, `gluProject` | **Reusable with small changes.** Signatures exist in Emscripten (`libsigs.js`). Still needs the legacy GL link flags |
| WGL context | `wglCreateContext`, `wglMakeCurrent`, `wglDeleteContext`, `ChoosePixelFormat`, `SetPixelFormat`, `SwapBuffers` in `src/Core/Window.cpp` `CWindow::InitOpenGL` | **Requires an adapter.** Replace with `emscripten_webgl_create_context` on `#canvas` |
| Display-list text | `wglUseFontBitmapsA`, `glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f` in `src/UI/UI_LegacyExterns.cpp` | **Requires substantial rewriting.** None of these are implemented in `libglemu.js` |
| `glPushAttrib` / `glPopAttrib` | 2 call sites | **Requires an adapter.** Not implemented in legacy GL. Small, localized |
| `glTexEnvf` | 1 call site | **Unknown.** `libglemu.js` warns and does not implement it. Find the call site before relying on texture combine |
| Win32 window | `CreateWindowExA`, `RegisterClassA`, `WndProc`, `PeekMessage` / `GetMessage`, `ChangeDisplaySettingsA` — `src/Core/Window.cpp`, `src/Core/WndProc.cpp`, `src/WinMain.cpp` | **Requires an adapter.** Canvas + `emscripten_set_main_loop`. Fullscreen is the browser Fullscreen API, not `ChangeDisplaySettings` |
| Keyboard / mouse | `GetAsyncKeyState` (`src/UI/Chat_InputTick.cpp`), mouse in `WndProc` / `src/Input/Input_WndProc.cpp` | **Requires an adapter.** HTML5 keyboard and mouse state feeding the same globals (`g_MouseX` / `g_MouseY`, normalized to 640×480) |
| IME | `ImmGetContext` and `WM_IME_COMPOSITION` in `WndProc` | **Requires an adapter.** A DOM text field for chat. Korean DBCS composition is the behavior to preserve, not `imm32` |
| GDI fonts | `CreateFontA`, `SelectObject`, `GetTextExtentPoint32A`, `CreateDIBSection` — `src/Core/Font.cpp`, UI text | **Requires an adapter.** Measure with canvas `measureText` or a bitmap font. Do not pretend `wglUseFontBitmaps` exists |
| Timers | `GetTickCount`, `timeGetTime`, `QueryPerformanceCounter`, `CFrameLimiter` (`src/Game/FrameLimiter.h`), `SetTimer` id 1000 in `WndProc` | **Requires an adapter.** `emscripten_get_now` / `performance.now`. Do not `Sleep` on the main thread |
| `fopen` asset loaders | `Terrain.cpp`, `Item_Data.cpp`, `Monster_Data.cpp`, `Skill_Data.cpp`, `Music.cpp`, `BMD` loaders | **Reusable with small changes.** Same `fopen` calls once `Data/` is in MEMFS. The fetch/preload step is new and sits outside these files |
| `GetPrivateProfile*` / registry | `src/Config/Config_Load.cpp`, `src/Config/UserSettings.cpp` | **Requires an adapter.** Read `Config.ini` from MEMFS. Drop the registry fallback or back it with `localStorage` |
| Winsock TCP | `WSAStartup`, `socket`, `connect`, `gethostbyname`, `closesocket`, `recv` — `src/Net/Net_Connect.cpp`, `src/Net/Network.cpp`, `src/Render/SMD_Parser.cpp` | **Requires an adapter.** WebSocket binary frames, or a gateway. See the porting plan |
| `WSAAsyncSelect` + `WM_USER` | `CWsctlc_Connect`, `WndProc` case `WM_USER` | **Requires an adapter.** Deliver `FD_READ` / `FD_WRITE` / `FD_CLOSE` into `gNetwork.OnSocketEvent` from the socket callback. The handlers stay |
| Packet framing and crypto | `PacketFrame_WantsEncrypt` (`src/Net/PacketFrame.cpp`), `MuEmu_send_hook` (`src/Net/MuEmu.cpp`), `src/Net/Crypto.cpp`, `src/Net/Recv/Recv_*.cpp` (~6,074 lines of handlers), `SecondPassword.cpp` (4,505) | **Reusable with small changes.** These are the bytes. They do not open sockets themselves, except where a handler calls `closesocket` |
| HWID | `HWID_Init`, `HWID_Send` in `src/Net/HWID.cpp`. `GetVolumeInformationA`, `UuidCreateSequential`, `GetSystemInfo`. Packet `F1/05` | **Requires an adapter** for the id inputs. **The send path stays.** The server compares this packet |
| DirectSound | `DirectSoundCreate`, `PlayBuffer`, `Sound_UpdatePos` in `src/Sound/Sound.cpp`, `src/Sound/Sound_DS3D.cpp`. `CWaveFile` in `src/Sound/Sound.h` | **Requires an adapter.** Keep the 420-slot API. Replace `LPDIRECTSOUNDBUFFER` |
| SEH | `__try` / `__except` in the eight files listed in the audit | **Requires small edits at each site**, and those edits are mandatory. The keyword is rejected on wasm32 |
| `intrin.h` / `_ReturnAddress` | `src/functions.h:6` | **Reusable with small changes.** One inline substitute. The include fails today because clang's wasm `intrin.h` does `#include_next <intrin.h>` |
| MiniDump / dbghelp | `src/Debug/MiniDump.cpp`, `SetUnhandledExceptionFilter` in `WinMain.cpp` | **Requires substantial rewriting** if crash reports must exist. Otherwise compile the file out of the browser target. It is not gameplay |
| GameGuard | `GameGuard_Init`, `GameGuard_TickCheck`, `src/GameGuard/*`, pipe handles in `globals.h` | **Unknown.** The reconstruction already treats much of this as health stubs. Confirm against a login capture before deleting the timer hook. Do not invent a GameGuard packet |
| Named pipes | `src/Net/Pipe_Legacy.cpp`, `src/Net/Net_PipeQuery.cpp` | **Unknown / likely dead on this port.** 12 + 62 non-blank lines. Confirm there is no live caller before dropping them |
| `version.dll` `FileVersion_Get` | `src/Config/Config_Load.cpp` | **Requires an adapter or can be dropped.** It reads a PE version resource the browser build will not have |
| Threads | none found | **Reusable without changes.** There is nothing to port |
| Inline assembly | none in compiled code | **Reusable without changes** |

## Preamble symbols the browser `stdafx` must provide

These are the types and macros the first successful parse of `globals.h` will demand, beyond hiding `winsock2.h`:

- `DWORD`, `WORD`, `BYTE`, `byte`, `LPBYTE`, `BOOL`, `CHAR`, `LPCSTR`, `LPSTR`
- `HANDLE`, `HWND`, `HDC`, `HGLRC`, `HINSTANCE`, `HFONT`, `HGDIOBJ`
- `UINT`, `WPARAM`, `LPARAM`, `LRESULT`, `WNDPROC`
- `RECT`, `POINT`, `SIZE`, `LPSIZE`
- `SOCKET`, `sockaddr_in`, `hostent`, `WSADATA`
- `HRESULT`, `S_OK`, `E_FAIL`, `FAILED`
- `__int64` or `-fms-extensions`
- `__forceinline` → `inline`
- `__cdecl`, `__fastcall`, `__stdcall`, `__thiscall` as empty or as clang MS extensions
- `GetTickCount`, because `stdafx.h` calls it from `__ftol()`

`#include "stdafx.h"` is a quoted include, so the compiler searches the `.cpp`'s directory first. A `-I` directory cannot override `src/stdafx.h`. The browser preamble has to be an `#ifdef __EMSCRIPTEN__` branch inside `src/stdafx.h`, or the browser target has to compile copies. The native MSVC branch stays as it is. That edit is not done in this audit.

## Build dependencies that stay on the native target

Do not pass the browser flags to `mu97k.vcxproj`. The native link line remains:

`jpeg.lib opengl32.lib glu32.lib winmm.lib ws2_32.lib version.lib dxguid.lib dsound.lib imm32.lib`

The browser target does not link any of those `.lib` files. It links Emscripten's `libGL`, the compiled `jpeg` objects, and later miniaudio.
