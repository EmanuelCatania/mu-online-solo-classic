#pragma once

#include "stdafx.h"
#include <cstdint>
#include <functional>

// Callback type for sending packets to WebAssembly host (JavaScript / Web Worker)
using SoloPacketCallback = void (*)(const uint8_t* data, int size);

void SoloTransport_SetSendCallback(SoloPacketCallback callback);
void SoloTransport_SendToClient(int aIndex, const uint8_t* data, int size);
int  SoloTransport_GetPlayerIndex();
void SoloTransport_SetPlayerIndex(int index);
