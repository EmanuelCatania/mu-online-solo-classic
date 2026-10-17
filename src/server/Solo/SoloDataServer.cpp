#include "stdafx.h"
#include "SoloDataServer.h"
#include "DSProtocol.h"
#include "JSProtocol.h"
#include "DefaultClassInfo.h"
#include "Warehouse.h"
#include "ServerDisplayer.h"
#include "Log.h"
#include "Util.h"

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

static std::string g_SaveDirectory = "./saves";
static SoloSaveEventCallback g_SoloSaveEventCallback = nullptr;

void SoloDataServer_SetSaveEventCallback(SoloSaveEventCallback callback)
{
	g_SoloSaveEventCallback = callback;
}

struct SOLO_SAVED_CHARACTER_HEADER
{
	char name[11];
	BYTE slot;
	WORD level;
	BYTE Class;
	BYTE CtlCode;
	BYTE equipment[60];
};

struct SOLO_ACCOUNT_DATA
{
	char account[11];
	BYTE characterCount;
	SOLO_SAVED_CHARACTER_HEADER characters[5];
};

static std::string GetAccountFilePath(const char* account)
{
	return g_SaveDirectory + "/account_" + std::string(account) + ".dat";
}

static std::string GetCharacterFilePath(const char* name)
{
	return g_SaveDirectory + "/char_" + std::string(name) + ".dat";
}

static std::string GetWarehouseFilePath(const char* account)
{
	return g_SaveDirectory + "/warehouse_" + std::string(account) + ".dat";
}

static bool LoadAccountData(const char* account, SOLO_ACCOUNT_DATA& data)
{
	std::memset(&data, 0, sizeof(data));
	std::memcpy(data.account, account, sizeof(data.account));

	std::string path = GetAccountFilePath(account);
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	return true;
}

static bool SaveAccountData(const SOLO_ACCOUNT_DATA& data)
{
	std::string path = GetAccountFilePath(data.account);
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		return false;
	}

	file.write(reinterpret_cast<const char*>(&data), sizeof(data));
	return true;
}

void SoloDataServer_Init(const std::string& saveDir)
{
	g_SaveDirectory = saveDir;
	std::error_code ec;
	fs::create_directories(g_SaveDirectory, ec);
	LogAdd(LOG_BLUE, "[SoloDataServer] Initialized storage at %s", g_SaveDirectory.c_str());
}

void SoloJoinServer_ProcessPacket(BYTE* lpMsg, int size)
{
	BYTE head = (lpMsg[0] == 0xC1) ? lpMsg[2] : lpMsg[3];

	switch (head)
	{
		case 0x00: // Server info
		{
			break;
		}

		case 0x01: // Connect account
		{
			auto* lpSend = reinterpret_cast<SDHP_CONNECT_ACCOUNT_SEND*>(lpMsg);

			SDHP_CONNECT_ACCOUNT_RECV pMsg;
			pMsg.header.set(0x01, sizeof(pMsg));
			pMsg.index = lpSend->index;
			std::memcpy(pMsg.account, lpSend->account, sizeof(pMsg.account));
			pMsg.result = 1; // 1 = Login success
			pMsg.AccountLevel = 0;
			std::memset(pMsg.AccountExpireDate, 0, sizeof(pMsg.AccountExpireDate));

			LogAdd(LOG_GREEN, "[SoloJoinServer] Account login approved: %s", lpSend->account);
			JGConnectAccountRecv(&pMsg);
			break;
		}

		case 0x02: // Disconnect account
		{
			break;
		}

		case 0x03: // Account level
		{
			auto* lpSend = reinterpret_cast<SDHP_ACCOUNT_LEVEL_SEND*>(lpMsg);

			SDHP_ACCOUNT_LEVEL_RECV pMsg;
			pMsg.header.set(0x03, sizeof(pMsg));
			pMsg.index = lpSend->index;
			std::memcpy(pMsg.account, lpSend->account, sizeof(pMsg.account));
			pMsg.AccountLevel = 0;
			std::memset(pMsg.AccountExpireDate, 0, sizeof(pMsg.AccountExpireDate));

			JGAccountLevelRecv(&pMsg);
			break;
		}
	}
}

void SoloDataServer_ProcessPacket(BYTE* lpMsg, int size)
{
	BYTE head = (lpMsg[0] == 0xC1) ? lpMsg[2] : lpMsg[3];
	BYTE subhead = (lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4];

	switch (head)
	{
		case 0x00: // Server info
		{
			break;
		}

		case 0x01: // Character operations
		{
			switch (subhead)
			{
				case 0x00: // Character list request
				{
					auto* lpSend = reinterpret_cast<SDHP_CHARACTER_LIST_SEND*>(lpMsg);

					SOLO_ACCOUNT_DATA accData;
					LoadAccountData(lpSend->account, accData);

					BYTE sendBuffer[sizeof(SDHP_CHARACTER_LIST_RECV) + 5 * sizeof(SDHP_CHARACTER_LIST)];
					std::memset(sendBuffer, 0, sizeof(sendBuffer));

					auto* pListRecv = reinterpret_cast<SDHP_CHARACTER_LIST_RECV*>(sendBuffer);
					pListRecv->header.set(0x01, 0x00, static_cast<WORD>(sizeof(SDHP_CHARACTER_LIST_RECV) + accData.characterCount * sizeof(SDHP_CHARACTER_LIST)));
					pListRecv->index = lpSend->index;
					std::memcpy(pListRecv->account, lpSend->account, sizeof(pListRecv->account));
					pListRecv->count = accData.characterCount;

					auto* pEntries = reinterpret_cast<SDHP_CHARACTER_LIST*>(sendBuffer + sizeof(SDHP_CHARACTER_LIST_RECV));
					for (int i = 0; i < accData.characterCount; ++i)
					{
						pEntries[i].slot = accData.characters[i].slot;
						std::memcpy(pEntries[i].name, accData.characters[i].name, sizeof(pEntries[i].name));
						pEntries[i].level = accData.characters[i].level;
						pEntries[i].Class = accData.characters[i].Class;
						pEntries[i].CtlCode = accData.characters[i].CtlCode;
						std::memcpy(pEntries[i].Inventory, accData.characters[i].equipment, sizeof(pEntries[i].Inventory));
					}

					LogAdd(LOG_BLUE, "[SoloDataServer] Sent character list (%d chars) for account: %s", accData.characterCount, lpSend->account);
					DGCharacterListRecv(pListRecv);
					break;
				}

				case 0x01: // Character create
				{
					auto* lpSend = reinterpret_cast<SDHP_CHARACTER_CREATE_SEND*>(lpMsg);

					SOLO_ACCOUNT_DATA accData;
					LoadAccountData(lpSend->account, accData);

					BYTE result = 0;
					BYTE slot = 0;

					if (accData.characterCount < 5)
					{
						// Find next free slot
						bool usedSlots[5] = { false };
						for (int i = 0; i < accData.characterCount; ++i)
						{
							if (accData.characters[i].slot < 5)
							{
								usedSlots[accData.characters[i].slot] = true;
							}
						}

						for (BYTE s = 0; s < 5; ++s)
						{
							if (!usedSlots[s])
							{
								slot = s;
								break;
							}
						}

						int idx = accData.characterCount++;
						std::memcpy(accData.characters[idx].name, lpSend->name, sizeof(accData.characters[idx].name));
						accData.characters[idx].slot = slot;
						accData.characters[idx].level = 1;
						accData.characters[idx].Class = lpSend->Class;
						accData.characters[idx].CtlCode = 0;
						std::memset(accData.characters[idx].equipment, 0xFF, sizeof(accData.characters[idx].equipment));

						SaveAccountData(accData);

						// Create initial character save
						SDHP_CHARACTER_INFO_SAVE_SEND initChar;
						std::memset(&initChar, 0, sizeof(initChar));
						initChar.header.set(0x01, 0x04, sizeof(initChar));
						initChar.index = lpSend->index;
						std::memcpy(initChar.account, lpSend->account, sizeof(initChar.account));
						std::memcpy(initChar.name, lpSend->name, sizeof(initChar.name));
						initChar.Level = 1;
						initChar.Class = lpSend->Class;
						initChar.LevelUpPoint = 0;
						initChar.Experience = 0;
						initChar.Money = 1000;

						int classIdx = lpSend->Class % MAX_CLASS;
						initChar.Strength = gDefaultClassInfo.m_DefaultClassInfo[classIdx].Strength;
						initChar.Dexterity = gDefaultClassInfo.m_DefaultClassInfo[classIdx].Dexterity;
						initChar.Vitality = gDefaultClassInfo.m_DefaultClassInfo[classIdx].Vitality;
						initChar.Energy = gDefaultClassInfo.m_DefaultClassInfo[classIdx].Energy;
						initChar.Life = static_cast<DWORD>(gDefaultClassInfo.m_DefaultClassInfo[classIdx].MaxLife);
						initChar.MaxLife = static_cast<DWORD>(gDefaultClassInfo.m_DefaultClassInfo[classIdx].MaxLife);
						initChar.Mana = static_cast<DWORD>(gDefaultClassInfo.m_DefaultClassInfo[classIdx].MaxMana);
						initChar.MaxMana = static_cast<DWORD>(gDefaultClassInfo.m_DefaultClassInfo[classIdx].MaxMana);
						initChar.BP = 20;
						initChar.MaxBP = 20;

						initChar.Map = (lpSend->Class == CLASS_FE) ? 3 : 0;
						initChar.X = (lpSend->Class == CLASS_FE) ? 175 : 125;
						initChar.Y = (lpSend->Class == CLASS_FE) ? 115 : 125;
						initChar.Dir = 3;

						std::memset(initChar.Inventory, 0xFF, sizeof(initChar.Inventory));
						std::memset(initChar.Skill, 0xFF, sizeof(initChar.Skill));
						std::memset(initChar.Quest, 0xFF, sizeof(initChar.Quest));

						std::string charPath = GetCharacterFilePath(lpSend->name);
						std::ofstream charFile(charPath, std::ios::binary | std::ios::trunc);
						if (charFile.is_open())
						{
							charFile.write(reinterpret_cast<const char*>(&initChar), sizeof(initChar));
						}

						result = 1; // Success
						LogAdd(LOG_GREEN, "[SoloDataServer] Character created: %s, class %d, slot %d", lpSend->name, lpSend->Class, slot);
						if (g_SoloSaveEventCallback) g_SoloSaveEventCallback();
					}

					SDHP_CHARACTER_CREATE_RECV pMsg;
					pMsg.header.set(0x01, 0x01, sizeof(pMsg));
					pMsg.index = lpSend->index;
					std::memcpy(pMsg.account, lpSend->account, sizeof(pMsg.account));
					std::memcpy(pMsg.name, lpSend->name, sizeof(pMsg.name));
					pMsg.result = result;
					pMsg.slot = slot;
					std::memset(pMsg.equipment, 0xFF, sizeof(pMsg.equipment));

					DGCharacterCreateRecv(&pMsg);
					break;
				}

				case 0x02: // Character delete
				{
					auto* lpSend = reinterpret_cast<SDHP_CHARACTER_DELETE_SEND*>(lpMsg);

					SOLO_ACCOUNT_DATA accData;
					LoadAccountData(lpSend->account, accData);

					int foundIndex = -1;
					for (int i = 0; i < accData.characterCount; ++i)
					{
						if (std::strcmp(accData.characters[i].name, lpSend->name) == 0)
						{
							foundIndex = i;
							break;
						}
					}

					if (foundIndex >= 0)
					{
						for (int i = foundIndex; i < accData.characterCount - 1; ++i)
						{
							accData.characters[i] = accData.characters[i + 1];
						}
						accData.characterCount--;
						SaveAccountData(accData);

						std::error_code ec;
						fs::remove(GetCharacterFilePath(lpSend->name), ec);
						LogAdd(LOG_BLUE, "[SoloDataServer] Character deleted: %s", lpSend->name);
						if (g_SoloSaveEventCallback) g_SoloSaveEventCallback();
					}

					SDHP_CHARACTER_DELETE_RECV pMsg;
					pMsg.header.set(0x01, 0x02, sizeof(pMsg));
					pMsg.index = lpSend->index;
					std::memcpy(pMsg.account, lpSend->account, sizeof(pMsg.account));
					pMsg.result = 1;

					DGCharacterDeleteRecv(&pMsg);
					break;
				}

				case 0x03: // Character info load
				{
					auto* lpSend = reinterpret_cast<SDHP_CHARACTER_INFO_SEND*>(lpMsg);

					SDHP_CHARACTER_INFO_RECV pRecv;
					std::memset(&pRecv, 0, sizeof(pRecv));
					pRecv.header.set(0x01, 0x03, sizeof(pRecv));
					pRecv.index = lpSend->index;
					std::memcpy(pRecv.account, lpSend->account, sizeof(pRecv.account));
					std::memcpy(pRecv.name, lpSend->name, sizeof(pRecv.name));
					pRecv.result = 0;

					std::string charPath = GetCharacterFilePath(lpSend->name);
					std::ifstream file(charPath, std::ios::binary);
					if (file.is_open())
					{
						SDHP_CHARACTER_INFO_SAVE_SEND saved;
						file.read(reinterpret_cast<char*>(&saved), sizeof(saved));
						if (file.gcount() == sizeof(saved))
						{
							pRecv.result = 1;
							pRecv.Class = saved.Class;
							pRecv.Level = saved.Level;
							pRecv.LevelUpPoint = saved.LevelUpPoint;
							pRecv.Experience = saved.Experience;
							pRecv.Money = saved.Money;
							pRecv.Strength = saved.Strength;
							pRecv.Dexterity = saved.Dexterity;
							pRecv.Vitality = saved.Vitality;
							pRecv.Energy = saved.Energy;
							pRecv.Life = saved.Life;
							pRecv.MaxLife = saved.MaxLife;
							pRecv.Mana = saved.Mana;
							pRecv.MaxMana = saved.MaxMana;
							pRecv.BP = saved.BP;
							pRecv.MaxBP = saved.MaxBP;
							std::memcpy(pRecv.Inventory, saved.Inventory, sizeof(pRecv.Inventory));
							std::memcpy(pRecv.Skill, saved.Skill, sizeof(pRecv.Skill));
							pRecv.Map = saved.Map;
							pRecv.X = saved.X;
							pRecv.Y = saved.Y;
							pRecv.Dir = saved.Dir;
							pRecv.PKCount = saved.PKCount;
							pRecv.PKLevel = saved.PKLevel;
							pRecv.PKTime = saved.PKTime;
							pRecv.CtlCode = 0;
							std::memcpy(pRecv.Quest, saved.Quest, sizeof(pRecv.Quest));
							pRecv.FruitAddPoint = saved.FruitAddPoint;
							pRecv.FruitSubPoint = saved.FruitSubPoint;
							std::memcpy(pRecv.Effect, saved.Effect, sizeof(pRecv.Effect));
							pRecv.Reset = 0;
							pRecv.GrandReset = 0;

							LogAdd(LOG_GREEN, "[SoloDataServer] Loaded character: %s (Lv: %d, Map: %d, X: %d, Y: %d)",
								lpSend->name, pRecv.Level, pRecv.Map, pRecv.X, pRecv.Y);
						}
					}

					DGCharacterInfoRecv(&pRecv);
					break;
				}

				case 0x04: // Character info save
				{
					auto* lpSave = reinterpret_cast<SDHP_CHARACTER_INFO_SAVE_SEND*>(lpMsg);

					std::string charPath = GetCharacterFilePath(lpSave->name);
					std::ofstream file(charPath, std::ios::binary | std::ios::trunc);
					if (file.is_open())
					{
						file.write(reinterpret_cast<const char*>(lpSave), sizeof(*lpSave));
						LogAdd(LOG_BLUE, "[SoloDataServer] Saved character: %s", lpSave->name);
					}

					// Update cached character info in account file
					SOLO_ACCOUNT_DATA accData;
					if (LoadAccountData(lpSave->account, accData))
					{
						for (int i = 0; i < accData.characterCount; ++i)
						{
							if (std::strcmp(accData.characters[i].name, lpSave->name) == 0)
							{
								accData.characters[i].level = lpSave->Level;
								accData.characters[i].Class = lpSave->Class;
								for (int s = 0; s < 9; ++s)
								{
									std::memcpy(&accData.characters[i].equipment[s * 5], lpSave->Inventory[s], 5);
								}
								break;
							}
						}
						SaveAccountData(accData);
						if (g_SoloSaveEventCallback) g_SoloSaveEventCallback();
					}
					break;
				}
			}
			break;
		}

		case 0x02: // Items / Warehouse
		{
			switch (subhead)
			{
				case 0x00: // Create item (drop / chaos box / inventory grant)
				{
					auto* lpSend = reinterpret_cast<SDHP_CREATE_ITEM_SEND*>(lpMsg);

					SDHP_CREATE_ITEM_RECV pRecv;
					std::memset(&pRecv, 0, sizeof(pRecv));
					pRecv.header.set(0x02, 0x00, sizeof(pRecv));
					pRecv.index = lpSend->index;
					std::memcpy(pRecv.account, lpSend->account, sizeof(pRecv.account));
					pRecv.X = lpSend->X;
					pRecv.Y = lpSend->Y;
					pRecv.Map = lpSend->Map;
					static DWORD s_itemSerial = 100000;
					pRecv.Serial = ++s_itemSerial;
					pRecv.ItemIndex = lpSend->ItemIndex;
					pRecv.Level = lpSend->Level;
					pRecv.Dur = lpSend->Dur;
					pRecv.SkillOption = lpSend->SkillOption;
					pRecv.LuckOption = lpSend->LuckOption;
					pRecv.AddOption = lpSend->AddOption;
					pRecv.ExceOption = lpSend->ExceOption;
					pRecv.LootIndex = lpSend->LootIndex;

					DGCreateItemRecv(&pRecv);
					break;
				}

				case 0x01: // Inventory save
				{
					auto* lpSave = reinterpret_cast<SDHP_INVENTORY_ITEM_SAVE_SEND*>(lpMsg);
					std::string charPath = GetCharacterFilePath(lpSave->name);
					std::ifstream inFile(charPath, std::ios::binary);
					if (inFile.is_open())
					{
						SDHP_CHARACTER_INFO_SAVE_SEND saved;
						inFile.read(reinterpret_cast<char*>(&saved), sizeof(saved));
						inFile.close();
						if (inFile.gcount() == sizeof(saved))
						{
							std::memcpy(saved.Inventory, lpSave->Inventory, sizeof(saved.Inventory));
							std::ofstream outFile(charPath, std::ios::binary | std::ios::trunc);
							if (outFile.is_open())
							{
								outFile.write(reinterpret_cast<const char*>(&saved), sizeof(saved));
							}
						}
					}

					SOLO_ACCOUNT_DATA accData;
					if (LoadAccountData(lpSave->account, accData))
					{
						for (int i = 0; i < accData.characterCount; ++i)
						{
							if (std::strcmp(accData.characters[i].name, lpSave->name) == 0)
							{
								for (int s = 0; s < 9; ++s)
								{
									std::memcpy(&accData.characters[i].equipment[s * 5], lpSave->Inventory[s], 5);
								}
								break;
							}
						}
						SaveAccountData(accData);
						if (g_SoloSaveEventCallback) g_SoloSaveEventCallback();
					}
					break;
				}

				case 0x02: // Warehouse info request
				{
					auto* lpSend = reinterpret_cast<SDHP_WAREHOUSE_ITEM_SEND*>(lpMsg);

					SDHP_WAREHOUSE_ITEM_RECV pRecv;
					std::memset(&pRecv, 0, sizeof(pRecv));
					pRecv.header.set(0x02, 0x02, sizeof(pRecv));
					pRecv.index = lpSend->index;
					std::memcpy(pRecv.account, lpSend->account, sizeof(pRecv.account));
					std::memset(pRecv.WarehouseItem, 0xFF, sizeof(pRecv.WarehouseItem));
					pRecv.WarehouseMoney = 0;
					pRecv.WarehousePassword = 0;

					std::string whPath = GetWarehouseFilePath(lpSend->account);
					std::ifstream file(whPath, std::ios::binary);
					if (file.is_open())
					{
						file.read(reinterpret_cast<char*>(&pRecv), sizeof(pRecv));
						pRecv.index = lpSend->index;
					}

					gWarehouse.DGWarehouseItemRecv(&pRecv);
					break;
				}

				case 0x04: // Warehouse save
				{
					auto* lpSave = reinterpret_cast<SDHP_WAREHOUSE_ITEM_SAVE_SEND*>(lpMsg);
					std::string whPath = GetWarehouseFilePath(lpSave->account);
					std::ofstream file(whPath, std::ios::binary | std::ios::trunc);
					if (file.is_open())
					{
						file.write(reinterpret_cast<const char*>(lpSave), sizeof(*lpSave));
						LogAdd(LOG_BLUE, "[SoloDataServer] Warehouse saved for %s", lpSave->account);
						if (g_SoloSaveEventCallback) g_SoloSaveEventCallback();
					}
					break;
				}
			}
			break;
		}

		case 0x03: // Option data
		{
			switch (subhead)
			{
				case 0x00: // Option data load
				{
					auto* lpSend = reinterpret_cast<SDHP_OPTION_DATA_SEND*>(lpMsg);
					SDHP_OPTION_DATA_RECV pRecv;
					std::memset(&pRecv, 0, sizeof(pRecv));
					pRecv.header.set(0x03, 0x00, sizeof(pRecv));
					pRecv.index = lpSend->index;
					std::memcpy(pRecv.account, lpSend->account, sizeof(pRecv.account));
					std::memcpy(pRecv.name, lpSend->name, sizeof(pRecv.name));
					std::memset(pRecv.SkillKey, 0xFF, sizeof(pRecv.SkillKey));
					pRecv.GameOption = 0;
					pRecv.QKey = 0;
					pRecv.WKey = 0;
					pRecv.EKey = 0;
					pRecv.ChatWindow = 0;
					DGOptionDataRecv(&pRecv);
					break;
				}

				case 0x01: // Option data save
				{
					break;
				}
			}
			break;
		}
	}
}
