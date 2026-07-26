/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiDirector.h"
#include "AiBotBridge.h"
#include "AiEconomyAnalyzer.h"
#include "AiEventScheduler.h"
#include "AiFactionScore.h"
#include "AiLlmClient.h"
#include "AiTaskQueue.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "AiZoneAnalyzer.h"
#include "Chat.h"
#include "GameTime.h"
#include "Log.h"
#include "StringFormat.h"
#include "prototype/HillsbradFront.h"
#include <algorithm>
#include <cmath>
#include <sstream>

AiDirector* AiDirector::instance()
{
    static AiDirector director;
    return &director;
}

void AiDirector::Initialize()
{
    if (_started)
        return;

    sAiWorldMemory->LoadFromDB();
    sAiFactionScore->Load();
    sAiTaskQueue->Start();
    _started = true;
    _worldTension = 0;
    LOG_INFO("module", ">> AiWorld Director initialized");
}

void AiDirector::Shutdown()
{
    if (!_started)
        return;
    sAiFactionScore->Flush();
    sAiTaskQueue->Stop();
    _started = false;
}

void AiDirector::Update(uint32 diff)
{
    if (!sAiWorldConfig().IsEnabled())
        return;

    if (!_started)
        Initialize();

    sAiZoneAnalyzer->Update(diff);
    sAiEventScheduler->Update(diff);
    sAiBotBridge->Update(diff);
    sAiEconomyAnalyzer->Update(diff);

    while (auto action = sAiTaskQueue->PopCompletedAction())
        ApplyAction(*action);

    _purgeTimer += diff;
    if (_purgeTimer >= 3600000)
    {
        _purgeTimer = 0;
        sAiWorldMemory->PurgeExpired();
    }

    _directorTimer += diff;
    uint32 intervalMs = sAiWorldConfig().GetDirectorIntervalSec() * IN_MILLISECONDS;
    if (_directorTimer < intervalMs)
        return;

    _directorTimer = 0;
    RunDirectorCycle();
}

void AiDirector::ForceAnalyze(uint32 zoneId)
{
    if (!_started)
        Initialize();

    if (zoneId)
    {
        AiWorld::ZoneSnapshot snap = sAiZoneAnalyzer->GetSnapshot(zoneId);
        AiWorld::DirectorAction action = DecideRules(snap);
        if (action.type != AiWorld::DirectorActionType::None)
            ApplyAction(action);
        return;
    }

    RunDirectorCycle();
}

void AiDirector::RunDirectorCycle()
{
    sAiZoneAnalyzer->SampleZones();

    for (uint32 zoneId : sAiWorldConfig().GetWatchedZones())
    {
        if (zoneId == AiWorld::ZONE_HILLSBRAD_FOOTHILLS && !sAiWorldConfig().IsHillsbradEnabled())
            continue;

        AiWorld::ZoneSnapshot snap = sAiZoneAnalyzer->GetSnapshot(zoneId);
        AdjustZoneTension(zoneId, snap, 0);

        AiWorld::DirectorAction action = DecideRules(snap);
        if (action.type == AiWorld::DirectorActionType::None && zoneId == AiWorld::ZONE_HILLSBRAD_FOOTHILLS)
            action = HillsbradFront::Decide(snap);

        if (action.type != AiWorld::DirectorActionType::None)
            ApplyAction(action);

        if (sAiWorldConfig().IsLlmEnabled() && sAiLlmClient->IsAvailable() && snap.tension >= 40)
        {
            AiTask task;
            task.type = AiWorld::AiTaskType::ProposeScenario;
            task.snapshot = snap;
            auto history = sAiWorldMemory->GetRecentHistory(zoneId, 5);
            std::string events = "[";
            for (size_t i = 0; i < history.size(); ++i)
            {
                if (i)
                    events += ",";
                events += Acore::StringFormat("{{\"type\":\"{}\",\"winner\":\"{}\"}}",
                    history[i].eventType, AiWorld::SideName(history[i].winner));
            }
            events += "]";
            task.recentEventsJson = events;
            sAiTaskQueue->Enqueue(std::move(task));
        }
    }

    sAiFactionScore->Flush();
}

AiWorld::DirectorAction AiDirector::DecideRules(AiWorld::ZoneSnapshot const& snap) const
{
    AiWorld::DirectorAction action;
    action.zoneId = snap.zoneId;

    uint32 now = uint32(GameTime::GetGameTime().count());
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _zoneActionCooldown.find(snap.zoneId);
        if (it != _zoneActionCooldown.end() && now < it->second)
            return action;
    }

    int32 alliancePower = int32(snap.alliancePlayers * 2 + snap.allianceBots + snap.pvpKillsAlliance);
    int32 hordePower = int32(snap.hordePlayers * 2 + snap.hordeBots + snap.pvpKillsHorde);
    int32 imbalance = alliancePower - hordePower;
    uint32 threshold = sAiWorldConfig().GetImbalanceThreshold();

    if (std::abs(imbalance) <= int32(threshold))
        return action;

    AiWorld::FactionSide weak = imbalance > 0 ? AiWorld::FactionSide::Horde : AiWorld::FactionSide::Alliance;
    AiWorld::FactionSide strong = imbalance > 0 ? AiWorld::FactionSide::Alliance : AiWorld::FactionSide::Horde;

    int32 projectedScore = snap.controlScore + (imbalance > 0 ? 10 : -10);
    uint32 flipThreshold = sAiWorldConfig().GetControlFlipThreshold();

    if (std::abs(projectedScore) >= int32(flipThreshold)
        && ((projectedScore > 0 && snap.control != AiWorld::TerritoryState::AllianceHeld)
            || (projectedScore < 0 && snap.control != AiWorld::TerritoryState::HordeHeld)))
    {
        action.type = AiWorld::DirectorActionType::ChangeTerritory;
        action.beneficiary = strong;
        action.priority = 10;
        action.reason = Acore::StringFormat("control_flip imbalance={}", imbalance);
        return action;
    }

    action.type = AiWorld::DirectorActionType::DeployBots;
    action.beneficiary = weak;
    action.botCount = std::min<uint32>(sAiWorldConfig().GetBotsMaxDeploy(),
        uint32(std::abs(imbalance)) + 2);
    action.priority = 7;
    action.reason = Acore::StringFormat("reinforce_weak imbalance={}", imbalance);

    if (snap.tension >= 50 && !sAiEventScheduler->IsOnCooldown(snap.zoneId, AiWorld::EVENT_HILLSBRAD_SKIRMISH))
    {
        // Prefer event when tension high; bots still applied as secondary by ApplyAction chain in Hillsbrad.
        action.type = AiWorld::DirectorActionType::StartEvent;
        action.eventTemplateId = AiWorld::EVENT_HILLSBRAD_SKIRMISH;
        action.beneficiary = weak;
        action.priority = 8;
        action.reason = Acore::StringFormat("skirmish_for_weak imbalance={}", imbalance);
    }

    return action;
}

void AiDirector::ApplyAction(AiWorld::DirectorAction const& action)
{
    if (action.type == AiWorld::DirectorActionType::None)
        return;

    uint32 now = uint32(GameTime::GetGameTime().count());
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _zoneActionCooldown[action.zoneId] = now + sAiWorldConfig().GetActionCooldownSec();
        _lastActions.push_back(action);
        if (_lastActions.size() > 20)
            _lastActions.erase(_lastActions.begin());
    }

    switch (action.type)
    {
        case AiWorld::DirectorActionType::ChangeTerritory:
            ApplyTerritoryChange(action.zoneId, action.beneficiary, action.reason);
            break;
        case AiWorld::DirectorActionType::DeployBots:
            sAiBotBridge->RequestDeploy(action.beneficiary, action.zoneId,
                action.botCount ? action.botCount : 4, action.reason);
            break;
        case AiWorld::DirectorActionType::StartEvent:
            sAiEventScheduler->StartEvent(action.eventTemplateId ? action.eventTemplateId
                : AiWorld::EVENT_HILLSBRAD_SKIRMISH, action.zoneId, action.beneficiary, action.reason);
            sAiBotBridge->RequestDefend(action.zoneId, action.beneficiary, action.reason);
            break;
        case AiWorld::DirectorActionType::AdjustTension:
            SetWorldTension(action.botCount); // reuse field as tension value for GM injects
            break;
        case AiWorld::DirectorActionType::Announce:
            if (sAiWorldConfig().AnnounceWorld())
                ChatHandler(nullptr).SendWorldText("{}", action.reason);
            sAiWorldMemory->RecordEvent(action.zoneId, "announce", 0, action.beneficiary,
                AiWorld::FactionSide::Neutral, action.reason);
            break;
        default:
            break;
    }

    LOG_INFO("module", "AiWorld Director applied action {} zone {} side {} ({})",
        uint32(action.type), action.zoneId, AiWorld::SideName(action.beneficiary), action.reason);
}

void AiDirector::ApplyTerritoryChange(uint32 zoneId, AiWorld::FactionSide beneficiary, std::string_view reason)
{
    AiWorld::TerritoryRecord record = sAiWorldMemory->GetTerritory(zoneId);
    record.zoneId = zoneId;
    record.controller = beneficiary;
    record.state = AiWorld::SideToTerritory(beneficiary);
    record.controlScore = (beneficiary == AiWorld::FactionSide::Alliance)
        ? int32(sAiWorldConfig().GetControlFlipThreshold())
        : (beneficiary == AiWorld::FactionSide::Horde)
            ? -int32(sAiWorldConfig().GetControlFlipThreshold())
            : 0;
    record.updatedAt = uint32(GameTime::GetGameTime().count());
    if (record.tension < 60)
        record.tension = 60;
    sAiWorldMemory->SetTerritory(record);

    sAiFactionScore->AddCapture(beneficiary);

    if (sAiWorldConfig().AnnounceWorld())
        ChatHandler(nullptr).SendWorldText(AiWorld::STRING_TERRITORY_FLIP, AiWorld::SideName(beneficiary), zoneId);

    sAiWorldMemory->RecordEvent(zoneId, "territory_flip", 0, beneficiary, beneficiary, std::string(reason));
}

void AiDirector::AdjustZoneTension(uint32 zoneId, AiWorld::ZoneSnapshot const& snap, int32 /*imbalance*/)
{
    AiWorld::TerritoryRecord record = sAiWorldMemory->GetTerritory(zoneId);
    int32 alliancePower = int32(snap.alliancePlayers * 2 + snap.allianceBots + snap.pvpKillsAlliance);
    int32 hordePower = int32(snap.hordePlayers * 2 + snap.hordeBots + snap.pvpKillsHorde);
    int32 imbalance = alliancePower - hordePower;

    int32 tensionDelta = 0;
    if (std::abs(imbalance) > int32(sAiWorldConfig().GetImbalanceThreshold()))
        tensionDelta = 5;
    else if (snap.alliancePlayers + snap.hordePlayers == 0)
        tensionDelta = -2;
    else
        tensionDelta = -1;

    int32 next = int32(record.tension) + tensionDelta;
    if (next < 0)
        next = 0;
    if (next > 100)
        next = 100;
    record.tension = uint32(next);
    record.controlScore += (imbalance > 0 ? 1 : (imbalance < 0 ? -1 : 0));
    if (record.controlScore > 1000)
        record.controlScore = 1000;
    if (record.controlScore < -1000)
        record.controlScore = -1000;
    record.updatedAt = uint32(GameTime::GetGameTime().count());
    sAiWorldMemory->SetTerritory(record);

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _worldTension = (_worldTension * 3 + record.tension) / 4;
    }
}

void AiDirector::SetWorldTension(uint32 tension)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _worldTension = std::min<uint32>(tension, 100);
}

uint32 AiDirector::GetWorldTension() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _worldTension;
}

void AiDirector::SimulateImbalance(uint32 zoneId, AiWorld::FactionSide strongSide, uint32 magnitude)
{
    if (strongSide == AiWorld::FactionSide::Alliance)
    {
        sAiZoneAnalyzer->InjectSimulatedBots(zoneId, AiWorld::FactionSide::Alliance, magnitude);
        sAiZoneAnalyzer->InjectSimulatedBots(zoneId, AiWorld::FactionSide::Horde, 0);
    }
    else if (strongSide == AiWorld::FactionSide::Horde)
    {
        sAiZoneAnalyzer->InjectSimulatedBots(zoneId, AiWorld::FactionSide::Horde, magnitude);
        sAiZoneAnalyzer->InjectSimulatedBots(zoneId, AiWorld::FactionSide::Alliance, 0);
    }
    else
        sAiZoneAnalyzer->ClearSimulation(zoneId);

    // Boost control score toward strong side for faster demo flips.
    AiWorld::TerritoryRecord record = sAiWorldMemory->GetTerritory(zoneId);
    if (strongSide == AiWorld::FactionSide::Alliance)
        record.controlScore += int32(magnitude) * 5;
    else if (strongSide == AiWorld::FactionSide::Horde)
        record.controlScore -= int32(magnitude) * 5;
    record.tension = std::min<uint32>(100, record.tension + 20);
    record.updatedAt = uint32(GameTime::GetGameTime().count());
    sAiWorldMemory->SetTerritory(record);
}

std::string AiDirector::BuildStatusText() const
{
    std::ostringstream ss;
    ss << "AiWorld enabled=" << (sAiWorldConfig().IsEnabled() ? 1 : 0)
       << " tension=" << GetWorldTension()
       << " llm=" << (sAiWorldConfig().IsLlmEnabled() && sAiLlmClient->IsAvailable() ? 1 : 0)
       << " playerbots=" << (sAiBotBridge->IsPlayerBotsAvailable() ? 1 : 0)
       << " events=" << sAiEventScheduler->GetActiveCount()
       << " botOrders=" << sAiBotBridge->GetPendingCount() << "\n";

    for (uint32 zoneId : sAiWorldConfig().GetWatchedZones())
    {
        AiWorld::ZoneSnapshot snap = sAiZoneAnalyzer->GetSnapshot(zoneId);
        ss << "Zone " << zoneId
           << " A=" << snap.alliancePlayers << "+" << snap.allianceBots
           << " H=" << snap.hordePlayers << "+" << snap.hordeBots
           << " killsA=" << snap.pvpKillsAlliance << " killsH=" << snap.pvpKillsHorde
           << " control=" << AiWorld::TerritoryName(snap.control)
           << " score=" << snap.controlScore
           << " zoneTension=" << snap.tension << "\n";
    }

    auto actions = GetLastActions();
    ss << "Last actions: " << actions.size() << "\n";
    for (auto const& a : actions)
        ss << "  type=" << uint32(a.type) << " zone=" << a.zoneId
           << " side=" << AiWorld::SideName(a.beneficiary) << " (" << a.reason << ")\n";

    return ss.str();
}

std::vector<AiWorld::DirectorAction> AiDirector::GetLastActions() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _lastActions;
}
