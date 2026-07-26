/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiEventScheduler.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "Chat.h"
#include "Creature.h"
#include "GameTime.h"
#include "Log.h"
#include "MapMgr.h"
#include "Position.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include <algorithm>

namespace
{
uint64 CooldownKey(uint32 zoneId, uint32 templateId)
{
    return (uint64(zoneId) << 32) | templateId;
}
}

AiEventScheduler* AiEventScheduler::instance()
{
    static AiEventScheduler scheduler;
    return &scheduler;
}

void AiEventScheduler::Update(uint32 /*diff*/)
{
    if (!sAiWorldConfig().IsEnabled())
        return;

    uint32 now = uint32(GameTime::GetGameTime().count());
    std::vector<AiWorld::WorldEvent> toEnd;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& event : _events)
        {
            if (event.state == AiWorld::WorldEventState::Active && event.endUnix && now >= event.endUnix)
                toEnd.push_back(event);
        }
    }

    for (auto& event : toEnd)
        EndEvent(event);
}

bool AiEventScheduler::StartEvent(uint32 templateId, uint32 zoneId,
    AiWorld::FactionSide aggressor, std::string_view reason)
{
    if (!templateId)
        return false;

    if (GetActiveCount() >= sAiWorldConfig().GetEventsMaxConcurrent())
    {
        LOG_DEBUG("module", "AiWorld: event rejected, max concurrent reached");
        return false;
    }

    if (IsOnCooldown(zoneId, templateId))
    {
        LOG_DEBUG("module", "AiWorld: event rejected, cooldown zone {} template {}", zoneId, templateId);
        return false;
    }

    uint32 now = uint32(GameTime::GetGameTime().count());
    AiWorld::WorldEvent event;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        event.id = _nextId++;
        event.templateId = templateId;
        event.zoneId = zoneId;
        event.aggressor = aggressor;
        event.startUnix = now;
        event.endUnix = now + sAiWorldConfig().GetEventDurationSec();
        event.state = AiWorld::WorldEventState::Active;
        event.reason = std::string(reason);
        _events.push_back(event);
        _cooldowns[CooldownKey(zoneId, templateId)] = now + sAiWorldConfig().GetActionCooldownSec();
    }

    std::string aggressorName = AiWorld::SideName(aggressor);
    if (sAiWorldConfig().AnnounceWorld())
        ChatHandler(nullptr).SendWorldText(AiWorld::STRING_EVENT_START, aggressorName, zoneId);

    if (templateId == AiWorld::EVENT_HILLSBRAD_SKIRMISH)
        SpawnSkirmish(event);

    sAiWorldMemory->RecordEvent(zoneId, "event_start", templateId, aggressor, AiWorld::FactionSide::Neutral,
        std::string(reason));

    LOG_INFO("module", "AiWorld: started event {} in zone {} ({})", templateId, zoneId, reason);
    return true;
}

void AiEventScheduler::EndEvent(AiWorld::WorldEvent& event)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& e : _events)
        {
            if (e.id == event.id)
            {
                e.state = AiWorld::WorldEventState::Completed;
                event = e;
                break;
            }
        }
    }

    if (event.templateId == AiWorld::EVENT_HILLSBRAD_SKIRMISH)
        DespawnSkirmish(event);

    if (sAiWorldConfig().AnnounceWorld())
        ChatHandler(nullptr).SendWorldText(AiWorld::STRING_EVENT_END, event.zoneId);

    sAiWorldMemory->RecordEvent(event.zoneId, "event_end", event.templateId, event.aggressor,
        AiWorld::FactionSide::Neutral, event.reason);

    std::lock_guard<std::mutex> lock(_mutex);
    _events.erase(std::remove_if(_events.begin(), _events.end(),
        [&](AiWorld::WorldEvent const& e) { return e.id == event.id; }), _events.end());
    _spawnedCreatures.erase(event.id);
}

void AiEventScheduler::CancelAll()
{
    std::vector<AiWorld::WorldEvent> copy;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        copy = _events;
    }
    for (auto& event : copy)
    {
        event.endUnix = uint32(GameTime::GetGameTime().count());
        EndEvent(event);
    }
}

uint32 AiEventScheduler::GetActiveCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    uint32 count = 0;
    for (auto const& e : _events)
        if (e.state == AiWorld::WorldEventState::Active)
            ++count;
    return count;
}

std::vector<AiWorld::WorldEvent> AiEventScheduler::GetActiveEvents() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<AiWorld::WorldEvent> out;
    for (auto const& e : _events)
        if (e.state == AiWorld::WorldEventState::Active)
            out.push_back(e);
    return out;
}

bool AiEventScheduler::IsOnCooldown(uint32 zoneId, uint32 templateId) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _cooldowns.find(CooldownKey(zoneId, templateId));
    if (it == _cooldowns.end())
        return false;
    return uint32(GameTime::GetGameTime().count()) < it->second;
}

void AiEventScheduler::SpawnSkirmish(AiWorld::WorldEvent const& event)
{
    uint32 entry = sAiWorldConfig().GetSkirmishCreatureEntry();
    uint32 count = sAiWorldConfig().GetSkirmishCreatureCount();
    if (!entry || !count)
        return;

    Map* map = sMapMgr->FindMap(sAiWorldConfig().GetHillsbradMapId(), 0);
    if (!map)
    {
        LOG_WARN("module", "AiWorld: Hillsbrad map not loaded, skip skirmish spawns");
        return;
    }

    float x = sAiWorldConfig().GetHillsbradX();
    float y = sAiWorldConfig().GetHillsbradY();
    float z = sAiWorldConfig().GetHillsbradZ();
    float o = sAiWorldConfig().GetHillsbradO();

    uint32 durationMs = sAiWorldConfig().GetEventDurationSec() * IN_MILLISECONDS;
    std::vector<ObjectGuid> guids;
    for (uint32 i = 0; i < count; ++i)
    {
        float ox = x + float(i) * 2.0f;
        if (TempSummon* summon = map->SummonCreature(entry, Position(ox, y, z, o), nullptr, durationMs))
        {
            guids.push_back(summon->GetGUID());
            sAiWorldMemory->RecordNpcChange(event.zoneId, entry, "spawn",
                Acore::StringFormat("{{\"eventId\":{}}}", event.id));
        }
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _spawnedCreatures[event.id] = std::move(guids);
}

void AiEventScheduler::DespawnSkirmish(AiWorld::WorldEvent const& event)
{
    std::vector<ObjectGuid> guids;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _spawnedCreatures.find(event.id);
        if (it == _spawnedCreatures.end())
            return;
        guids = it->second;
    }

    Map* map = sMapMgr->FindMap(sAiWorldConfig().GetHillsbradMapId(), 0);
    if (!map)
        return;

    for (ObjectGuid guid : guids)
        if (Creature* creature = map->GetCreature(guid))
            creature->DespawnOrUnsummon();
}
