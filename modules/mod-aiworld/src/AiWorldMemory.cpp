/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiWorldMemory.h"
#include "AiWorldConfig.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "GameTime.h"
#include "Log.h"
#include "QueryResult.h"
#include <algorithm>

AiWorldMemory* AiWorldMemory::instance()
{
    static AiWorldMemory mgr;
    return &mgr;
}

void AiWorldMemory::LoadFromDB()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _territories.clear();
    _historyByZone.clear();
    _factionScores.clear();

    if (QueryResult result = CharacterDatabase.Query(
        "SELECT zone_id, state, controller, control_score, tension, updated_at FROM aiworld_territory"))
    {
        do
        {
            Field* fields = result->Fetch();
            AiWorld::TerritoryRecord record;
            record.zoneId = fields[0].Get<uint32>();
            record.state = static_cast<AiWorld::TerritoryState>(fields[1].Get<uint8>());
            record.controller = static_cast<AiWorld::FactionSide>(fields[2].Get<uint8>());
            record.controlScore = fields[3].Get<int32>();
            record.tension = fields[4].Get<uint32>();
            record.updatedAt = fields[5].Get<uint32>();
            _territories[record.zoneId] = record;
        } while (result->NextRow());
    }

    uint32 cacheLimit = sAiWorldConfig().GetHistoryCacheLimit();
    if (QueryResult result = CharacterDatabase.Query(
        "SELECT id, zone_id, event_type, template_id, aggressor, winner, payload_json, created_at "
        "FROM aiworld_event_history ORDER BY id DESC LIMIT 500"))
    {
        do
        {
            Field* fields = result->Fetch();
            AiWorld::EventHistoryRecord record;
            record.id = fields[0].Get<uint64>();
            record.zoneId = fields[1].Get<uint32>();
            record.eventType = fields[2].Get<std::string>();
            record.templateId = fields[3].Get<uint32>();
            record.aggressor = static_cast<AiWorld::FactionSide>(fields[4].Get<uint8>());
            record.winner = static_cast<AiWorld::FactionSide>(fields[5].Get<uint8>());
            record.payloadJson = fields[6].Get<std::string>();
            record.createdAt = fields[7].Get<uint32>();

            auto& deque = _historyByZone[record.zoneId];
            if (deque.size() < cacheLimit)
                deque.push_front(std::move(record));
        } while (result->NextRow());
    }

    if (QueryResult result = CharacterDatabase.Query(
        "SELECT side, window_hour, window_day, total, updated_at FROM aiworld_faction_score"))
    {
        do
        {
            Field* fields = result->Fetch();
            AiWorld::FactionScoreRecord record;
            record.side = static_cast<AiWorld::FactionSide>(fields[0].Get<uint8>());
            record.windowHour = fields[1].Get<int32>();
            record.windowDay = fields[2].Get<int32>();
            record.total = fields[3].Get<int64>();
            record.updatedAt = fields[4].Get<uint32>();
            _factionScores[uint8(record.side)] = record;
        } while (result->NextRow());
    }

    LOG_INFO("module", ">> AiWorld: loaded {} territories, {} faction scores",
        _territories.size(), _factionScores.size());
}

void AiWorldMemory::PurgeExpired()
{
    uint32 days = sAiWorldConfig().GetMemoryRetentionDays();
    if (!days)
        return;

    uint32 cutoff = uint32(GameTime::GetGameTime().count()) - days * 86400;
    CharacterDatabase.Execute("DELETE FROM aiworld_event_history WHERE created_at < {}", cutoff);
    CharacterDatabase.Execute("DELETE FROM aiworld_player_action WHERE created_at < {}", cutoff);
    CharacterDatabase.Execute("DELETE FROM aiworld_npc_change WHERE created_at < {}", cutoff);
    CharacterDatabase.Execute(
        "DELETE FROM aiworld_llm_cache WHERE expires_at < {}",
        uint32(GameTime::GetGameTime().count()));
}

AiWorld::TerritoryRecord AiWorldMemory::GetTerritory(uint32 zoneId) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _territories.find(zoneId);
    if (it != _territories.end())
        return it->second;

    AiWorld::TerritoryRecord record;
    record.zoneId = zoneId;
    record.state = AiWorld::TerritoryState::Contested;
    record.controller = AiWorld::FactionSide::Neutral;
    return record;
}

void AiWorldMemory::SetTerritory(AiWorld::TerritoryRecord const& record)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _territories[record.zoneId] = record;
    }

    CharacterDatabase.Execute(
        "REPLACE INTO aiworld_territory (zone_id, state, controller, control_score, tension, updated_at) "
        "VALUES ({}, {}, {}, {}, {}, {})",
        record.zoneId, uint32(record.state), uint32(record.controller),
        record.controlScore, record.tension, record.updatedAt);
}

void AiWorldMemory::RecordEvent(uint32 zoneId, std::string_view eventType, uint32 templateId,
    AiWorld::FactionSide aggressor, AiWorld::FactionSide winner, std::string_view payloadJson)
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    std::string type(eventType);
    std::string payload(payloadJson);
    CharacterDatabase.EscapeString(type);
    CharacterDatabase.EscapeString(payload);

    CharacterDatabase.Execute(
        "INSERT INTO aiworld_event_history "
        "(zone_id, event_type, template_id, aggressor, winner, payload_json, created_at) "
        "VALUES ({}, '{}', {}, {}, {}, '{}', {})",
        zoneId, type, templateId, uint32(aggressor), uint32(winner), payload, now);

    AiWorld::EventHistoryRecord record;
    record.zoneId = zoneId;
    record.eventType = std::string(eventType);
    record.templateId = templateId;
    record.aggressor = aggressor;
    record.winner = winner;
    record.payloadJson = std::string(payloadJson);
    record.createdAt = now;

    std::lock_guard<std::mutex> lock(_mutex);
    auto& deque = _historyByZone[zoneId];
    deque.push_back(std::move(record));
    uint32 limit = sAiWorldConfig().GetHistoryCacheLimit();
    while (deque.size() > limit)
        deque.pop_front();
}

std::vector<AiWorld::EventHistoryRecord> AiWorldMemory::GetRecentHistory(uint32 zoneId, uint32 limit) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<AiWorld::EventHistoryRecord> out;
    auto it = _historyByZone.find(zoneId);
    if (it == _historyByZone.end())
        return out;

    auto const& deque = it->second;
    uint32 count = 0;
    for (auto rit = deque.rbegin(); rit != deque.rend() && count < limit; ++rit, ++count)
        out.push_back(*rit);
    return out;
}

void AiWorldMemory::RecordPlayerAction(ObjectGuid::LowType guid, uint32 zoneId,
    std::string_view actionType, AiWorld::FactionSide side)
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    std::string type(actionType);
    CharacterDatabase.EscapeString(type);
    CharacterDatabase.Execute(
        "INSERT INTO aiworld_player_action (guid, zone_id, action_type, side, created_at) "
        "VALUES ({}, {}, '{}', {}, {})",
        guid, zoneId, type, uint32(side), now);
}

void AiWorldMemory::RecordNpcChange(uint32 zoneId, uint32 entry,
    std::string_view changeType, std::string_view detailsJson)
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    std::string type(changeType);
    std::string details(detailsJson);
    CharacterDatabase.EscapeString(type);
    CharacterDatabase.EscapeString(details);
    CharacterDatabase.Execute(
        "INSERT INTO aiworld_npc_change (zone_id, entry, change_type, details_json, created_at) "
        "VALUES ({}, {}, '{}', '{}', {})",
        zoneId, entry, type, details, now);
}

void AiWorldMemory::SetFactionScore(AiWorld::FactionScoreRecord const& record)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _factionScores[uint8(record.side)] = record;
    }

    CharacterDatabase.Execute(
        "REPLACE INTO aiworld_faction_score (side, window_hour, window_day, total, updated_at) "
        "VALUES ({}, {}, {}, {}, {})",
        uint32(record.side), record.windowHour, record.windowDay, record.total, record.updatedAt);
}

AiWorld::FactionScoreRecord AiWorldMemory::GetFactionScore(AiWorld::FactionSide side) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _factionScores.find(uint8(side));
    if (it != _factionScores.end())
        return it->second;

    AiWorld::FactionScoreRecord record;
    record.side = side;
    return record;
}

std::optional<std::string> AiWorldMemory::GetLlmCache(std::string const& cacheKey) const
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    std::string key = cacheKey;
    CharacterDatabase.EscapeString(key);
    if (QueryResult result = CharacterDatabase.Query(
        "SELECT response_json FROM aiworld_llm_cache WHERE cache_key = '{}' AND expires_at > {}", key, now))
        return result->Fetch()[0].Get<std::string>();
    return std::nullopt;
}

void AiWorldMemory::SetLlmCache(std::string const& cacheKey, std::string const& responseJson, uint32 ttlSec)
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    std::string key = cacheKey;
    std::string response = responseJson;
    CharacterDatabase.EscapeString(key);
    CharacterDatabase.EscapeString(response);
    CharacterDatabase.Execute(
        "REPLACE INTO aiworld_llm_cache (cache_key, response_json, created_at, expires_at) "
        "VALUES ('{}', '{}', {}, {})",
        key, response, now, now + ttlSec);
}
