#include "stdafx.h"
#include "SocketManagerUdp.h"

CSocketManagerUdp gSocketManagerUdp;

CSocketManagerUdp::CSocketManagerUdp()
{
	this->m_socket = INVALID_SOCKET;
	this->m_RecvSize = 0;
	this->m_SendSize = 0;
}

CSocketManagerUdp::~CSocketManagerUdp()
{
}

bool CSocketManagerUdp::Start(WORD port)
{
	return true;
}

bool CSocketManagerUdp::Connect(char* IpAddress, WORD port)
{
	return true;
}

void CSocketManagerUdp::Clean()
{
}

bool CSocketManagerUdp::DataRecv()
{
	return true;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg, int size)
{
	// ConnectServer is not used in solo mode
	return true;
}

DWORD WINAPI CSocketManagerUdp::ServerRecvThread(CSocketManagerUdp* lpSocketManagerUdp)
{
	return 0;
}
