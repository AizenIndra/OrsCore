/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_COMMON_H
#define MOD_AIWORLD_COMMON_H

#include "ObjectGuid.h"
#include <cstdint>
#include <string>
#include <vector>

namespace AiWorld
{
constexpr uint32 ZONE_HILLSBRAD_FOOTHILLS = 267;

enum class FactionSide : uint8
{
    Neutral = 0,
    Alliance = 1,
    Horde = 2
};

enum class TerritoryState : uint8
{
    Contested = 0,
    AllianceHeld = 1,
    HordeHeld = 2
};

enum class DirectorActionType : uint8
{
    None = 0,
    ChangeTerritory = 1,
    DeployBots = 2,
    StartEvent = 3,
    AdjustTension = 4,
    Announce = 5
};

enum class AiTaskType : uint8
{
    AnalyzeHistory = 0,
    ProposeScenario = 1,
    GenerateDialogue = 2
};

enum class BotOrderType : uint8
{
    Defend = 0,
    Attack = 1,
    Assist = 2,
    Group = 3,
    Deploy = 4
};

enum class WorldEventState : uint8
{
    Scheduled = 0,
    Active = 1,
    Completed = 2,
    Cancelled = 3
};

enum EventTemplate : uint32
{
    EVENT_NONE = 0,
    EVENT_HILLSBRAD_SKIRMISH = 1
};

enum StringId : uint32
{
    STRING_DISABLED = 900200,
    STRING_TERRITORY_FLIP = 900201,
    STRING_EVENT_START = 900202,
    STRING_EVENT_END = 900203,
    STRING_BOTS_DEPLOY = 900204,
    STRING_STATUS_HEADER = 900205
};

struct ZoneSnapshot
{
    uint32 zoneId = 0;
    uint32 alliancePlayers = 0;
    uint32 hordePlayers = 0;
    uint32 allianceBots = 0;
    uint32 hordeBots = 0;
    uint32 pvpKillsAlliance = 0;
    uint32 pvpKillsHorde = 0;
    TerritoryState control = TerritoryState::Contested;
    uint32 tension = 0;
    int32 controlScore = 0;
    uint32 unixTime = 0;
};

struct DirectorAction
{
    DirectorActionType type = DirectorActionType::None;
    uint32 zoneId = 0;
    FactionSide beneficiary = FactionSide::Neutral;
    uint32 eventTemplateId = 0;
    uint32 botCount = 0;
    uint8 priority = 0;
    std::string reason;
};

struct WorldEvent
{
    uint64 id = 0;
    uint32 templateId = 0;
    uint32 zoneId = 0;
    FactionSide aggressor = FactionSide::Neutral;
    uint32 startUnix = 0;
    uint32 endUnix = 0;
    WorldEventState state = WorldEventState::Scheduled;
    std::string reason;
};

struct BotOrder
{
    ObjectGuid botGuid;
    uint32 zoneId = 0;
    BotOrderType orderType = BotOrderType::Defend;
    FactionSide side = FactionSide::Neutral;
    uint32 count = 1;
    uint32 expireUnix = 0;
    std::string reason;
};

struct TerritoryRecord
{
    uint32 zoneId = 0;
    TerritoryState state = TerritoryState::Contested;
    FactionSide controller = FactionSide::Neutral;
    int32 controlScore = 0;
    uint32 tension = 0;
    uint32 updatedAt = 0;
};

struct EventHistoryRecord
{
    uint64 id = 0;
    uint32 zoneId = 0;
    std::string eventType;
    uint32 templateId = 0;
    FactionSide aggressor = FactionSide::Neutral;
    FactionSide winner = FactionSide::Neutral;
    std::string payloadJson;
    uint32 createdAt = 0;
};

struct FactionScoreRecord
{
    FactionSide side = FactionSide::Neutral;
    int32 windowHour = 0;
    int32 windowDay = 0;
    int64 total = 0;
    uint32 updatedAt = 0;
};

struct LlmProposal
{
    bool valid = false;
    DirectorActionType action = DirectorActionType::None;
    FactionSide beneficiary = FactionSide::Neutral;
    uint32 botCount = 0;
    uint32 eventTemplateId = 0;
    std::string reason;
    std::string dialogue;
};

inline char const* SideName(FactionSide side)
{
    switch (side)
    {
        case FactionSide::Alliance: return "Alliance";
        case FactionSide::Horde: return "Horde";
        default: return "Neutral";
    }
}

inline char const* TerritoryName(TerritoryState state)
{
    switch (state)
    {
        case TerritoryState::AllianceHeld: return "AllianceHeld";
        case TerritoryState::HordeHeld: return "HordeHeld";
        default: return "Contested";
    }
}

inline FactionSide TeamIdToSide(uint8 teamId)
{
    // TEAM_ALLIANCE = 0, TEAM_HORDE = 1
    if (teamId == 0)
        return FactionSide::Alliance;
    if (teamId == 1)
        return FactionSide::Horde;
    return FactionSide::Neutral;
}

inline TerritoryState SideToTerritory(FactionSide side)
{
    if (side == FactionSide::Alliance)
        return TerritoryState::AllianceHeld;
    if (side == FactionSide::Horde)
        return TerritoryState::HordeHeld;
    return TerritoryState::Contested;
}
} // namespace AiWorld

#endif
