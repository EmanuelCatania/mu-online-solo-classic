#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__EMSCRIPTEN__)
#define SOLO_API __attribute__((used))
#else
#define SOLO_API
#endif

// Initialize Solo GameServer with paths to game data and player save directories
SOLO_API int  SoloServer_Init(const char* dataDir, const char* saveDir);

// Advance server game loop by elapsedMs
SOLO_API void SoloServer_Tick(int elapsedMs);

// Connect a local player session and return player index
SOLO_API int  SoloServer_ConnectPlayer(const char* ipAddress);

// Disconnect player session
SOLO_API void SoloServer_DisconnectPlayer(int aIndex);

// Feed incoming packet from Client to GameServer
SOLO_API void SoloServer_ReceiveClientPacket(const uint8_t* data, int size);

// Register callback for packets emitted by GameServer towards Client
SOLO_API void SoloServer_SetSendCallback(void (*callback)(const uint8_t* data, int size));

// Register callback triggered whenever player data or warehouse is saved
SOLO_API void SoloServer_SetSaveCallback(void (*callback)());

// Query basic server stats
SOLO_API int  SoloServer_GetPlayerCount();
SOLO_API int  SoloServer_GetMonsterCount();

// Difficulty management (0: Easy, 1: Medium, 2: Hard)
SOLO_API void SoloServer_SetDifficulty(int mode);
SOLO_API int  SoloServer_GetDifficulty();

#ifdef __cplusplus
}
#endif

