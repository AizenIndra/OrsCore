/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_ZONE_ANALYZER_H
#define MOD_AIWORLD_ZONE_ANALYZER_H

#include "AiWorldCommon.h"
#include <mutex>
#include <unordered_map>

class AiZoneAnalyzer
{
public:
    static AiZoneAnalyzer* instance();

    void Update(uint32 diff);
    void SampleZones();

    void OnPlayerEnterZone(ObjectGuid guid, uint32 zoneId, AiWorld::FactionSide side, bool isBot);
    void OnPlayerLeaveZone(ObjectGuid guid, uint32 zoneId);
    void OnPvpKill(uint32 zoneId, AiWorld::FactionSide killerSide);

    void InjectSimulatedBots(uint32 zoneId, AiWorld::FactionSide side, uint32 count);
    void ClearSimulation(uint32 zoneId);

    [[nodiscard]] AiWorld::ZoneSnapshot GetSnapshot(uint32 zoneId) const;

private:
    AiZoneAnalyzer() = default;

    struct ZoneLive
    {
        uint32 alliancePlayers = 0;
        uint32 hordePlayers = 0;
        uint32 allianceBots = 0;
        uint32 hordeBots = 0;
        uint32 simAllianceBots = 0;
        uint32 simHordeBots = 0;
        uint32 pvpKillsAlliance = 0;
        uint32 pvpKillsHorde = 0;
        uint32 windowResetAt = 0;
    };

    mutable std::mutex _mutex;
    std::unordered_map<uint32, ZoneLive> _zones;
    std::unordered_map<ObjectGuid, std::pair<uint32, AiWorld::FactionSide>> _playerZones;
    uint32 _sampleTimer = 0;
};

#define sAiZoneAnalyzer AiZoneAnalyzer::instance()

#endif
