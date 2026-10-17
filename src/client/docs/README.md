# MU Online Client Documentation

This directory contains technical documentation for the client subsystems, WebGL2 port, and platform compatibility.

## Documents

- [client-architecture.md](./client-architecture.md)
  - Complete architecture of the WebGL2 browser client engine.
  - Subsystems overview (Combat, Entity, Item, Model, Net, Render, Scene, Sound, Terrain, UI).
  - Zero-network in-memory bridge (`net_ws.cpp` + `window.soloBridge`).
  - 3 Difficulty Realms (Arcadia, Valhalla, Asgard) & seamless passwordless login flow.
  - Compilation & linking pipeline (`src/client/link-login.bat`).

- [platform-dependencies.md](./platform-dependencies.md)
  - Detailed classification of Win32 dependencies vs WebGL2/WASM adapters.
  - Subsystem line count and complexity breakdown.
  - Emscripten preamble symbol mapping (`src/stdafx.h` & `mu_emscripten.h`).
  - Native Win32 vs Browser WASM targets.
