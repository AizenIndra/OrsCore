/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_DIRECTOR_H
#define MOD_AIWORLD_DIRECTOR_H

#include "AiWorldCommon.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AiDirector
{
public:
    static AiDirector* instance();

    void Initialize();
    void Shutdown();
    void Update(uint32 diff);

    void ForceAnalyze(uint32 zoneId = 0);
    void ApplyAction(AiWorld::DirectorAction const& action);
    void SetWorldTension(uint32 tension);
    [[nodiscard]] uint32 GetWorldTension() const;

    void SimulateImbalance(uint32 zoneId, AiWorld::FactionSide strongSide, uint32 magnitude);

    [[nodiscard]] std::string BuildStatusText() const;
    [[nodiscard]] std::vector<AiWorld::DirectorAction> GetLastActions() const;

private:
    AiDirector() = default;

    void RunDirectorCycle();
    AiWorld::DirectorAction DecideRules(AiWorld::ZoneSnapshot const& snap) const;
    void ApplyTerritoryChange(uint32 zoneId, AiWorld::FactionSide beneficiary, std::string_view reason);
    void AdjustZoneTension(uint32 zoneId, AiWorld::ZoneSnapshot const& snap, int32 imbalance);

    mutable std::mutex _mutex;
    uint32 _directorTimer = 0;
    uint32 _worldTension = 0;
    uint32 _purgeTimer = 0;
    std::unordered_map<uint32, uint32> _zoneActionCooldown;
    std::vector<AiWorld::DirectorAction> _lastActions;
    bool _started = false;
};

#define sAiDirector AiDirector::instance()

#endif
