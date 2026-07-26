/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiEconomyAnalyzer.h"
#include "AiWorldConfig.h"
#include "GameTime.h"

AiEconomyAnalyzer* AiEconomyAnalyzer::instance()
{
    static AiEconomyAnalyzer analyzer;
    return &analyzer;
}

void AiEconomyAnalyzer::Update(uint32 diff)
{
    if (!sAiWorldConfig().IsEnabled())
        return;

    // Stub: reserve periodic sampling slot for future AH/vendor hooks.
    _timer += diff;
    if (_timer < 600000)
        return;
    _timer = 0;
}

AiEconomyAnalyzer::ZoneEconomySnapshot AiEconomyAnalyzer::GetSnapshot(uint32 zoneId) const
{
    ZoneEconomySnapshot snap;
    snap.zoneId = zoneId;
    snap.sampleUnix = uint32(GameTime::GetGameTime().count());
    return snap;
}
