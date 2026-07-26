/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "HillsbradFront.h"
#include "AiWorldConfig.h"
#include "AiEventScheduler.h"
#include "StringFormat.h"
#include <cmath>

namespace HillsbradFront
{
AiWorld::DirectorAction Decide(AiWorld::ZoneSnapshot const& snap)
{
    AiWorld::DirectorAction action;
    action.zoneId = AiWorld::ZONE_HILLSBRAD_FOOTHILLS;

    if (!sAiWorldConfig().IsHillsbradEnabled())
        return action;

    if (snap.zoneId != AiWorld::ZONE_HILLSBRAD_FOOTHILLS)
        return action;

    int32 alliancePower = int32(snap.alliancePlayers * 2 + snap.allianceBots + snap.pvpKillsAlliance);
    int32 hordePower = int32(snap.hordePlayers * 2 + snap.hordeBots + snap.pvpKillsHorde);
    int32 imbalance = alliancePower - hordePower;
    uint32 threshold = sAiWorldConfig().GetImbalanceThreshold();

    uint32 flipThreshold = sAiWorldConfig().GetControlFlipThreshold();
    if (std::abs(imbalance) <= int32(threshold)
        && std::abs(snap.controlScore) < int32(flipThreshold))
        return action;

    AiWorld::FactionSide strong = imbalance >= 0
        ? AiWorld::FactionSide::Alliance : AiWorld::FactionSide::Horde;
    AiWorld::FactionSide weak = imbalance >= 0
        ? AiWorld::FactionSide::Horde : AiWorld::FactionSide::Alliance;

    if (std::abs(snap.controlScore) >= int32(flipThreshold))
    {
        AiWorld::FactionSide holder = snap.controlScore > 0
            ? AiWorld::FactionSide::Alliance : AiWorld::FactionSide::Horde;
        if (AiWorld::SideToTerritory(holder) != snap.control)
        {
            action.type = AiWorld::DirectorActionType::ChangeTerritory;
            action.beneficiary = holder;
            action.priority = 10;
            action.reason = "hillsbrad_control_threshold";
            return action;
        }
    }

    if (!sAiEventScheduler->IsOnCooldown(snap.zoneId, AiWorld::EVENT_HILLSBRAD_SKIRMISH) && snap.tension >= 30)
    {
        action.type = AiWorld::DirectorActionType::StartEvent;
        action.eventTemplateId = AiWorld::EVENT_HILLSBRAD_SKIRMISH;
        action.beneficiary = weak;
        action.botCount = 4;
        action.priority = 8;
        action.reason = Acore::StringFormat("hillsbrad_skirmish strong={}", AiWorld::SideName(strong));
        return action;
    }

    action.type = AiWorld::DirectorActionType::DeployBots;
    action.beneficiary = weak;
    action.botCount = 4;
    action.priority = 6;
    action.reason = "hillsbrad_reinforce";
    return action;
}
} // namespace HillsbradFront
