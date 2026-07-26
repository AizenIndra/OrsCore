/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraConfig.h"

MakGoraConfigData::MakGoraConfigData()
    : ConfigValueCache(MakGoraConfigOption::NUM_CONFIGS)
{
}

void MakGoraConfigData::BuildConfigCache()
{
    SetConfigValue<bool>(MakGoraConfigOption::Enable, "MakGora.Enable", false);
    SetConfigValue<uint32>(MakGoraConfigOption::MaxLevel, "MakGora.MaxLevel", 60);
    SetConfigValue<uint32>(MakGoraConfigOption::MinLevel, "MakGora.MinLevel", 60);
    SetConfigValue<bool>(MakGoraConfigOption::RequireSameFaction, "MakGora.RequireSameFaction", true);
    SetConfigValue<bool>(MakGoraConfigOption::PermadeathOnLoss, "MakGora.PermadeathOnLoss", true);
    SetConfigValue<bool>(MakGoraConfigOption::AnnounceWorld, "MakGora.AnnounceWorld", true);
    SetConfigValue<uint32>(MakGoraConfigOption::ChampionEntry, "MakGora.ChampionEntry", 0);
    SetConfigValue<std::string>(MakGoraConfigOption::AddonPrefix, "MakGora.AddonPrefix", std::string("MakGora"));
}

bool MakGoraConfigData::IsEnabled() const
{
    return GetConfigValue<bool>(MakGoraConfigOption::Enable);
}

uint8 MakGoraConfigData::GetMaxLevel() const
{
    return static_cast<uint8>(GetConfigValue<uint32>(MakGoraConfigOption::MaxLevel));
}

uint8 MakGoraConfigData::GetMinLevel() const
{
    return static_cast<uint8>(GetConfigValue<uint32>(MakGoraConfigOption::MinLevel));
}

bool MakGoraConfigData::RequireSameFaction() const
{
    return GetConfigValue<bool>(MakGoraConfigOption::RequireSameFaction);
}

bool MakGoraConfigData::PermadeathOnLoss() const
{
    return GetConfigValue<bool>(MakGoraConfigOption::PermadeathOnLoss);
}

bool MakGoraConfigData::AnnounceWorld() const
{
    return GetConfigValue<bool>(MakGoraConfigOption::AnnounceWorld);
}

uint32 MakGoraConfigData::GetChampionEntry() const
{
    return GetConfigValue<uint32>(MakGoraConfigOption::ChampionEntry);
}

std::string_view MakGoraConfigData::GetAddonPrefix() const
{
    return GetConfigValue(MakGoraConfigOption::AddonPrefix);
}

MakGoraConfigData& sMakGoraConfig()
{
    static MakGoraConfigData instance;
    return instance;
}
