/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiZoneAnalyzer.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "GameTime.h"
#include "Player.h"
#include "WorldSession.h"
#include "WorldSessionMgr.h"

#ifdef MOD_PLAYERBOTS
#include "RandomPlayerbotMgr.h"
#endif

AiZoneAnalyzer* AiZoneAnalyzer::instance()
{
    static AiZoneAnalyzer analyzer;
    return &analyzer;
}

void AiZoneAnalyzer::Update(uint32 diff)
{
    if (!sAiWorldConfig().IsEnabled())
        return;

    _sampleTimer += diff;
    uint32 sampleMs = sAiWorldConfig().GetMetricsSampleSec() * IN_MILLISECONDS;
    if (_sampleTimer < sampleMs)
        return;

    _sampleTimer = 0;
    SampleZones();
}

void AiZoneAnalyzer::SampleZones()
{
    std::unordered_map<uint32, ZoneLive> next;
    std::unordered_map<ObjectGuid, std::pair<uint32, AiWorld::FactionSide>> nextPlayers;
    uint32 now = uint32(GameTime::GetGameTime().count());

    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto const& [zoneId, live] : _zones)
        {
            ZoneLive copy;
            copy.simAllianceBots = live.simAllianceBots;
            copy.simHordeBots = live.simHordeBots;
            copy.pvpKillsAlliance = live.pvpKillsAlliance;
            copy.pvpKillsHorde = live.pvpKillsHorde;
            copy.windowResetAt = live.windowResetAt ? live.windowResetAt : now + 3600;
            if (now >= copy.windowResetAt)
            {
                copy.pvpKillsAlliance = 0;
                copy.pvpKillsHorde = 0;
                copy.windowResetAt = now + 3600;
            }
            next[zoneId] = copy;
        }
    }

    sWorldSessionMgr->DoForAllOnlinePlayers([&](Player* player)
    {
        if (!player)
            return;

        uint32 zoneId = player->GetZoneId();
        if (!sAiWorldConfig().IsWatchedZone(zoneId))
            return;

        AiWorld::FactionSide side = AiWorld::TeamIdToSide(player->GetTeamId());
        bool isBot = false;
#ifdef MOD_PLAYERBOTS
        isBot = sRandomPlayerbotMgr.IsRandomBot(player);
#endif

        ZoneLive& live = next[zoneId];
        if (side == AiWorld::FactionSide::Alliance)
        {
            if (isBot)
                ++live.allianceBots;
            else
                ++live.alliancePlayers;
        }
        else if (side == AiWorld::FactionSide::Horde)
        {
            if (isBot)
                ++live.hordeBots;
            else
                ++live.hordePlayers;
        }

        nextPlayers[player->GetGUID()] = { zoneId, side };
    });

    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [zoneId, live] : next)
        {
            auto old = _zones.find(zoneId);
            if (old != _zones.end())
            {
                live.simAllianceBots = old->second.simAllianceBots;
                live.simHordeBots = old->second.simHordeBots;
            }
            live.allianceBots += live.simAllianceBots;
            live.hordeBots += live.simHordeBots;
        }
        _zones = std::move(next);
        _playerZones = std::move(nextPlayers);
    }
}

void AiZoneAnalyzer::OnPlayerEnterZone(ObjectGuid guid, uint32 zoneId, AiWorld::FactionSide side, bool /*isBot*/)
{
    if (!sAiWorldConfig().IsWatchedZone(zoneId))
        return;

    std::lock_guard<std::mutex> lock(_mutex);
    _playerZones[guid] = { zoneId, side };
}

void AiZoneAnalyzer::OnPlayerLeaveZone(ObjectGuid guid, uint32 /*zoneId*/)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _playerZones.erase(guid);
}

void AiZoneAnalyzer::OnPvpKill(uint32 zoneId, AiWorld::FactionSide killerSide)
{
    if (!sAiWorldConfig().IsWatchedZone(zoneId))
        return;

    std::lock_guard<std::mutex> lock(_mutex);
    ZoneLive& live = _zones[zoneId];
    uint32 now = uint32(GameTime::GetGameTime().count());
    if (!live.windowResetAt)
        live.windowResetAt = now + 3600;
    if (now >= live.windowResetAt)
    {
        live.pvpKillsAlliance = 0;
        live.pvpKillsHorde = 0;
        live.windowResetAt = now + 3600;
    }

    if (killerSide == AiWorld::FactionSide::Alliance)
        ++live.pvpKillsAlliance;
    else if (killerSide == AiWorld::FactionSide::Horde)
        ++live.pvpKillsHorde;
}

void AiZoneAnalyzer::InjectSimulatedBots(uint32 zoneId, AiWorld::FactionSide side, uint32 count)
{
    std::lock_guard<std::mutex> lock(_mutex);
    ZoneLive& live = _zones[zoneId];
    if (side == AiWorld::FactionSide::Alliance)
        live.simAllianceBots = count;
    else if (side == AiWorld::FactionSide::Horde)
        live.simHordeBots = count;
}

void AiZoneAnalyzer::ClearSimulation(uint32 zoneId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _zones.find(zoneId);
    if (it == _zones.end())
        return;
    it->second.simAllianceBots = 0;
    it->second.simHordeBots = 0;
}

AiWorld::ZoneSnapshot AiZoneAnalyzer::GetSnapshot(uint32 zoneId) const
{
    AiWorld::ZoneSnapshot snap;
    snap.zoneId = zoneId;
    snap.unixTime = uint32(GameTime::GetGameTime().count());

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _zones.find(zoneId);
        if (it != _zones.end())
        {
            snap.alliancePlayers = it->second.alliancePlayers;
            snap.hordePlayers = it->second.hordePlayers;
            snap.allianceBots = it->second.allianceBots;
            snap.hordeBots = it->second.hordeBots;
            snap.pvpKillsAlliance = it->second.pvpKillsAlliance;
            snap.pvpKillsHorde = it->second.pvpKillsHorde;
        }
    }

    AiWorld::TerritoryRecord territory = sAiWorldMemory->GetTerritory(zoneId);
    snap.control = territory.state;
    snap.tension = territory.tension;
    snap.controlScore = territory.controlScore;
    return snap;
}
