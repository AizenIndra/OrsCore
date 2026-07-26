/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "PremiumMgr.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "QueryResult.h"
#include "SharedDefines.h"

namespace
{
constexpr uint32 PremiumMountSpells[] = { 31700, 18991, 18992 };
}

PremiumMgr* PremiumMgr::instance()
{
    static PremiumMgr mgr;
    return &mgr;
}

void PremiumMgr::LoadAccount(uint32 accountId)
{
    Premium::AccountRecord record;
    if (QueryResult result = LoginDatabase.Query(
        "SELECT UNIX_TIMESTAMP(EndTime), active, chat_text_color FROM account_premium "
        "WHERE id = {} AND active = 1 AND EndTime > CURRENT_TIMESTAMP()", accountId))
    {
        Field* fields = result->Fetch();
        record.endTime = time_t(fields[0].Get<uint64>());
        record.active = fields[1].Get<uint8>() != 0;
        record.chatTextColor = fields[2].Get<uint8>();
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _cache[accountId] = record;
}

void PremiumMgr::Invalidate(uint32 accountId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cache.erase(accountId);
}

bool PremiumMgr::IsPremium(uint32 accountId) const
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(accountId);
        if (it != _cache.end())
        {
            if (!it->second.active || it->second.endTime <= GameTime::GetGameTime().count())
                return false;
            return true;
        }
    }

    // Cache miss — sync query (login path usually preloads)
    const_cast<PremiumMgr*>(this)->LoadAccount(accountId);
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _cache.find(accountId);
    return it != _cache.end() && it->second.active
        && it->second.endTime > GameTime::GetGameTime().count();
}

bool PremiumMgr::IsPremium(Player const* player) const
{
    if (!player || !player->GetSession())
        return false;
    return IsPremium(player->GetSession()->GetAccountId());
}

time_t PremiumMgr::GetEndTime(uint32 accountId) const
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(accountId);
        if (it != _cache.end())
            return it->second.endTime;
    }

    if (QueryResult result = LoginDatabase.Query(
        "SELECT UNIX_TIMESTAMP(EndTime) FROM account_premium WHERE id = {} AND active = 1", accountId))
        return time_t(result->Fetch()[0].Get<uint64>());
    return 0;
}

uint8 PremiumMgr::GetChatTextColor(uint32 accountId) const
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(accountId);
        if (it != _cache.end())
            return it->second.chatTextColor;
    }

    if (QueryResult result = LoginDatabase.Query(
        "SELECT chat_text_color FROM account_premium WHERE id = {}", accountId))
        return result->Fetch()[0].Get<uint8>();
    return 1;
}

void PremiumMgr::SetOrUpdate(uint32 accountId, time_t endTime)
{
    uint8 color = 1;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(accountId);
        if (it != _cache.end() && it->second.chatTextColor)
            color = it->second.chatTextColor;
    }

    LoginDatabase.Execute(
        "INSERT INTO account_premium (id, StartTime, EndTime, active, chat_text_color) "
        "VALUES ({}, CURRENT_TIMESTAMP(), FROM_UNIXTIME({}), 1, {}) "
        "ON DUPLICATE KEY UPDATE StartTime = CURRENT_TIMESTAMP(), EndTime = FROM_UNIXTIME({}), active = 1",
        accountId, uint64(endTime), uint32(color), uint64(endTime));

    Premium::AccountRecord record;
    record.active = true;
    record.endTime = endTime;
    record.chatTextColor = color;

    std::lock_guard<std::mutex> lock(_mutex);
    _cache[accountId] = record;
}

void PremiumMgr::Extend(uint32 accountId, time_t addSeconds)
{
    if (addSeconds <= 0)
        return;

    time_t now = GameTime::GetGameTime().count();
    time_t current = GetEndTime(accountId);
    time_t base = (IsPremium(accountId) && current > now) ? current : now;
    SetOrUpdate(accountId, base + addSeconds);
}

void PremiumMgr::Remove(uint32 accountId)
{
    LoginDatabase.Execute("DELETE FROM account_premium WHERE id = {}", accountId);
    std::lock_guard<std::mutex> lock(_mutex);
    _cache.erase(accountId);
}

void PremiumMgr::SetChatTextColor(uint32 accountId, uint8 colorId)
{
    LoginDatabase.Execute(
        "UPDATE account_premium SET chat_text_color = {} WHERE id = {}",
        uint32(colorId), accountId);

    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _cache.find(accountId);
    if (it != _cache.end())
        it->second.chatTextColor = colorId;
}

bool PremiumMgr::HasClaimedFreeDay(uint32 accountId) const
{
    QueryResult result = LoginDatabase.Query(
        "SELECT 1 FROM account_premium_free_day WHERE id = {}", accountId);
    return result != nullptr;
}

void PremiumMgr::MarkFreeDayClaimed(uint32 accountId)
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    LoginDatabase.Execute(
        "INSERT INTO account_premium_free_day (id, claimed_at) VALUES ({}, {})",
        accountId, now);
}

void PremiumMgr::EnsureBonusRow(uint32 accountId)
{
    LoginDatabase.Execute(
        "INSERT IGNORE INTO account_donate (id, bonuses, votes, total_bonuses, total_votes) "
        "VALUES ({}, 0, 0, 0, 0)", accountId);
}

uint32 PremiumMgr::GetBonuses(uint32 accountId) const
{
    if (QueryResult result = LoginDatabase.Query(
        "SELECT bonuses FROM account_donate WHERE id = {}", accountId))
        return result->Fetch()[0].Get<uint32>();

    LoginDatabase.Execute(
        "INSERT IGNORE INTO account_donate (id, bonuses, votes, total_bonuses, total_votes) "
        "VALUES ({}, 0, 0, 0, 0)", accountId);
    return 0;
}

bool PremiumMgr::TrySpendBonuses(uint32 accountId, uint32 cost)
{
    if (!cost)
        return true;

    EnsureBonusRow(accountId);
    uint32 balance = GetBonuses(accountId);
    if (balance < cost)
        return false;

    uint32 left = balance - cost;
    LoginDatabase.Execute(
        "UPDATE account_donate SET bonuses = {} WHERE id = {}", left, accountId);
    return true;
}

void PremiumMgr::AddBonuses(uint32 accountId, uint32 amount)
{
    if (!amount)
        return;

    EnsureBonusRow(accountId);
    LoginDatabase.Execute(
        "UPDATE account_donate SET bonuses = bonuses + {}, total_bonuses = total_bonuses + {} WHERE id = {}",
        amount, amount, accountId);
}

void PremiumMgr::ApplyMounts(Player* player)
{
    if (!player)
        return;
    for (uint32 spellId : PremiumMountSpells)
        if (!player->HasSpell(spellId))
            player->learnSpell(spellId, false, false);
}

void PremiumMgr::RemoveMounts(Player* player)
{
    if (!player)
        return;
    for (uint32 spellId : PremiumMountSpells)
        if (player->HasSpell(spellId))
            player->removeSpell(spellId, SPEC_MASK_ALL, false);
}

void PremiumMgr::SyncPlayer(Player* player)
{
    if (!player || !player->GetSession())
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    LoadAccount(accountId);
    if (IsPremium(accountId))
        ApplyMounts(player);
    else
        RemoveMounts(player);
}
