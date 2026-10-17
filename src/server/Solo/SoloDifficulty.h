#pragma once

#include <cstdint>

enum SOLO_DIFFICULTY_MODE
{
	SOLO_DIFFICULTY_EASY = 0,
	SOLO_DIFFICULTY_MEDIUM = 1,
	SOLO_DIFFICULTY_HARD = 2
};

struct SOLO_DIFFICULTY_CONFIG
{
	int expRate;             // Easy: 300x, Medium: 100x, Hard: 30x
	int eventExpRate;        // Easy: 300x, Medium: 100x, Hard: 30x (Blood Castle / Devil Square)
	int itemDropRate;        // Easy: 150%, Medium: 100%, Hard: 50%
	int zenDropRate;         // Easy: 100%, Medium: 50%, Hard: 25%
	int resetLevel;          // Easy: 300, Medium: 400, Hard: 400
	int grandResetResets;    // Easy: 50, Medium: 100, Hard: 150
	float monsterHpRate;     // Easy: 0.7x, Medium: 1.0x, Hard: 1.5x
	float monsterDmgRate;    // Easy: 0.7x, Medium: 1.0x, Hard: 1.5x
	float deathExpPenalty;   // Easy: 0.0 (None), Medium: 0.03 (Normal 3%), Hard: 0.06 (Double 6%)
	float excDropRate;       // Easy: 1.5x, Medium: 1.0x, Hard: 0.5x
};

extern SOLO_DIFFICULTY_CONFIG g_SoloDifficulty;

void SoloDifficulty_Apply(SOLO_DIFFICULTY_MODE mode);
SOLO_DIFFICULTY_CONFIG SoloDifficulty_Get();
SOLO_DIFFICULTY_MODE SoloDifficulty_GetMode();
const char* SoloDifficulty_GetName(SOLO_DIFFICULTY_MODE mode);
