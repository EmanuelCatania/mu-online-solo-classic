#include "stdafx.h"
#include "SocketManager.h"
#include "SoloTransport.h"
#include "User.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Log.h"
#include "PacketManager.h"
#include "SerialCheck.h"

CSocketManager gSocketManager;

static SoloPacketCallback g_SoloSendCallback = nullptr;
static int g_SoloPlayerIndex = -1;

void SoloTransport_SetSendCallback(SoloPacketCallback callback)
{
	g_SoloSendCallback = callback;
}

void SoloTransport_SendToClient(int aIndex, const uint8_t* data, int size)
{
	if (g_SoloSendCallback != nullptr && data != nullptr && size > 0)
	{
		g_SoloSendCallback(data, size);
	}
}

int SoloTransport_GetPlayerIndex()
{
	return g_SoloPlayerIndex;
}

void SoloTransport_SetPlayerIndex(int index)
{
	g_SoloPlayerIndex = index;
}

CSocketManager::CSocketManager()
{
}

CSocketManager::~CSocketManager()
{
	this->Clean();
}

bool CSocketManager::Start(WORD port)
{
	LogAdd(LOG_BLUE, "[SoloSocketManager] In-memory transport initialized");
	return true;
}

void CSocketManager::Clean()
{
}

bool CSocketManager::CreateListenSocket()
{
	return true;
}

bool CSocketManager::CreateCompletionPort()
{
	return true;
}

bool CSocketManager::CreateAcceptThread()
{
	return true;
}

bool CSocketManager::CreateWorkerThread()
{
	return true;
}

bool CSocketManager::CreateServerQueue()
{
	return true;
}

bool CSocketManager::DataRecv(int index, IO_MAIN_BUFFER* lpIoBuffer)
{
	return true;
}

bool CSocketManager::DataSend(int index, BYTE* lpMsg, int size)
{
	if (lpMsg == nullptr || size <= 0)
	{
		return false;
	}

	BYTE send[MAX_MAIN_PACKET_SIZE];
	memcpy(send, lpMsg, size);

	if (lpMsg[0] == 0xC3 || lpMsg[0] == 0xC4)
	{
		if (lpMsg[0] == 0xC3)
		{
			BYTE save = lpMsg[1];
			lpMsg[1] = (OBJECT_USER_RANGE(index)) ? gSerialCheck[index].GetSendSerial() : 0;
			size = gPacketManager.Encrypt(&send[2], &lpMsg[1], (size - 1)) + 2;
			lpMsg[1] = save;
			send[0] = 0xC3;
			send[1] = size;
		}
		else
		{
			BYTE save = lpMsg[2];
			lpMsg[2] = (OBJECT_USER_RANGE(index)) ? gSerialCheck[index].GetSendSerial() : 0;
			size = gPacketManager.Encrypt(&send[3], &lpMsg[2], (size - 2)) + 3;
			lpMsg[2] = save;
			send[0] = 0xC4;
			send[1] = HIBYTE(size);
			send[2] = LOBYTE(size);
		}
	}

	SoloTransport_SendToClient(index, send, size);
	return true;
}

void CSocketManager::Disconnect(int index)
{
	if (OBJECT_USER_RANGE(index) && gObj[index].Connected != OBJECT_OFFLINE)
	{
		gObjDel(index);
		if (g_SoloPlayerIndex == index)
		{
			g_SoloPlayerIndex = -1;
		}
		LogAdd(LOG_BLUE, "[SoloSocketManager] Player disconnected: index %d", index);
	}
}

void CSocketManager::OnRecv(int index, DWORD IoSize, IO_RECV_CONTEXT* lpIoContext)
{
}

void CSocketManager::OnSend(int index, DWORD IoSize, IO_SEND_CONTEXT* lpIoContext)
{
}
