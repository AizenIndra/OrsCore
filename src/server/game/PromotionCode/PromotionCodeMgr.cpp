/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PromotionCodeMgr.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "Util.h"
#include "WorldSession.h"

PromotionCodeMgr* PromotionCodeMgr::instance()
{
    static PromotionCodeMgr instance;
    return &instance;
}

PromotionCodeMgr::~PromotionCodeMgr()
{
    _promoCodesStore.clear();
}

void PromotionCodeMgr::Initialize()
{
    _LoadPromoCodes();
    _LoadPromoCodesHistory();
}

void PromotionCodeMgr::ReloadCodes()
{
    _LoadPromoCodes();
    _LoadPromoCodesHistory();
}

void PromotionCodeMgr::_LoadPromoCodes()
{
    uint32 oldMSTime = getMSTime();

    _promoCodesStore.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT id, collection, code, honor, arena, money, item_1, item_2, item_3, item_count_1, item_count_2, item_count_3, aura, spell_1, spell_2, spell_3, coin, count_of_exists FROM promotion_codes");

    if (!result)
    {
        LOG_INFO("server.loading", ">> Loaded 0 Promo Codes. DB table `promotion_codes` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].Get<uint32>();

        PromotionCodes pc;
        pc.collection = fields[1].Get<uint32>();
        pc.code = fields[2].Get<std::string>();
        pc.honor = fields[3].Get<uint32>();
        pc.arena = fields[4].Get<uint32>();
        pc.money = fields[5].Get<uint32>();
        pc.item_1 = fields[6].Get<uint32>();
        pc.item_2 = fields[7].Get<uint32>();
        pc.item_3 = fields[8].Get<uint32>();
        pc.item_count_1 = fields[9].Get<uint32>();
        pc.item_count_2 = fields[10].Get<uint32>();
        pc.item_count_3 = fields[11].Get<uint32>();
        pc.aura = fields[12].Get<uint32>();
        pc.spell_1 = fields[13].Get<uint32>();
        pc.spell_2 = fields[14].Get<uint32>();
        pc.spell_3 = fields[15].Get<uint32>();
        pc.coin = fields[16].Get<uint32>();
        pc.exist_count = fields[17].Get<uint32>();

        if (pc.code.empty())
        {
            LOG_ERROR("server.loading", "PromotionCodeMgr::_LoadPromoCodes: Found promo code with empty code string (id: {}). Skipped.", id);
            continue;
        }

        _promoCodesStore[id] = pc;
        ++count;
    } while (result->NextRow());

    LOG_INFO("server.loading", ">> Loaded {} Promotion Codes in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void PromotionCodeMgr::_LoadPromoCodesHistory()
{
    uint32 oldMSTime = getMSTime();

    _promoHistoryStore.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT id, codeID, code, accountID, playerID, use_unix_time FROM promotion_codes_history");

    if (!result)
    {
        LOG_INFO("server.loading", ">> Loaded 0 Promo Codes History count. DB table `promotion_codes_history` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].Get<uint32>();

        PromoHistory ph;
        ph.codeId = fields[1].Get<uint32>();
        ph.code = fields[2].Get<std::string>();
        ph.accountId = fields[3].Get<uint32>();
        ph.playerGUID = fields[4].Get<uint32>();
        ph.time = fields[5].Get<uint64>();
        _promoHistoryStore[id] = ph;
        ++count;
    } while (result->NextRow());

    LOG_INFO("server.loading", ">> Loaded {} Promotion Codes history count in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

PromotionCodes const* PromotionCodeMgr::GetPromoCode(std::string const& name, uint32& id) const
{
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return nullptr;

    id = 0;
    for (auto const& [promoId, promoCode] : _promoCodesStore)
    {
        if (promoCode.code == name)
        {
            id = promoId;
            return &promoCode;
        }
    }

    return nullptr;
}

bool PromotionCodeMgr::AddPromoCode(PromotionCodes& promo)
{
    if (promo.code.empty())
    {
        LOG_ERROR("server.loading", "PromotionCodeMgr::AddPromoCode: Cannot add promo code with empty code string.");
        return false;
    }

    uint32 existingId = 0;
    if (GetPromoCode(promo.code, existingId))
    {
        LOG_ERROR("server.loading", "PromotionCodeMgr::AddPromoCode: Promo code '{}' already exists (id: {}).", promo.code, existingId);
        return false;
    }

    uint32 new_id = 0;
    for (auto const& [promoId, _] : _promoCodesStore)
        if (promoId > new_id)
            new_id = promoId;
    ++new_id;

    _promoCodesStore[new_id] = promo;

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_PROMO_CODE);
    stmt->SetData(0, new_id);
    stmt->SetData(1, promo.collection);
    stmt->SetData(2, promo.code);
    stmt->SetData(3, promo.honor);
    stmt->SetData(4, promo.arena);
    stmt->SetData(5, promo.money);
    stmt->SetData(6, promo.item_1);
    stmt->SetData(7, promo.item_2);
    stmt->SetData(8, promo.item_3);
    stmt->SetData(9, promo.item_count_1);
    stmt->SetData(10, promo.item_count_2);
    stmt->SetData(11, promo.item_count_3);
    stmt->SetData(12, promo.aura);
    stmt->SetData(13, promo.spell_1);
    stmt->SetData(14, promo.spell_2);
    stmt->SetData(15, promo.spell_3);
    stmt->SetData(16, promo.coin);
    stmt->SetData(17, promo.exist_count);
    WorldDatabase.Execute(stmt);

    LOG_INFO("server.loading", "PromotionCodeMgr::AddPromoCode: Added promo code '{}' with id {}.", promo.code, new_id);
    return true;
}

bool PromotionCodeMgr::DeletePromoCode(std::string const& name)
{
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return false;

    for (auto itr = _promoCodesStore.begin(); itr != _promoCodesStore.end(); ++itr)
    {
        if (itr->second.code == name)
        {
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_PROMO_CODE);
            stmt->SetData(0, itr->second.code);
            WorldDatabase.Execute(stmt);
            _promoCodesStore.erase(itr);
            LOG_INFO("server.loading", "PromotionCodeMgr::DeletePromoCode: Deleted promo code '{}'.", name);
            return true;
        }
    }

    LOG_WARN("server.loading", "PromotionCodeMgr::DeletePromoCode: Promo code '{}' not found.", name);
    return false;
}

bool PromotionCodeMgr::_UpdateCountOfExistPromoCode(uint32 id, Player* player)
{
    auto itr = _promoCodesStore.find(id);
    if (itr == _promoCodesStore.end())
    {
        LOG_ERROR("server.loading", "PromotionCodeMgr::_UpdateCountOfExistPromoCode: Promo code with id {} not found.", id);
        return false;
    }

    if (itr->second.exist_count == 0)
    {
        LOG_WARN("server.loading", "PromotionCodeMgr::_UpdateCountOfExistPromoCode: Promo code '{}' (id: {}) has no remaining uses.", itr->second.code, id);
        return false;
    }

    itr->second.exist_count -= 1;
    std::string code = itr->second.code;

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_PROMO_CODE_USED);
    stmt->SetData(0, itr->second.exist_count);
    stmt->SetData(1, id);
    WorldDatabase.Execute(stmt);

    return _AddCodeInHistory(id, code, player);
}

bool PromotionCodeMgr::CheckedEnteredCodeByPlayer(std::string const& code, Player* player, uint32 collection)
{
    if (!player || !player->GetSession())
        return false;

    std::wstring wname;
    if (!Utf8toWStr(code, wname))
        return false;

    if (!_CanUseCode(code, player->GetGUID().GetCounter()))
        return false;

    uint32 codeid = _TryToRewardForCode(code, player, collection);
    if (!codeid)
        return false;

    return _UpdateCountOfExistPromoCode(codeid, player);
}

uint32 PromotionCodeMgr::_TryToRewardForCode(std::string const& code, Player* player, uint32 collection)
{
    uint32 result = 0;

    for (auto const& [promoId, promoCode] : _promoCodesStore)
    {
        if (promoCode.code != code || promoCode.exist_count == 0)
            continue;

        if (promoCode.collection != 0)
        {
            if (collection == 0 || collection != promoCode.collection)
                continue;
        }

        result = promoId;

        if (promoCode.arena > 0)
            player->ModifyArenaPoints(promoCode.arena);

        if (promoCode.honor > 0)
            player->ModifyHonorPoints(promoCode.honor);

        if (promoCode.item_1 > 0)
        {
            uint32 count = (promoCode.item_count_1 > 1) ? promoCode.item_count_1 : 1;
            player->AddItem(promoCode.item_1, count);
        }

        if (promoCode.item_2 > 0)
        {
            uint32 count = (promoCode.item_count_2 > 1) ? promoCode.item_count_2 : 1;
            player->AddItem(promoCode.item_2, count);
        }

        if (promoCode.item_3 > 0)
        {
            uint32 count = (promoCode.item_count_3 > 1) ? promoCode.item_count_3 : 1;
            player->AddItem(promoCode.item_3, count);
        }

        if (promoCode.money > 0)
            player->ModifyMoney(promoCode.money);

        if (promoCode.coin > 0)
            player->GetSession()->AddDonateBonusOrVote(promoCode.coin, 1, false);

        if (promoCode.spell_1 > 0)
            player->learnSpell(promoCode.spell_1, false, false);

        if (promoCode.spell_2 > 0)
            player->learnSpell(promoCode.spell_2, false, false);

        if (promoCode.spell_3 > 0)
            player->learnSpell(promoCode.spell_3, false, false);

        if (promoCode.aura > 0)
            player->AddAura(promoCode.aura, player);

        break;
    }

    return result;
}

bool PromotionCodeMgr::_AddCodeInHistory(uint32 id, std::string const& code, Player* player)
{
    if (!player || !player->GetSession())
    {
        LOG_ERROR("server.loading", "PromotionCodeMgr::_AddCodeInHistory: Invalid player or session.");
        return false;
    }

    uint32 new_id = 0;
    for (auto const& [historyId, _] : _promoHistoryStore)
        if (historyId > new_id)
            new_id = historyId;
    ++new_id;

    PromoHistory ph;
    ph.codeId = id;
    ph.code = code;
    ph.accountId = player->GetSession()->GetAccountId();
    ph.playerGUID = player->GetGUID().GetCounter();
    ph.time = GameTime::GetGameTime().count();
    _promoHistoryStore[new_id] = ph;

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_PROMO_CODE_HISTORY);
    stmt->SetData(0, new_id);
    stmt->SetData(1, ph.codeId);
    stmt->SetData(2, ph.code);
    stmt->SetData(3, ph.accountId);
    stmt->SetData(4, ph.playerGUID);
    stmt->SetData(5, uint64(ph.time));
    WorldDatabase.Execute(stmt);

    return true;
}

bool PromotionCodeMgr::_CanUseCode(std::string const& code, ObjectGuid::LowType plrGUID) const
{
    for (auto const& [_, history] : _promoHistoryStore)
        if (history.playerGUID == plrGUID && history.code == code)
            return false;

    return true;
}

bool PromotionCodeMgr::CanUsePromoCode(std::string const& code, Player* player) const
{
    if (!player)
        return false;

    uint32 codeId = 0;
    PromotionCodes const* promoCode = GetPromoCode(code, codeId);
    if (!promoCode)
        return false;

    if (promoCode->exist_count == 0)
        return false;

    if (!_CanUseCode(code, player->GetGUID().GetCounter()))
        return false;

    return true;
}
