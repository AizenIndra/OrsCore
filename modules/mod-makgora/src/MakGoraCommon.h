/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_MAKGORA_COMMON_H
#define MOD_MAKGORA_COMMON_H

#include "ObjectGuid.h"
#include "SharedDefines.h"
#include <cstdint>
#include <string>

namespace MakGora
{
enum class RitualState : uint8
{
    None = 0,
    Pending,       // Challenge accepted, waiting for duel / arena
    Ceremony,      // NPC ceremony in progress (content phase)
    ChampionTrial, // Elite champion encounter
    Arena,         // Active duel
    Resolved
};

enum class RitualOutcome : uint8
{
    None = 0,
    TitleTransfer = 1,
    Permadeath = 2,
    Fled = 3,
    Interrupted = 4
};

enum StringId : uint32
{
    STRING_DISABLED = 900100,
    STRING_CORONATION = 900101,
    STRING_PERMADEATH = 900102,
    STRING_NOT_ELIGIBLE = 900103,
    STRING_WRONG_FACTION = 900104,
    STRING_RITUAL_BEGIN = 900105,
    STRING_GHOST_FOREVER = 900106,
    STRING_CURRENT_LEADER = 900107,
    STRING_NO_LEADER = 900108,
    STRING_ELIGIBLE_SET = 900109,
    STRING_HARDCORE_TOGGLE = 900110,
    STRING_NEED_CHAMPION = 900111,
    STRING_INTERRUPTED = 900112
};

struct LeaderRecord
{
    ObjectGuid::LowType guid = 0;
    std::string name;
    time_t claimedAt = 0;
    uint32 reignCount = 0;
};

struct PlayerRecord
{
    bool eligible = false;
    bool championDefeated = false;
    bool hardcoreBound = false;
    bool permadeath = false;
    uint32 cosmetics = 0;
};

struct RitualSession
{
    ObjectGuid challenger;
    ObjectGuid opponent;
    TeamId teamId = TEAM_NEUTRAL;
    RitualState state = RitualState::None;
    bool hardcore = false;
    time_t startedAt = 0;
};
} // namespace MakGora

#endif
