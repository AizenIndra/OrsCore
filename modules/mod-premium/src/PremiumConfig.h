/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_PREMIUM_CONFIG_H
#define MOD_PREMIUM_CONFIG_H

#include "ConfigValueCache.h"

enum class PremiumConfigOption
{
    Enable,
    CmdDebuff,
    CmdBank,
    CmdMail,
    CmdRepair,
    CmdResetTalents,
    CmdTaxi,
    CmdHome,
    CmdChangeRace,
    CmdCustomize,
    CmdCapital,
    CmdAppear,
    CmdSummon,
    CmdBuff,
    CmdFreeDay,
    CmdBuy,
    RateXpKill,
    RateXpQuest,
    RateHonor,
    RateReputation,
    RateSkillGain,
    Price1Day,
    Price7Days,
    Price31Days,
    NUM_CONFIGS
};

class PremiumConfigData : public ConfigValueCache<PremiumConfigOption>
{
public:
    PremiumConfigData();
    void BuildConfigCache() override;

    [[nodiscard]] bool IsEnabled() const;
    [[nodiscard]] bool IsCmdEnabled(PremiumConfigOption opt) const;
    [[nodiscard]] float GetRateXpKill() const;
    [[nodiscard]] float GetRateXpQuest() const;
    [[nodiscard]] float GetRateHonor() const;
    [[nodiscard]] float GetRateReputation() const;
    [[nodiscard]] float GetRateSkillGain() const;
    [[nodiscard]] uint32 GetPrice1Day() const;
    [[nodiscard]] uint32 GetPrice7Days() const;
    [[nodiscard]] uint32 GetPrice31Days() const;
    [[nodiscard]] uint32 GetPriceForDays(uint32 days) const;
};

PremiumConfigData& sPremiumConfig();

#endif
