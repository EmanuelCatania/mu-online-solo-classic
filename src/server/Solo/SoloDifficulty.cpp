#include "stdafx.h"
#include "SoloDifficulty.h"
#include "ServerInfo.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Log.h"

static SOLO_DIFFICULTY_MODE g_CurrentMode = SOLO_DIFFICULTY_EASY;

SOLO_DIFFICULTY_CONFIG g_SoloDifficulty = {
	300,   // expRate (300x)
	300,   // eventExpRate (300x)
	150,   // itemDropRate (150%)
	100,   // zenDropRate (100%)
	300,   // resetLevel (Level 300)
	50,    // grandResetResets (50)
	0.7f,  // monsterHpRate (0.7x)
	0.7f,  // monsterDmgRate (0.7x)
	0.0f,  // deathExpPenalty (None)
	1.5f   // excDropRate (1.5x)
};

static const SOLO_DIFFICULTY_CONFIG kDifficultyPresets[3] = {
	// 0: 🟢 Easy
	{
		300,   // expRate
		300,   // eventExpRate
		150,   // itemDropRate
		100,   // zenDropRate
		300,   // resetLevel
		50,    // grandResetResets
		0.7f,  // monsterHpRate
		0.7f,  // monsterDmgRate
		0.0f,  // deathExpPenalty (None)
		1.5f   // excDropRate
	},
	// 1: 🟡 Medium
	{
		100,   // expRate
		100,   // eventExpRate
		100,   // itemDropRate
		50,    // zenDropRate
		400,   // resetLevel
		100,   // grandResetResets
		1.0f,  // monsterHpRate
		1.0f,  // monsterDmgRate
		0.03f, // deathExpPenalty (Normal 3%)
		1.0f   // excDropRate
	},
	// 2: 🔴 Hard
	{
		30,    // expRate
		30,    // eventExpRate
		50,    // itemDropRate
		25,    // zenDropRate
		400,   // resetLevel
		150,   // grandResetResets
		1.5f,  // monsterHpRate
		1.5f,  // monsterDmgRate
		0.06f, // deathExpPenalty (Double 6%)
		0.5f   // excDropRate
	}
};

void SoloDifficulty_Apply(SOLO_DIFFICULTY_MODE mode)
{
	if (mode < SOLO_DIFFICULTY_EASY || mode > SOLO_DIFFICULTY_HARD)
	{
		mode = SOLO_DIFFICULTY_EASY;
	}

	g_CurrentMode = mode;
	g_SoloDifficulty = kDifficultyPresets[mode];

	// Apply to ServerInfo rates
	for (int i = 0; i < MAX_ACCOUNT_LEVEL; ++i)
	{
		gServerInfo.m_AddExperienceRate[i] = g_SoloDifficulty.expRate;
		gServerInfo.m_AddEventExperienceRate[i] = g_SoloDifficulty.eventExpRate;
		gServerInfo.m_ItemDropRate[i] = g_SoloDifficulty.itemDropRate;
		gServerInfo.m_MoneyAmountDropRate[i] = g_SoloDifficulty.zenDropRate;
		gServerInfo.m_CommandResetLevel[i] = g_SoloDifficulty.resetLevel;
	}

	// Strictly disable player-to-player trade
	gServerInfo.m_TradeSwitch = 0;

	LogAdd(LOG_BLUE, "[SoloDifficulty] Applied Mode: %s (EXP: %dx, ItemDrop: %d%%, Zen: %d%%, Monster: %.1fx)",
		SoloDifficulty_GetName(mode),
		g_SoloDifficulty.expRate,
		g_SoloDifficulty.itemDropRate,
		g_SoloDifficulty.zenDropRate,
		g_SoloDifficulty.monsterHpRate);
}

SOLO_DIFFICULTY_CONFIG SoloDifficulty_Get()
{
	return g_SoloDifficulty;
}

SOLO_DIFFICULTY_MODE SoloDifficulty_GetMode()
{
	return g_CurrentMode;
}

const char* SoloDifficulty_GetName(SOLO_DIFFICULTY_MODE mode)
{
	switch (mode)
	{
		case SOLO_DIFFICULTY_EASY:   return "Easy";
		case SOLO_DIFFICULTY_MEDIUM: return "Medium";
		case SOLO_DIFFICULTY_HARD:   return "Hard";
		default:                     return "Unknown";
	}
}
