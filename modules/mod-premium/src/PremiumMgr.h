/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_PREMIUM_MGR_H
#define MOD_PREMIUM_MGR_H

#include "PremiumCommon.h"
#include <mutex>
#include <unordered_map>

class Player;

class PremiumMgr
{
public:
    static PremiumMgr* instance();

    [[nodiscard]] bool IsPremium(uint32 accountId) const;
    [[nodiscard]] bool IsPremium(Player const* player) const;
    [[nodiscard]] time_t GetEndTime(uint32 accountId) const;
    [[nodiscard]] uint8 GetChatTextColor(uint32 accountId) const;

    void LoadAccount(uint32 accountId);
    void Invalidate(uint32 accountId);
    void SetOrUpdate(uint32 accountId, time_t endTime);
    void Extend(uint32 accountId, time_t addSeconds);
    void Remove(uint32 accountId);
    void SetChatTextColor(uint32 accountId, uint8 colorId);

    [[nodiscard]] bool HasClaimedFreeDay(uint32 accountId) const;
    void MarkFreeDayClaimed(uint32 accountId);

    [[nodiscard]] uint32 GetBonuses(uint32 accountId) const;
    void EnsureBonusRow(uint32 accountId);
    bool TrySpendBonuses(uint32 accountId, uint32 cost);
    void AddBonuses(uint32 accountId, uint32 amount);

    void ApplyMounts(Player* player);
    void RemoveMounts(Player* player);
    void SyncPlayer(Player* player);

private:
    PremiumMgr() = default;

    mutable std::mutex _mutex;
    mutable std::unordered_map<uint32, Premium::AccountRecord> _cache;
};

#define sPremiumMgr PremiumMgr::instance()

#endif
