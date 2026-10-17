#pragma once

#include "stdafx.h"
#include <string>

void SoloDataServer_Init(const std::string& saveDir);
void SoloJoinServer_ProcessPacket(BYTE* lpMsg, int size);
void SoloDataServer_ProcessPacket(BYTE* lpMsg, int size);
