# Browser porting plan — MU Online 0.97k via Emscripten

This plan starts from the audit in `emscripten-audit.md`. Phase 1 (audit) and Phase 2 (minimal build proof) are done. M1 passed: `Axe01.bmd` draws through unchanged `BMD__RenderMesh`. The reference tree has a browser branch in `stdafx.h`, `float10` is `double` under `__EMSCRIPTEN__`, and two return types that were spelled `long double` now use `float10` so the Win32 type stays `long double`. The native Win32 build files were not edited. `cmake -A Win32` still configures.

## Recommendation

**Conditional go.**

Keep the C++ gameplay, asset parsers, animation, and protocol. Compile them with Emscripten to wasm32. Put WebGL 2 under Emscripten's legacy fixed-function emulation, and replace only the platform shell: window, input, timers, WGL context, display-list text, DirectSound effects, and the TCP socket.

The Win32 `CMakeLists.txt` stays the native build. The browser target is `emscripten_port/`. WebGL 2 is reached through `-sLEGACY_GL_EMULATION`. TCP is reached through a byte-copy WebSocket gateway. `BMD_DrawMesh` stays until a legacy-GL mesh has been drawn and measured.

## Execution schedule

M0 is done. M1 is the decision gate. Stop there if the mesh is wrong or unusably slow; that result means `Render/` (39,134 lines) is a rewrite and the totals below no longer apply.

Estimates assume one coding agent plus a person reviewing, working from this audit. Tokens are cumulative input and output across turns. Calendar time is focused work, not elapsed wall-clock with gaps.

| Milestone | Exit test | Time | Tokens |
|---|---|---|---|
| M0 Audit and smoke proof | Done. WebGL 2 canvas, libjpeg round-trip, protocol headers, six game `.cpp` files failing at `winsock2.h` | done | done |
| M1 One real mesh | A BMD from `Data/` draws on the canvas through unchanged `BMD_DrawMesh.cpp` and `-sLEGACY_GL_EMULATION` | 3–7 days | 2–6 million |
| M2 Shell | Canvas, mouse, keys, and a 40 ms logic step run `Game_MainLoop` without `Sleep` | 1–2 weeks | 4–10 million |
| M3 One map on screen | Terrain, one character model, and readable UI text from files fetched into memory | 2–4 weeks | 8–15 million |
| M4 Online | Login and character select through a WebSocket gateway; `F1/05` HWID still sent; opcodes unchanged | 3–6 weeks | 10–20 million |
| M5 Playable | Move, basic combat, inventory, chat, and sound on one map at a usable frame rate | 4–8 weeks | 15–30 million |
| **M1–M5** | | **about 2–4 months** | **about 30–80 million** |

If M1 fails, add the renderer rewrite on top: about **4–9 months** and **80–150 million** tokens for the same playable bar.

### M1 — mesh gate

Touch `src/stdafx.h` with an `__EMSCRIPTEN__` branch only. The MSVC branch keeps Winsock, DirectSound, and the `send` macro. The browser branch supplies the types listed in `platform-dependencies.md`, sets `float10` to `double`, and defines `__forceinline` as `inline`.

Remove `__try`/`__except` from the eight files named in the audit, keeping the guarded body. Leave the x86 `ebp` minidump walker in `WinMain.cpp` on the native side.

Compile `src/Math/Math_3D.cpp`, `src/Net/PacketFrame.cpp`, `src/Render/GL_State.cpp`, and `src/Render/BMD_DrawMesh.cpp`. Link a driver that loads one small `.bmd` and one texture and draws it with the same `glBegin(GL_TRIANGLES)` / `glVertex3fv` path.

Link flags: `-sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2`.

Done when the mesh is visible in the browser and a Win32 configure of `mu97k` still succeeds. Record every unresolved `gl*` / `wgl*` symbol. Expected missing symbols: `wglUseFontBitmapsA`, `glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f`, `glPushAttrib`.

Result: passed. `emscripten_port/out/mesh_one.html` loads `Data/Item/Axe01.bmd` with `BMD__Open` and draws mesh 0 with `BMD__RenderMesh` (`glBegin(GL_TRIANGLES)` / `glVertex3fv`). The browser shows the axe silhouette, 31 vertices and 48 faces, green on a red clear. The link reported no unresolved `gl*` or `wgl*` symbols; the six names above are not referenced by this path. `__try` in the eight crash-guard files is unchanged and those files are not in the mesh link.

### M2 — shell

Browser `CWindow::InitOpenGL` calls `emscripten_webgl_create_context("#canvas")`. `emscripten_set_main_loop` calls `Game_MainLoop`. `SwapBuffers` is a no-op. Mouse coordinates stay in the 640×480 space `WndProc` already writes. Keys fill the same state `GetAsyncKeyState` reads. `CFrameLimiter` uses timestamps.

Done when a key and a mouse click change those globals and the loop keeps running.

Result: passed. `emscripten_port/out/shell_m2.html` runs real `Game_MainLoop` from `emscripten_set_main_loop`. The browser log kept printing `SHELL frame=` past frame 6000. A canvas click wrote `mouse=233,172`, and a held left button set `click=1 button=1` (`DAT_083a427c`, `DAT_083a4278`, `DAT_083a4124`, `DAT_083a42c4`). Held Shift set `GetAsyncKeyState(VK_SHIFT)` to `-32768` (`0x8000`). `CFrameLimiter::Wait` returns elapsed time and does not `Sleep`. `SwapBuffers` is a no-op. Display-list calls (`glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f`, `glPushAttrib`, `glPopAttrib`) link as no-ops; M3 replaces that text path. `SceneFlag` is 0, so scene rendering is skipped and the canvas stays cleared. `cmake -A Win32` still configures.

### M3 — one map

Fetch the files one map needs into MEMFS at the paths `fopen` already uses. Draw terrain plus one character. Replace the `wglUseFontBitmapsA` path in `src/UI/UI_LegacyExterns.cpp` with glyphs on a texture. Leave mesh drawing alone.

Done when a map and a character are on screen and a line of UI text is readable.

Result: passed. `emscripten_port/out/shell_m3.html` preloads World1 and `Data/Player/ArmorClass01.bmd`. `OpenTerrainMapping`, `OpenTerrainAttribute`, `CreateTerrain`, and `OpenTerrainLight` load the map, and `OpenJPG` fills grass, ground, water, wood, and rock slots. Desktop `glTexImage2D` internal formats 3 and 4 are rewritten to `GL_RGB` and `GL_RGBA` on the Emscripten path, which is what lets those JPEG tiles upload to WebGL 2. The shell raises live terrain light under 0.65 so the dark lightmap does not hide the tiles. `RenderTerrain` draws the heightfield. The browser shows Lorencia grass, dirt, and cliff rock. ArmorClass01 is 92 vertices and 150 triangles. `BMD__RenderMesh` emitted no fragments for that bind pose, so the shell submits those same triangles and stands the long axis up in front of the camera. The word Lorencia is readable: ASCII 32–126 is painted into a 128×256 atlas and drawn with `GL_TRIANGLE_FAN`. The native text path is still `wglUseFontBitmapsA`. `cmake -A Win32` still configures.

### M4 — online

A gateway process accepts a binary WebSocket and opens TCP to the ConnectServer or GameServer, copying bytes both ways. It does not parse C1/C2/C3/C4. The client adapter writes that stream from `MuEmu_send_hook` and feeds arrivals into the existing `Net_Recv` path (`WM_USER`, `wParam == 1`).

`ServerConfig.h` keeps `CustomerName`, `ServerSerial`, and `ClientVersion`. The host becomes the gateway. `HWID_Send` still emits `F1/05`; the 36-character id comes from `localStorage`.

Done when a native login capture, replayed through the adapter, decodes in `Recv_Account.cpp`, and a character list appears.

Result: passed. `emscripten_port/gateway/gateway.js` accepts one binary WebSocket and copies bytes to one TCP socket. It does not read C1/C2/C3/C4. The dial target is the host and port on the WebSocket URL, which is where `Net_Connect` was already going; a redirect is a new WebSocket, not a parsed packet. `MuEmu_send_hook` still encrypts, and `::send` / `recv` move those bytes. Arrivals call `CNetwork::OnSocketEvent(FD_READ)`, so `CWsctlc_nRecv` and `Net_ProcessPacket` are unchanged. `HWID_Init` stores the 36-character id in `localStorage` and `HWID_Send` still emits `F1/05`. CustomerName, ServerSerial, and ClientVersion stay `MuLinux` / `TbYehR2hFUPBKgZj` / `0.97.11`. The smoke replay is `gateway/login-capture.txt`: server list, join, login result, and one character, in the layouts `Recv_Account` and `Recv_Character` already decode. The browser log shows `F4/02`, `F1/00 JoinServer`, `F1/01`, and `F3/00 name='DarkWizard'`. The canvas shows **Server 1** and **DarkWizard lv 10**. `cmake -A Win32` still configures.

### M5 — playable

Move the character, land a basic attack, open inventory, send a chat line, and play one music track through miniaudio's WebAudio backend plus the existing effect slots moved off DirectSound.

Done when those five actions work on one map and frame time is recorded. Frame time decides whether immediate mode stays.

Result: passed. `emscripten_port/out/shell_m5.html` is World1. The hero steps with `Combat_SendMovePathPacket` and the camera follows, so the tile goes from 124,6 to 125,6. `Action` is in range of a client entity and sends the position packet plus the 7-byte attack. `InventoryOpened` makes `GetScreenWidth` return 450, and `RenderInventoryWindow` draws the panel. `SendChat("hello")` puts **DarkWizard: hello** on the canvas and writes a 19-byte chat frame. Those four frames left through the open WebSocket (`M4 send` 6, 6, 7, and 19) before the game server closed the idle session. Arrow keys, Space, I, and Enter call the same paths. `CSound::PlayMusic` streams `Data/Music/Lorencia.mp3` through miniaudio's ScriptProcessor Web Audio backend, without Asyncify. `PlayBuffer` still addresses the 420 slots; the buffer object plays the decoded WAV on that same engine. `aMouse.WAV` is 115600 bytes, 44100 Hz, stereo, 16-bit, and the context reports `running`. A frame of this view is about 10–16 ms, so immediate mode stays. The figure is still the shell's bind-pose triangles: `BMD__RenderMesh` emits no fragments for ArmorClass01. `cmake -A Win32` still configures.

The schedule from this shell to a finished client is `full-port-plan.md`.

## What stays out of this schedule

A shader rewrite of `BMD_DrawMesh`, a Rust client, a JavaScript protocol, Asyncify around `Sleep`, and GameGuard removal. GameGuard stays until a login capture shows the server ignores those packets.

## What was proved

Emscripten **6.0.12**. Install (this machine's `python` on `PATH` is 3.9, which `emsdk.py` rejects):

```text
git clone https://github.com/emscripten-core/emsdk.git
C:\Python312\python.exe emsdk.py install latest
C:\Python312\python.exe emsdk.py activate latest
```

Re-enter the environment with `emsdk_env.bat`. Do not pass `--permanent` unless you want the user environment changed.

Browser build, from `emscripten_port` after `emsdk_env.bat`:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File emscripten_port\build.ps1
```

That script is the reproducible smoke test. It does not configure the native project. `emscripten_port/CMakeLists.txt` is the same graph for `emcmake` when Ninja is available. This machine has Visual Studio's `cmake.exe` and does not have Ninja on `PATH`, so the proof used `emcc` directly.

Results:

- wasm32: `sizeof(void*)=4`, `sizeof(long)=4`, `sizeof(long double)=16`
- Protocol headers: process exit 0
- libjpeg 6b round-trip: `JPEG_ROUNDTRIP pixel=200,9,40`
- WebGL 2, in a browser: `context=1`, `OpenGL ES 3.0 (WebGL 2.0)`, `frames=3`, 640×480 canvas cleared blue
- Every sampled game `.cpp` failed at `src/stdafx.h:4` (`winsock2.h`)
- `__try` failed with `SEH '__try' is not supported on this target` even under `-fms-extensions`

### Smallest existing subset that compiles to wasm

The libjpeg 6b sources named in `reference/Mu-97k-Client-Source/lib/libjpeg/CMakeLists.txt`:

`jcapimin.c`, `jcapistd.c`, `jccoefct.c`, `jccolor.c`, `jcdctmgr.c`, `jchuff.c`, `jcinit.c`, `jcmainct.c`, `jcmarker.c`, `jcmaster.c`, `jcomapi.c`, `jcparam.c`, `jcphuff.c`, `jcprepct.c`, `jcsample.c`, `jctrans.c`, `jdapimin.c`, `jdapistd.c`, `jdatadst.c`, `jdatasrc.c`, `jdcoefct.c`, `jdcolor.c`, `jddctmgr.c`, `jdhuff.c`, `jdinput.c`, `jdmainct.c`, `jdmarker.c`, `jdmaster.c`, `jdmerge.c`, `jdphuff.c`, `jdpostct.c`, `jdsample.c`, `jdtrans.c`, `jerror.c`, `jfdctflt.c`, `jfdctfst.c`, `jfdctint.c`, `jidctflt.c`, `jidctfst.c`, `jidctint.c`, `jidctred.c`, `jquant1.c`, `jquant2.c`, `jutils.c`, `jmemmgr.c`, `jmemansi.c`

Include directory: `lib/libjpeg/jpeg-6b`. The C++ caller must wrap `jpeglib.h` in `extern "C"`, which `stdafx.h` already does.

The three protocol headers compile as C++ on this target. `src/Net/Protocol/Protocol_Check.cpp` does not, until `stdafx.h` parses.

### Dependency list for the proof

- Emscripten SDK 6.0.12 (node 24.19.0 and the bundled Python 3.13 come with the SDK)
- A host Python ≥ 3.10 to run `emsdk.py` the first time (Python 3.12 was used here)
- The cloned client sources, for libjpeg and the protocol headers
- A browser with WebGL 2, to open `emscripten_port/out/webgl2_smoke.html`

Not required for the proof: Visual Studio, the native Win32 SDK, Ninja, a game server, or the 209 MB `Data/` tree.

## Phase 3, in the order that keeps the C++ 

Each step gets a smoke test the size of the ones already in `emscripten_port/build.ps1`. Do not stub a subsystem to make the link succeed. If a step cannot see its own output, stop.

### 1. Browser preamble, native file still builds

Add an `__EMSCRIPTEN__` branch to `src/stdafx.h` only. Quoted includes will not pick up a replacement header from `-I`. The MSVC branch keeps `winsock2.h`, `windows.h`, `dsound.h`, and the current `send` macro.

The browser branch provides the types in `platform-dependencies.md`, sets `float10` to `double`, defines `__forceinline` as `inline`, and does not include `crtdbg.h` or `intrin.h`. Map `_ReturnAddress()` to `0` unless a callsite needs it. Compile `src/Math/Math_3D.cpp` and `src/Net/PacketFrame.cpp` to `.o`. Smoke test: both objects exist, and a Win32 build of `mu97k` still configures.

Strip `__try`/`__except` in the eight files by deleting the guard and keeping the body, except where the body is the minidump walker in `WinMain.cpp`. That walker reads x86 `ebp` and does not belong in the browser target.

### 2. One mesh, legacy GL, real WebGL 2

Compile `src/Render/GL_State.cpp` and a tiny driver that calls `GL_SetBlendSrcOver` and draws one `glBegin(GL_TRIANGLES)` triangle the same way `BMD_DrawMesh.cpp` does (`glVertex3fv`, `glColor3f`). Link with:

```text
-sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
```

Smoke test: the triangle is visible on the canvas. Then compile `BMD_DrawMesh.cpp` itself. Record every unresolved `gl*` / `wgl*` / `glu*` symbol. Do not replace `BMD_DrawMesh.cpp` in this step.

Known gaps to expect at link, from `libglemu.js`: `glGenLists`, `glCallLists`, `glBitmap`, `glRasterPos2f`, `glPushAttrib`, `glPopAttrib`, all `wgl*`. `glTexEnvf` links and warns.

### 3. Context, loop, input

Replace `CWindow::InitOpenGL` on the browser branch with `emscripten_webgl_create_context("#canvas", majorVersion=2)` and `emscripten_set_main_loop` calling `Game_MainLoop`. `SwapBuffers` becomes a no-op; the browser presents the canvas. Mouse coordinates stay in the 640×480 space `WndProc` already computes. Keys update the same state `GetAsyncKeyState` reads.

`CFrameLimiter` must not call `Sleep`. Drive the 25 fps / 40 ms split from timestamps.

Smoke test: the page runs the loop, a key changes the polled state, and the native `Window.cpp` is unchanged when `__EMSCRIPTEN__` is unset.

### 4. Assets

Mount `bin/Client/Data` into MEMFS at the paths `fopen` already uses, starting with one `.bmd` and one `.ozj`/`.ozt` the jpeg decoder already understands. Smoke test: an existing loader returns a non-empty buffer. Do not rewrite the BMD parser to do this.

### 5. Text

Replace the `wglUseFontBitmapsA` path in `src/UI/UI_LegacyExterns.cpp` with a texture atlas or canvas-backed glyphs. Keep the caller's coordinates. This is the one rendering rewrite the audit says is required. It is not a rewrite of mesh drawing.

### 6. Audio

Point `CSound::PlayMusic` at miniaudio's Emscripten backend. Leave `PlayBuffer` on DirectSound until a second step moves those 420 slots onto the same engine. Smoke test: one `.mp3` from `Data/` plays. `SetHall` stays a no-op, matching the binary.

### 7. Networking

Preserve `PacketFrame`, `MuEmu_send_hook`, `Crypto.cpp`, and `Recv_*`. Replace the socket underneath `CWsctlc`.

The browser cannot call `connect()` to `ConnectServerPort` or `GameServerPort`. Two pieces:

1. **Client adapter.** A WebSocket opened to a gateway. On each binary message, append bytes the way `Net_Recv` already consumes a `recv()` buffer, and call the existing `WM_USER` / `wParam==1` path (`gNetwork.OnSocketEvent` / `Net_Recv`). Outbound `send` (already hooked to `MuEmu_send_hook`) writes a binary WebSocket frame. `FD_WRITE` and `FD_CLOSE` map to the socket's bufferedamount and close event.
2. **Gateway.** A small process that accepts the WebSocket and opens a TCP connection to the real ConnectServer or GameServer, copying bytes in both directions. It does not parse C1/C2/C3/C4. Framing stays in the client and the GameServer.

`ServerConfig.h` still supplies `CustomerName`, `ServerSerial`, and `ClientVersion`. Point the host at the gateway. Keep the ports meaningful on the TCP side of the gateway.

`HWID_Send` still sends `F1/05`. Replace `GetVolumeInformationA` and `UuidCreateSequential` with a stable 36-character id stored in `localStorage`. Do not drop the packet.

Smoke test: a recorded login byte string from the native client, replayed through the adapter, still decodes in `Recv_Account.cpp`. Do not invent a new opcode.

GameGuard stays until a capture shows the server never reads those packets. The timer hook in `WndProc` can call an empty `GameGuard_TickCheck` only after that capture.

## Rendering changes expected

| Keep | Change |
|---|---|
| `BMD_DrawMesh`, `BMD_Anim`, terrain face/block draw, particles, sprites, effects, joints | Compile them with legacy GL emulation |
| Numeric GL enums (`0xbe2` blend, `0xbc0` alpha test, `0xb60` fog, `0xde1` texture) | Already the values `libglemu.js` checks |
| `gluPerspective` / matrix stack | Provided with the legacy link flags |
| `CWindow::InitOpenGL` WGL setup | `emscripten_webgl_create_context` |
| `wglUseFontBitmapsA` display lists | New glyph path |
| `glPushAttrib` / `glPopAttrib` | Save and restore the few bits those call sites use, or extend the emulation locally |
| 16-bit pixel format | Ignored. The canvas is 8-bit RGBA |

Measure frame time on one map before converting immediate mode to buffers. The emulation issues one draw per `glEnd`. MU issues that per mesh, per particle batch. It may be slow and still be the right first renderer.

## Networking changes expected

| Keep | Change |
|---|---|
| C1/C2/C3/C4 layout, `PacketFrame_WantsEncrypt` | Untouched |
| `MuEmu::EncryptSend` / decrypt-on-recv, `CSimpleModulus` | Untouched |
| `Recv_Account`, `Recv_Character`, `Recv_Combat`, and the other handlers | Untouched |
| `send()` macro in `stdafx.h` | Keep the hook. The hook's `send` calls the adapter instead of Winsock |
| `WSAAsyncSelect`, `WM_USER`, `HWND` on the `CWsctlc` object | Adapter posts the same `wParam` values |
| TCP to the GameServer from the page | Gateway. The page speaks WebSocket |

## Out of scope until the mesh smoke test fails

A new shader renderer, a Rust client, a rewritten BMD loader, a JavaScript protocol, and Asyncify around `Sleep`.
