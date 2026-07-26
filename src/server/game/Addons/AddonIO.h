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

#ifndef _ADDONIO_H
#define _ADDONIO_H

#include "Player.h"

class AddonIO
{
private:
	AddonIO();
	~AddonIO();

public:
	static AddonIO* instance();

	void HandleMessage(Player* player, std::string message);

	// Transmogrification
	void HandleTransmogrificationInfoRequest(Player* player, std::string body);
	void HandleTransmogrificationPrepareRequest(Player* player, std::string body);
	void HandleTransmogrificationRemove(Player* player, std::string body);
	void HandleTransmogrificationApply(Player* player, std::string body);
    
    //Donate Service
    void HandleShopBalanceRequest(Player* player, std::string body);
    //void HandlePremiumInfoRequest(Player* player, std::string body);
    //void HandlePremiumRenewRequest(Player* player, std::string body);
    void HandleShopItemListRequest(Player* player, std::string body);
    void HandleShopRefundableListRequest(Player* player, std::string body);
    void HandleShopVersionRequest(Player* player, std::string body);
    void HandleShopBuyItemRequest(Player* player, std::string body);
    void HandleShopSpecialOfferListRequest(Player* player, std::string body);
    void HandleShopSubcriptionListRequest(Player* player, std::string body);
    void HandleShopCategoryNewItemsRequest(Player* player, std::string body);
    void HandleShopSubscribeRequest(Player* player, std::string body);
    void HandleShopPurchaseRefundRequest(Player* player, std::string body);
    void HandleShopCollectionLoadRequest(Player* player, std::string body);
    void HandleShopItemCountRequest(Player* player, std::string body);

    //Other
    void HandleAverageItemLevelRequest(Player* player, std::string body);

    void HandleGuildSpellsRequest(Player* player, std::string body);
    void HandleGuildLevelRequest(Player* player, std::string body);
    void HandleGuildOnlineRequest(Player* player, std::string body);
    void HandleGuildIlvlsRequest(Player* player, std::string body);
    void HandleGuildEmblemRequest(Player* player, std::string body);
    void HandleGuildTeamRequest(Player* player, std::string body);
    void HandleGuildFinderBrowse(Player* player, std::string body);
    void HandleGuildFinderGetApplications(Player* player, std::string body);
    void HandleGuildFinderAddRecruit(Player* player, std::string body);
    void HandleGuildFinderRemoveRecruit(Player* player, std::string body);
    void HandleGuildFinderGetRecruits(Player* player, std::string body);
    void HandleGuildFinderDeclineRecruit(Player* player, std::string body);
    void HandleGuildFinderPostRequest(Player* player, std::string body);
    void HandleGuildFinderSetGuildPost(Player* player, std::string body);

    /// Clears guild-finder pending applications after the player joins a guild (any guild).
    void RemoveGuildFinderApplicationsForPlayer(ObjectGuid playerGuid);
};

typedef void(AddonIO::*AddonMessageHandler)(Player*, std::string);

#define sAddonIO AddonIO::instance()

#endif // _ADDONIO_H
