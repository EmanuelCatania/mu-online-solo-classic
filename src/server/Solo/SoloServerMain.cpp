#include "stdafx.h"
#include "SoloServer.h"
#include "SoloTransport.h"
#include "SoloDataServer.h"
#include "GameMain.h"
#include "MiniDump.h"
#include "Path.h"
#include "QueueTimer.h"
#include "ServerDisplayer.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "Util.h"
#include "Protocol.h"
#include "MonsterManager.h"

static bool g_SoloServerInitialized = false;

extern "C" {

SOLO_API int SoloServer_Init(const char* dataDir, const char* saveDir)
{
	if (g_SoloServerInitialized)
	{
		LogAdd(LOG_BLUE, "[SoloServer] Server already initialized");
		return 1;
	}

	std::setlocale(LC_ALL, "C");
	CMiniDump::Start();
	SetLargeRand();

	const char* actualDataDir = (dataDir && dataDir[0] != '\0') ? dataDir : "./Data";
	const char* actualSaveDir = (saveDir && saveDir[0] != '\0') ? saveDir : "./saves";

	gPath.SetMainPath(actualDataDir);
	SoloDataServer_Init(actualSaveDir);

	char startupPath[MAX_PATH];
	std::snprintf(startupPath, sizeof(startupPath), "%s/GameServerInfo - StartUp.dat", actualDataDir);

	gServerInfo.ReadStartupInfo("GameServerInfo", startupPath);
	gServerDisplayer.Init(nullptr);

	if (gSocketManager.Start((WORD)gServerInfo.m_ServerPort) == 0)
	{
		LogAdd(LOG_RED, "[SoloServer] Could not start SoloSocketManager");
		return 0;
	}

	GameMainInit(nullptr);

	JoinServerConnect();
	DataServerConnect();

	// Setup game tick timers
	gQueueTimer.CreateTimer(QUEUE_TIMER_MONSTER, 100, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_MONSTER_MOVE, 100, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_EVENT, 100, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_VIEWPORT, 1000, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_FIRST, 1000, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_CLOSE, 1000, &QueueTimerCallback);
	gQueueTimer.CreateTimer(QUEUE_TIMER_ACCOUNT_LEVEL, 60000, &QueueTimerCallback);

	g_SoloServerInitialized = true;
	LogAdd(LOG_GREEN, "[SoloServer] Solo Classic MU Server started successfully!");

	return 1;
}

SOLO_API void SoloServer_Tick(int elapsedMs)
{
	if (!g_SoloServerInitialized)
	{
		return;
	}

	gQueueTimer.Tick(elapsedMs);
}

SOLO_API int SoloServer_ConnectPlayer(const char* ipAddress)
{
	if (!g_SoloServerInitialized)
	{
		return -1;
	}

	char ip[16] = "127.0.0.1";
	if (ipAddress && ipAddress[0] != '\0')
	{
		std::strncpy(ip, ipAddress, sizeof(ip) - 1);
		ip[sizeof(ip) - 1] = '\0';
	}

	SOCKET dummySocket = 1;
	int index = gObjAddSearch(dummySocket, ip);
	if (index == -1)
	{
		LogAdd(LOG_RED, "[SoloServer] gObjAddSearch failed");
		return -1;
	}

	if (gObjAdd(dummySocket, ip, index) == -1)
	{
		LogAdd(LOG_RED, "[SoloServer] gObjAdd failed for index %d", index);
		return -1;
	}

	LPOBJ lpObj = &gObj[index];
	if (lpObj->PerSocketContext != nullptr)
	{
		lpObj->PerSocketContext->Socket = dummySocket;
		lpObj->PerSocketContext->Index = index;
	}

	SoloTransport_SetPlayerIndex(index);

	// Send initial greeting packet
	GCConnectAccountSend(index, 1);
	LogAdd(LOG_GREEN, "[SoloServer] Player session created: index %d", index);

	return index;
}

SOLO_API void SoloServer_DisconnectPlayer(int aIndex)
{
	if (OBJECT_USER_RANGE(aIndex) && gObjIsConnected(aIndex))
	{
		CloseClient(aIndex);
	}
}

SOLO_API void SoloServer_ReceiveClientPacket(const uint8_t* data, int size)
{
	if (!g_SoloServerInitialized || !data || size <= 0)
	{
		return;
	}

	int playerIndex = SoloTransport_GetPlayerIndex();
	if (playerIndex < 0 || !OBJECT_USER_RANGE(playerIndex) || !gObjIsConnected(playerIndex))
	{
		LogAdd(LOG_RED, "[SoloServer] Received packet but player index %d is not connected", playerIndex);
		return;
	}

	BYTE head = (data[0] == 0xC1 || data[0] == 0xC3) ? data[2] : data[3];

	ProtocolCore(head, const_cast<BYTE*>(data), size, playerIndex, 0, 0);
}

SOLO_API void SoloServer_SetSendCallback(void (*callback)(const uint8_t* data, int size))
{
	SoloTransport_SetSendCallback(callback);
}

SOLO_API void SoloServer_SetSaveCallback(void (*callback)())
{
	SoloDataServer_SetSaveEventCallback(callback);
}

SOLO_API int SoloServer_GetPlayerCount()
{
	return gObjTotalUser;
}

SOLO_API int SoloServer_GetMonsterCount()
{
	return gObjTotalMonster;
}

} // extern "C"
