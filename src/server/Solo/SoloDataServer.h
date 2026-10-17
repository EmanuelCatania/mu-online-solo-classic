#pragma once

#include "stdafx.h"
#include <string>

using SoloSaveEventCallback = void (*)();

void SoloDataServer_Init(const std::string& saveDir);
void SoloDataServer_SetSaveEventCallback(SoloSaveEventCallback callback);
void SoloJoinServer_ProcessPacket(BYTE* lpMsg, int size);
void SoloDataServer_ProcessPacket(BYTE* lpMsg, int size);
