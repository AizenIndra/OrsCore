/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_ECONOMY_ANALYZER_H
#define MOD_AIWORLD_ECONOMY_ANALYZER_H

#include "AiWorldCommon.h"
#include <cstdint>

// Phase-6 stub: interface ready for AH / vendor metrics.
class AiEconomyAnalyzer
{
public:
    static AiEconomyAnalyzer* instance();

    struct ZoneEconomySnapshot
    {
        uint32 zoneId = 0;
        uint64 auctionVolumeCopper = 0;
        uint64 vendorSpendCopper = 0;
        uint32 sampleUnix = 0;
    };

    void Update(uint32 diff);
    [[nodiscard]] ZoneEconomySnapshot GetSnapshot(uint32 zoneId) const;

private:
    AiEconomyAnalyzer() = default;
    uint32 _timer = 0;
};

#define sAiEconomyAnalyzer AiEconomyAnalyzer::instance()

#endif
