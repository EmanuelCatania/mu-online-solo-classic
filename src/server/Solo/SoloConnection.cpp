#include "stdafx.h"
#include "Connection.h"
#include "SoloDataServer.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Log.h"

extern CConnection gJoinServerConnection;
extern CConnection gDataServerConnection;

CConnection::CConnection()
{
	this->m_hwnd = nullptr;
	this->m_socket = INVALID_SOCKET;
	this->m_EventHandlerThread = nullptr;
	this->m_hEvent = nullptr;
	this->m_RecvSize = 0;
	this->m_SendSize = 0;
	this->wsProtocolCore = nullptr;
}

CConnection::~CConnection()
{
	this->Disconnect();
}

void CConnection::Init(HWND hwnd, const char* name, ProtocolCoreFn function)
{
	this->m_hwnd = hwnd;
	this->sConnectionName = name ? name : "";
	this->wsProtocolCore = function;
}

bool CConnection::Connect(const char* IpAddress, WORD port)
{
	LogAdd(LOG_BLUE, "[SoloConnection] %s virtual connected", this->sConnectionName.c_str());
	return true;
}

void CConnection::Disconnect()
{
}

bool CConnection::CheckState()
{
	return true;
}

bool CConnection::DataRecv()
{
	return true;
}

bool CConnection::DataSend(BYTE* lpMsg, int size)
{
	if (lpMsg == nullptr || size <= 0)
	{
		return false;
	}

	if (this == &gJoinServerConnection || this->sConnectionName == "JoinServerConnection")
	{
		SoloJoinServer_ProcessPacket(lpMsg, size);
		return true;
	}

	if (this == &gDataServerConnection || this->sConnectionName == "DataServerConnection")
	{
		SoloDataServer_ProcessPacket(lpMsg, size);
		return true;
	}

	return false;
}

bool CConnection::DataSendEx()
{
	return true;
}

bool CConnection::CreateEventHandler()
{
	return true;
}

DWORD WINAPI CConnection::SocketEventsThread(CConnection* lpConnection)
{
	return 0;
}
