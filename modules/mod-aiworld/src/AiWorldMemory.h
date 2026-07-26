/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_MEMORY_H
#define MOD_AIWORLD_MEMORY_H

#include "AiWorldCommon.h"
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class AiWorldMemory
{
public:
    static AiWorldMemory* instance();

    void LoadFromDB();
    void PurgeExpired();

    [[nodiscard]] AiWorld::TerritoryRecord GetTerritory(uint32 zoneId) const;
    void SetTerritory(AiWorld::TerritoryRecord const& record);

    void RecordEvent(uint32 zoneId, std::string_view eventType, uint32 templateId,
        AiWorld::FactionSide aggressor, AiWorld::FactionSide winner, std::string_view payloadJson);
    [[nodiscard]] std::vector<AiWorld::EventHistoryRecord> GetRecentHistory(uint32 zoneId, uint32 limit) const;

    void RecordPlayerAction(ObjectGuid::LowType guid, uint32 zoneId,
        std::string_view actionType, AiWorld::FactionSide side);
    void RecordNpcChange(uint32 zoneId, uint32 entry,
        std::string_view changeType, std::string_view detailsJson);

    void SetFactionScore(AiWorld::FactionScoreRecord const& record);
    [[nodiscard]] AiWorld::FactionScoreRecord GetFactionScore(AiWorld::FactionSide side) const;

    [[nodiscard]] std::optional<std::string> GetLlmCache(std::string const& cacheKey) const;
    void SetLlmCache(std::string const& cacheKey, std::string const& responseJson, uint32 ttlSec);

private:
    AiWorldMemory() = default;

    mutable std::mutex _mutex;
    std::unordered_map<uint32, AiWorld::TerritoryRecord> _territories;
    std::unordered_map<uint32, std::deque<AiWorld::EventHistoryRecord>> _historyByZone;
    std::unordered_map<uint8, AiWorld::FactionScoreRecord> _factionScores;
};

#define sAiWorldMemory AiWorldMemory::instance()

#endif
