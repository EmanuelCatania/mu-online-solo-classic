#include "stdafx.h"
#include "HackPacketCheck.h"
#include "Log.h"
#include "ReadScript.h"
#include "SerialCheck.h"
#include "Util.h"

CHackPacketCheck gHackPacketCheck;

CHackPacketCheck::CHackPacketCheck()
{
	this->Init();
}

CHackPacketCheck::~CHackPacketCheck()
{

}

void CHackPacketCheck::Init()
{
	for (int n = 0; n < MAX_HACK_PACKET_INFO; n++)
	{
		this->m_HackPacketInfo[n].ResetIndex();

		this->m_HackPacketInfo[n].ResetValue();
	}
}

void CHackPacketCheck::Load(const char* path)
{
	CReadScript* lpReadScript = new CReadScript;

	if (lpReadScript == NULL)
	{
		ErrorMessageBox(READ_SCRIPT_ALLOC_ERROR, path);

		return;
	}

	if (!lpReadScript->Load(path))
	{
		ErrorMessageBox(READ_SCRIPT_FILE_ERROR, path);

		delete lpReadScript;

		return;
	}

	this->Init();

	try
	{
		eTokenResult token;

		while (true)
		{
			token = lpReadScript->GetToken();

			if (token == TOKEN_END || token == TOKEN_END_SECTION)
			{
				break;
			}

			HACK_PACKET_INFO info;

			info.Index = lpReadScript->GetNumber();

			info.Value = lpReadScript->GetAsNumber();

			info.Encrypt = lpReadScript->GetAsNumber();

			info.MaxDelay = lpReadScript->GetAsNumber();

			info.MinCount = lpReadScript->GetAsNumber();

			info.MaxCount = lpReadScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

void CHackPacketCheck::SetInfo(HACK_PACKET_INFO info)
{
	if (info.Index < 0 || info.Index >= MAX_HACK_PACKET_INFO)
	{
		return;
	}

	this->m_HackPacketInfo[info.Index].IndexInfo = info;

	if (info.Value != -1)
	{
		this->m_HackPacketInfo[info.Index].ValueInfo[info.Value] = info;
	}
}

HACK_PACKET_INFO* CHackPacketCheck::GetInfo(int index, int value)
{
	if (index < 0 || index >= MAX_HACK_PACKET_INFO)
	{
		return 0;
	}

	if (value < 0 || value >= MAX_HACK_PACKET_INFO)
	{
		return 0;
	}

	if (this->m_HackPacketInfo[index].IndexInfo.Index != index)
	{
		return 0;
	}

	if (this->m_HackPacketInfo[index].IndexInfo.Value != -1 && this->m_HackPacketInfo[index].ValueInfo[value].Value != value)
	{
		return 0;
	}

	return ((this->m_HackPacketInfo[index].IndexInfo.Value == -1) ? &this->m_HackPacketInfo[index].IndexInfo : &this->m_HackPacketInfo[index].ValueInfo[value]);
}

bool CHackPacketCheck::CheckPacketHack(int aIndex, int index, int value, int encrypt, int serial)
{
	if (serial >= 0 && OBJECT_USER_RANGE(aIndex))
	{
		gSerialCheck[aIndex].CheckSerial(serial);
	}

	return 1;
}

bool CHackPacketCheck::CheckPacketHackMaxDelay(int aIndex, int index, int MaxDelay)
{
	return ((MaxDelay == 0) ? 1 : (((GetTickCount() - gObj[aIndex].HackPacketDelay[index]) >= ((DWORD)MaxDelay)) ? 0 : 1));
}

bool CHackPacketCheck::CheckPacketHackMinCount(int aIndex, int index, int MinCount)
{
	return ((MinCount == 0) ? 1 : (((++gObj[aIndex].HackPacketCount[index]) > ((DWORD)MinCount)) ? 0 : 1));
}

bool CHackPacketCheck::CheckPacketHackMaxCount(int aIndex, int index, int MaxCount)
{
	return ((MaxCount == 0) ? 1 : ((gObj[aIndex].HackPacketCount[index] > ((DWORD)MaxCount)) ? 0 : 1));
}
