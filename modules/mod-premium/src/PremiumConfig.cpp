/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "PremiumConfig.h"

PremiumConfigData::PremiumConfigData()
    : ConfigValueCache(PremiumConfigOption::NUM_CONFIGS)
{
}

void PremiumConfigData::BuildConfigCache()
{
    SetConfigValue<bool>(PremiumConfigOption::Enable, "Premium.Enable", false);
    SetConfigValue<bool>(PremiumConfigOption::CmdDebuff, "Premium.Command.Debuff", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdBank, "Premium.Command.Bank", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdMail, "Premium.Command.Mail", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdRepair, "Premium.Command.Repair", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdResetTalents, "Premium.Command.ResetTalents", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdTaxi, "Premium.Command.Taxi", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdHome, "Premium.Command.Home", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdChangeRace, "Premium.Command.ChangeRace", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdCustomize, "Premium.Command.Customize", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdCapital, "Premium.Command.Capital", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdAppear, "Premium.Command.Appear", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdSummon, "Premium.Command.Summon", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdBuff, "Premium.Command.Buff", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdFreeDay, "Premium.Command.FreeDay", true);
    SetConfigValue<bool>(PremiumConfigOption::CmdBuy, "Premium.Command.Buy", true);
    SetConfigValue<float>(PremiumConfigOption::RateXpKill, "Premium.Rate.XP.Kill", 1.0f);
    SetConfigValue<float>(PremiumConfigOption::RateXpQuest, "Premium.Rate.XP.Quest", 1.0f);
    SetConfigValue<float>(PremiumConfigOption::RateHonor, "Premium.Rate.Honor", 1.0f);
    SetConfigValue<float>(PremiumConfigOption::RateReputation, "Premium.Rate.Reputation", 1.0f);
    SetConfigValue<float>(PremiumConfigOption::RateSkillGain, "Premium.Rate.SkillGain", 1.0f);
    SetConfigValue<float>(PremiumConfigOption::RateRankReward, "Premium.Rate.Rank.Reward", 1.0f);
    SetConfigValue<uint32>(PremiumConfigOption::Price1Day, "Premium.Price.1Day", 10);
    SetConfigValue<uint32>(PremiumConfigOption::Price7Days, "Premium.Price.7Days", 350);
    SetConfigValue<uint32>(PremiumConfigOption::Price31Days, "Premium.Price.31Days", 600);
}

bool PremiumConfigData::IsEnabled() const
{
    return GetConfigValue<bool>(PremiumConfigOption::Enable);
}

bool PremiumConfigData::IsCmdEnabled(PremiumConfigOption opt) const
{
    return GetConfigValue<bool>(opt);
}

float PremiumConfigData::GetRateXpKill() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateXpKill);
}

float PremiumConfigData::GetRateXpQuest() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateXpQuest);
}

float PremiumConfigData::GetRateHonor() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateHonor);
}

float PremiumConfigData::GetRateReputation() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateReputation);
}

float PremiumConfigData::GetRateSkillGain() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateSkillGain);
}

float PremiumConfigData::GetRateRankReward() const
{
    return GetConfigValue<float>(PremiumConfigOption::RateRankReward);
}

uint32 PremiumConfigData::GetPrice1Day() const
{
    return GetConfigValue<uint32>(PremiumConfigOption::Price1Day);
}

uint32 PremiumConfigData::GetPrice7Days() const
{
    return GetConfigValue<uint32>(PremiumConfigOption::Price7Days);
}

uint32 PremiumConfigData::GetPrice31Days() const
{
    return GetConfigValue<uint32>(PremiumConfigOption::Price31Days);
}

uint32 PremiumConfigData::GetPriceForDays(uint32 days) const
{
    switch (days)
    {
        case 1:  return GetPrice1Day();
        case 7:  return GetPrice7Days();
        case 31: return GetPrice31Days();
        default: return 0;
    }
}

PremiumConfigData& sPremiumConfig()
{
    static PremiumConfigData instance;
    return instance;
}
