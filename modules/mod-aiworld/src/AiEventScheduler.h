/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_EVENT_SCHEDULER_H
#define MOD_AIWORLD_EVENT_SCHEDULER_H

#include "AiWorldCommon.h"
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class AiEventScheduler
{
public:
    static AiEventScheduler* instance();

    void Update(uint32 diff);
    bool StartEvent(uint32 templateId, uint32 zoneId, AiWorld::FactionSide aggressor, std::string_view reason);
    void CancelAll();

    [[nodiscard]] uint32 GetActiveCount() const;
    [[nodiscard]] std::vector<AiWorld::WorldEvent> GetActiveEvents() const;
    [[nodiscard]] bool IsOnCooldown(uint32 zoneId, uint32 templateId) const;

private:
    AiEventScheduler() = default;

    void EndEvent(AiWorld::WorldEvent& event);
    void SpawnSkirmish(AiWorld::WorldEvent const& event);
    void DespawnSkirmish(AiWorld::WorldEvent const& event);

    mutable std::mutex _mutex;
    std::vector<AiWorld::WorldEvent> _events;
    std::unordered_map<uint64, std::vector<ObjectGuid>> _spawnedCreatures;
    std::unordered_map<uint64, uint32> _cooldowns; // key = zone<<32|template
    uint64 _nextId = 1;
};

#define sAiEventScheduler AiEventScheduler::instance()

#endif
