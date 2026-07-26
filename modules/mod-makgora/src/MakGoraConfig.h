/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_MAKGORA_CONFIG_H
#define MOD_MAKGORA_CONFIG_H

#include "ConfigValueCache.h"
#include "MakGoraCommon.h"
#include <string>

enum class MakGoraConfigOption
{
    Enable,
    MaxLevel,
    MinLevel,
    RequireSameFaction,
    PermadeathOnLoss,
    AnnounceWorld,
    ChampionEntry,
    AddonPrefix,
    NUM_CONFIGS
};

class MakGoraConfigData : public ConfigValueCache<MakGoraConfigOption>
{
public:
    MakGoraConfigData();

    void BuildConfigCache() override;

    [[nodiscard]] bool IsEnabled() const;
    [[nodiscard]] uint8 GetMaxLevel() const;
    [[nodiscard]] uint8 GetMinLevel() const;
    [[nodiscard]] bool RequireSameFaction() const;
    [[nodiscard]] bool PermadeathOnLoss() const;
    [[nodiscard]] bool AnnounceWorld() const;
    [[nodiscard]] uint32 GetChampionEntry() const;
    [[nodiscard]] std::string_view GetAddonPrefix() const;
};

MakGoraConfigData& sMakGoraConfig();

#endif
