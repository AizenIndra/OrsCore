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

#ifndef PROMOTIONCODE_MGR_H
#define PROMOTIONCODE_MGR_H

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <unordered_map>

struct PromotionCodes
{
    uint32 collection = 0;
    std::string code;
    uint32 honor = 0;
    uint32 arena = 0;
    uint32 money = 0;
    uint32 item_1 = 0;
    uint32 item_2 = 0;
    uint32 item_3 = 0;
    uint32 item_count_1 = 0;
    uint32 item_count_2 = 0;
    uint32 item_count_3 = 0;
    uint32 aura = 0;
    uint32 spell_1 = 0;
    uint32 spell_2 = 0;
    uint32 spell_3 = 0;
    uint32 coin = 0;
    uint32 exist_count = 0;
};

typedef std::unordered_map<uint32, PromotionCodes> PromotionCodesContainer;

struct PromoHistory
{
    uint32 codeId = 0;
    uint32 accountId = 0;
    uint32 playerGUID = 0;
    std::string code;
    time_t time = 0;
};

typedef std::unordered_map<uint32, PromoHistory> PromotionHistoryContainer;

class Player;

class AC_GAME_API PromotionCodeMgr
{
private:
    PromotionCodeMgr() { }
    ~PromotionCodeMgr();

public:
    static PromotionCodeMgr* instance();

    void Initialize();
    void ReloadCodes();

    PromotionCodes const* GetPromoCode(uint32 id) const
    {
        PromotionCodesContainer::const_iterator itr = _promoCodesStore.find(id);
        if (itr == _promoCodesStore.end())
            return nullptr;
        return &itr->second;
    }

    PromotionCodes const* GetPromoCode(std::string const& name, uint32& id) const;
    PromotionCodesContainer const& GetPromotionCodesMap() const { return _promoCodesStore; }
    bool AddPromoCode(PromotionCodes& data);
    bool DeletePromoCode(std::string const& name);

    bool CheckedEnteredCodeByPlayer(std::string const& code, Player* player, uint32 collection = 0);
    bool CanUsePromoCode(std::string const& code, Player* player) const;

protected:
    void _LoadPromoCodes();
    void _LoadPromoCodesHistory();

    uint32 _TryToRewardForCode(std::string const& code, Player* player, uint32 collection = 0);
    bool _UpdateCountOfExistPromoCode(uint32 id, Player* player);

    bool _AddCodeInHistory(uint32 id, std::string const& code, Player* player);
    bool _CanUseCode(std::string const& code, ObjectGuid::LowType plrGUID) const;

private:
    PromotionCodesContainer _promoCodesStore;
    PromotionHistoryContainer _promoHistoryStore;
};

#define sPromotionCodeMgr PromotionCodeMgr::instance()

#endif
