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

#include "AddonIO.h"
#include "TransmogrificationMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "PromotionCodeMgr.h"
#include "Log.h"
#include <unordered_map>
#include <algorithm>
#include "Tokenize.h"
#include "StringFormat.h"
#include "SharedDefines.h"
#include "GameTime.h"
#include "World.h"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "PreparedStatement.h"
#include "DatabaseEnv.h"
#include "WorldSessionMgr.h"

#define INSPECT_DISTANCE                28.0f
#define ASMSG_HARDCORE_DEATH            "ASMSG_HARDCORE_DEATH"

namespace
{
    constexpr uint32 STORE_ITEM_FLAG_SPECIAL = 0x00000010u;

    uint8 ResolveStoreDiscount(uint32 price, uint8 discount, uint32 discountPrice)
    {
        if (discount > 0)
            return discount;

        if (price > 0 && discountPrice > 0 && discountPrice < price)
        {
            uint32 discountValue = ((price - discountPrice) * 100u) / price;
            if (discountValue == 0)
                discountValue = 1;
            if (discountValue > 99u)
                discountValue = 99u;

            return static_cast<uint8>(discountValue);
        }

        return 0;
    }

    uint32 ResolveStoreDiscountPriceForClient(uint32 price, uint8 discount, uint32 discountPrice)
    {
        if (discount > 0 && discountPrice > 0 && discountPrice < price)
            return discountPrice;

        return 0;
    }

    uint32 ResolveStorePriceForPurchase(uint32 price, uint8 discount, uint32 discountPrice)
    {
        if (discount > 0 && discountPrice > 0 && discountPrice < price)
            return discountPrice;

        return price;
    }

    std::string GF_EscapeGuildFinderField(std::string_view s)
    {
        std::string r;
        r.reserve(s.size());
        for (unsigned char c : s)
        {
            if (c == '|' || c == '\t' || c == '\n' || c == '\r')
                r.push_back(' ');
            else
                r.push_back(static_cast<char>(c));
        }
        return r;
    }

    struct GuildFinderPendingApplication
    {
        ObjectGuid PlayerGuid;
        uint32 GuildId = 0;
        uint32 ClassRoles = 0;
        uint32 Interests = 0;
        uint32 Availability = 0;
        std::string Comment;
        std::string PlayerName;
        uint8 Level = 1;
        uint8 ClassId = 1;
        time_t CreatedAt = 0;
    };

    struct GuildFinderGuildListing
    {
        uint32 Interests = 31;
        uint32 Availability = 3;
        uint32 ClassRoles = 7;
        uint8 LevelMode = 1;
        bool Listed = true;
        std::string Comment;
    };

    std::vector<GuildFinderPendingApplication> GF_Applications;
    std::unordered_map<uint32, GuildFinderGuildListing> GF_GuildListings;

    constexpr uint32 GF_MAX_PENDING_APPLICATIONS_PER_PLAYER = 10;
    constexpr uint32 GF_APPLICATION_MAX_AGE_SEC = 14 * 86400;

    void GF_PruneExpiredApplications()
    {
        time_t const now = GameTime::GetGameTime().count();
        GF_Applications.erase(std::remove_if(GF_Applications.begin(), GF_Applications.end(),
            [&](GuildFinderPendingApplication const& a)
            {
                return now > a.CreatedAt && (now - a.CreatedAt > GF_APPLICATION_MAX_AGE_SEC);
            }), GF_Applications.end());
    }

    uint32 GF_CountPlayerApplications(ObjectGuid guid)
    {
        uint32 n = 0;
        for (GuildFinderPendingApplication const& a : GF_Applications)
            if (a.PlayerGuid == guid)
                ++n;
        return n;
    }

    bool GF_HasPendingApplication(ObjectGuid guid, uint32 guildId)
    {
        for (GuildFinderPendingApplication const& a : GF_Applications)
            if (a.PlayerGuid == guid && a.GuildId == guildId)
                return true;
        return false;
    }

    void GF_NotifyApplicantsChanged(Player* applicant)
    {
        if (applicant)
            applicant->SendAddonMessage("ASMSG_GF_APPLICATIONS_LIST_CHANGED\t");
    }

    void GF_NotifyGuildApplicantsUpdated(Guild* guild)
    {
        if (!guild)
            return;

        Guild* g = guild;
        auto notifyApplicantList = [g](Player* p)
        {
            if (g->HasRankRight(p, GR_RIGHT_INVITE))
                p->SendAddonMessage("ASMSG_GF_APPLICANT_LIST_UPDATED\t");
        };
        guild->BroadcastWorker(notifyApplicantList);
    }

    GuildFinderGuildListing GF_GetOrCreateListing(Guild const* guild)
    {
        uint32 const id = guild->GetId();
        auto itr = GF_GuildListings.find(id);
        if (itr != GF_GuildListings.end())
            return itr->second;

        GuildFinderGuildListing listing;
        listing.Comment = GF_EscapeGuildFinderField(guild->GetInfo());
        if (listing.Comment.empty())
            listing.Comment = GF_EscapeGuildFinderField(guild->GetMOTD());
        GF_GuildListings[id] = listing;
        return listing;
    }

    void GF_SendPostUpdated(Player* player, Guild const* guild)
    {
        GuildFinderGuildListing listing = GF_GetOrCreateListing(guild);
        bool const isLeader = guild->GetLeaderGUID() == player->GetGUID();

        std::string msg = Acore::StringFormat(
            "ASMSG_GF_POST_UPDATED\t{}|{}|{}|",
            isLeader ? 1 : 0,
            listing.Listed ? 1 : 0,
            listing.LevelMode);

        msg += listing.Comment;
        msg += Acore::StringFormat("|{}|{}|{}",
            listing.Availability,
            listing.ClassRoles,
            listing.Interests);

        player->SendAddonMessage(msg.c_str());
    }

    void GF_RemoveAllApplicationsForPlayer(ObjectGuid playerGuid)
    {
        GF_PruneExpiredApplications();

        std::unordered_set<uint32> guildsToNotify;
        auto const endIt = GF_Applications.end();
        auto const newEnd = std::remove_if(GF_Applications.begin(), endIt,
            [&](GuildFinderPendingApplication const& a)
            {
                if (a.PlayerGuid == playerGuid)
                {
                    guildsToNotify.insert(a.GuildId);
                    return true;
                }
                return false;
            });

        if (newEnd == endIt)
            return;

        GF_Applications.erase(newEnd, endIt);

        if (Player* applicant = ObjectAccessor::FindConnectedPlayer(playerGuid))
            GF_NotifyApplicantsChanged(applicant);

        for (uint32 gid : guildsToNotify)
            if (Guild* g = sGuildMgr->GetGuildById(gid))
                GF_NotifyGuildApplicantsUpdated(g);
    }
}

std::unordered_map<std::string, AddonMessageHandler> addonMessagesTable =
{
    // Transmogrification
    { "ACMSG_TRANSMOGRIFICATION_INFO_REQUEST",          &AddonIO::HandleTransmogrificationInfoRequest      },
    { "ACMSG_TRANSMOGRIFICATION_PREPARE_REQUEST",       &AddonIO::HandleTransmogrificationPrepareRequest   },
    { "ACMSG_TRANSMOGRIFICATION_REMOVE",                &AddonIO::HandleTransmogrificationRemove           },
    { "ACMSG_TRANSMOGRIFICATION_APPLY",                 &AddonIO::HandleTransmogrificationApply            },
    { "ACMSG_AVERAGE_ITEM_LEVEL_REQUEST",               &AddonIO::HandleAverageItemLevelRequest            },
    //Donate Service
    { "ACMSG_SHOP_BALANCE_REQUEST",                     &AddonIO::HandleShopBalanceRequest                 }, 
    //{ "ACMSG_PREMIUM_INFO_REQUEST",                     &AddonIO::HandlePremiumInfoRequest                 },
    //{ "ACMSG_PREMIUM_RENEW_REQUEST",                    &AddonIO::HandlePremiumRenewRequest                },
    { "ACMSG_SHOP_ITEM_LIST_REQUEST",                   &AddonIO::HandleShopItemListRequest                },
    /*{ "ACMSG_SHOP_REFUNDABLE_PURCHASE_LIST_REQUEST",    &AddonIO::HandleShopRefundableListRequest          },*/
    { "ACMSG_SHOP_VERSION",                             &AddonIO::HandleShopVersionRequest                 },
    { "ACMSG_SHOP_BUY_ITEM",                            &AddonIO::HandleShopBuyItemRequest                 },
    { "ACMSG_SHOP_SPECIAL_OFFER_LIST_REQUEST",          &AddonIO::HandleShopSpecialOfferListRequest        },
    /*{ "ACMSG_SHOP_SUBSCRIPTION_LIST_REQUEST",           &AddonIO::HandleShopSubcriptionListRequest         },*/
    /*{ "ACMSG_SHOP_CATEGORY_NEW_ITEMS_REQUEST",          &AddonIO::HandleShopCategoryNewItemsRequest        },*/
    /*{ "ACMSG_SHOP_SUBSCRIBE",                           &AddonIO::HandleShopSubscribeRequest               },*/
    /*{ "ACMSG_SHOP_PURCHASE_REFUND",                     &AddonIO::HandleShopPurchaseRefundRequest          },*/
    { "ACMSG_SHOP_COLLECTION_LOAD_REQUEST",             &AddonIO::HandleShopCollectionLoadRequest          },
    { "ACMSG_SHOP_ITEM_COUNT",                          &AddonIO::HandleShopItemCountRequest               },
    { "ACMSG_HARDCORE_CREATE_SET",                      &AddonIO::HandleHardcoreCreateSet                  },
    // Guild System
    { "ACMSG_GUILD_SPELLS_REQUEST",                     &AddonIO::HandleGuildSpellsRequest                 },
    { "ACMSG_GUILD_LEVEL_REQUEST",                      &AddonIO::HandleGuildLevelRequest                  },
    { "ACMSG_GUILD_ONLINE_REQUEST",                     &AddonIO::HandleGuildOnlineRequest                 },
    { "ACMSG_GUILD_ILVLS_REQUEST",                      &AddonIO::HandleGuildIlvlsRequest                  },
    { "ACMSG_GUILD_EMBLEM_REQUEST",                     &AddonIO::HandleGuildEmblemRequest                 },
    { "ACMSG_GUILD_TEAM",                               &AddonIO::HandleGuildTeamRequest                   },
    { "ACMSG_GF_BROWSE",                                &AddonIO::HandleGuildFinderBrowse                  },
    { "ACMSG_GF_GET_APPLICATIONS",                      &AddonIO::HandleGuildFinderGetApplications         },
    { "ACMSG_GF_ADD_RECRUIT",                           &AddonIO::HandleGuildFinderAddRecruit              },
    { "ACMSG_GF_REMOVE_RECRUIT",                        &AddonIO::HandleGuildFinderRemoveRecruit           },
    { "ACMSG_GF_GET_RECRUITS",                          &AddonIO::HandleGuildFinderGetRecruits             },
    { "ACMSG_GF_DECLINE_RECRUIT",                       &AddonIO::HandleGuildFinderDeclineRecruit          },
    { "ACMSG_GF_POST_REQUEST",                          &AddonIO::HandleGuildFinderPostRequest             },
    { "ACMSG_GF_SET_GUILD_POST",                        &AddonIO::HandleGuildFinderSetGuildPost            },
    { "ACMSG_PROMOCODE_REWARD",                         &AddonIO::HandlePromoCodeRewardRequest             },
    { "ACMSG_PROMOCODE_SUBMIT",                         &AddonIO::HandlePromoCodeSubmitRequest             }

};

/*********SHOPSERVICE*************/
enum STORE_ENUM{
    STORE_PREMIUM_BUY_1         = 1,
    STORE_PREMIUM_BUY_2         = 2,
    STORE_PREMIUM_BUY_3         = 3,
    STORE_PREMIUM_BUY_4         = 4,

    COST_STORE_PREMIUM_BUY_1    = 1,
    COST_STORE_PREMIUM_BUY_2    = 6,
    COST_STORE_PREMIUM_BUY_3    = 12,
    COST_STORE_PREMIUM_BUY_4    = 25,

    TIME_STORE_PREMIUM_BUY_1    = 86400,
    TIME_STORE_PREMIUM_BUY_2    = 604800,
    TIME_STORE_PREMIUM_BUY_3    = 1209600,
    TIME_STORE_PREMIUM_BUY_4    = 2592000,
};

enum PAID_SERVICE {
    PAID_SERVICE_NAME_CHANGE            = 1000000,
    PAID_SERVICE_FACTION_CHANGE         = 1000001,
    PAID_SERVICE_RACE_CHANGE            = 1000002,
    PAID_SERVICE_GUILDNAME_CHANGE       = 1000003,
    PAID_SERVICE_GOLD_BUY               = 1000004,
    PAID_SERVICE_ALCHEMY_LEARH          = 1000005,
    PAID_SERVICE_BLACKSMITHING_LEARH    = 1000006,
    PAID_SERVICE_ENCHANTING_LEARN       = 1000007,
    PAID_SERVICE_ENGINEERIN_LEARN       = 1000008,
    PAID_SERVICE_JEWELCRAFTING_LEARN    = 1000009,
    PAID_SERVICE_HERBALISM_LEARN        = 1000010,
    PAID_SERVICE_LEATHERWORKING_LEARN   = 1000011,
    PAID_SERVICE_MINING_LEARN           = 1000012,
    PAID_SERVICE_SKINNING_LEARN         = 1000013,
    PAID_SERVICE_TAILORING_LEARN        = 1000014,
    PAID_SERVICE_FISHING_LEARH          = 1000015,
    PAID_SERVICE_INSCRIPTION_LEARN      = 1000016,
    PAID_SERVICE_COOKING_LEARN          = 1000017,
    PAID_SERVICE_FIRST_AID_LEARN        = 1000018,
    PAID_SERVICE_LEVELUP                = 1000020,
    //MMOTOP
    PAID_SERVICE_PREMIUM_ONE_DAY        = 1000019,
};

static bool IsPaidServiceEntry(uint32 itemEntry)
{
    switch (itemEntry)
    {
        case PAID_SERVICE_NAME_CHANGE:
        case PAID_SERVICE_FACTION_CHANGE:
        case PAID_SERVICE_RACE_CHANGE:
        case PAID_SERVICE_GUILDNAME_CHANGE:
        case PAID_SERVICE_GOLD_BUY:
        case PAID_SERVICE_ALCHEMY_LEARH:
        case PAID_SERVICE_BLACKSMITHING_LEARH:
        case PAID_SERVICE_ENCHANTING_LEARN:
        case PAID_SERVICE_ENGINEERIN_LEARN:
        case PAID_SERVICE_JEWELCRAFTING_LEARN:
        case PAID_SERVICE_HERBALISM_LEARN:
        case PAID_SERVICE_LEATHERWORKING_LEARN:
        case PAID_SERVICE_MINING_LEARN:
        case PAID_SERVICE_SKINNING_LEARN:
        case PAID_SERVICE_TAILORING_LEARN:
        case PAID_SERVICE_FISHING_LEARH:
        case PAID_SERVICE_INSCRIPTION_LEARN:
        case PAID_SERVICE_COOKING_LEARN:
        case PAID_SERVICE_FIRST_AID_LEARN:
        case PAID_SERVICE_PREMIUM_ONE_DAY:
        case PAID_SERVICE_LEVELUP:
            return true;
        default:
            return false;
    }
}

#define ARMORY_CATEGORYID               4

uint8 ShopProfesstionResponse(Player* pl, SkillType skill)
{
    if (pl->GetLevel() < DEFAULT_MAX_LEVEL)
        return 7; //ERROR_LOW_LEVEL
    else if (pl->PlayerAlreadyHasTwoProfessions(pl) && !pl->IsSecondarySkill(skill))
        return 6; //ERROR_PROF_TWO_IS_EXIST
    else
    {
        if (pl->LearnAllRecipesInProfession(pl, skill))
            return 0; //OK
    }

    return 1; //ERROR
}

bool ShopSendItem(Player* m_sender, Player* m_receiver, std::string m_text, uint32 item, uint32 count)
{
    Player* receiver = m_receiver;
    ObjectGuid receiverGuid = receiver->GetGUID();

    std::string subject = "Refund";
    std::string text = m_text;

    typedef std::pair<uint32, uint32> ItemPair;
    typedef std::list< ItemPair > ItemPairs;
    ItemPairs items;

    uint32 itemId = item;

    ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(itemId);
    if (!item_proto)
        return false;

    uint32 itemCount = count;
    if (itemCount < 1 || (item_proto->MaxCount > 0 && itemCount > uint32(item_proto->MaxCount)))
        return false;


    while (itemCount > item_proto->GetMaxStackSize())
    {
        items.push_back(ItemPair(itemId, item_proto->GetMaxStackSize()));
        itemCount -= item_proto->GetMaxStackSize();
    }

    items.push_back(ItemPair(itemId, itemCount));

    if (items.size() > MAX_MAIL_ITEMS)
        return false;


    MailSender sender(MAIL_NORMAL, m_sender->GetSession() ? m_sender->GetSession()->GetPlayer()->GetGUID() : 0, MAIL_STATIONERY_GM);


    MailDraft draft(subject, text);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
    {
        if (Item* item = Item::CreateItem(itr->first, itr->second, m_sender->GetSession() ? m_sender->GetSession()->GetPlayer() : 0))
        {
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
    }

    // draft.SendMailTo(trans, MailReceiver(receiver, GUID_LOPART(receiverGuid)), sender);
    draft.SendMailTo(trans, MailReceiver(receiver), sender);
    CharacterDatabase.CommitTransaction(trans);

    return true;
}

uint8 ShopAddItem(Player* player, Player* receiver, uint32 itemId, uint32 count, uint8 moneyID, uint32 cost, std::string text = "")
{
    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
    if (!itemTemplate)
        return 4;

    if (moneyID == 10)
        player->ModifyMoney(-cost);
    else if (!player->GetSession()->SetAccountCurrency(cost, moneyID, false))
        return 1; //UNKNOWN_ERROR
    else if (moneyID == 1)
        player->GetSession()->WritePurchaseToLogs(player->GetSession(), "BUY ITEM", itemId, count, cost, uint32(time(nullptr)));

    uint32 noSpaceForCount = 0;

    ItemPosCountVec dest;
    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)
        count -= noSpaceForCount;

    Item* item = player->StoreNewItem(dest, itemId, true);

    if (player)
    {
        for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if (Item* item1 = player->GetItemByPos(itr->pos))
                item1->SetBinding(false);
    }

    if (count > 0 && item)
    {
        if (text == "")
            player->SendNewItem(item, count, false, true);
        else
            ShopSendItem(player, receiver, text, itemId, count);
    }


    if (noSpaceForCount > 0)
        ShopSendItem(player, player, "Return of lost items", itemId, noSpaceForCount);

    return 0;
}

uint8 ShopPaidService(Player* player, uint32 itemId, uint32 count, uint8 moneyID, uint32 cost, bool isProfession)
{
    auto sess = player->GetSession();
    uint8 p_resp = 0;

    if (!player->GetSession()->SetAccountCurrency(cost, moneyID, isProfession))
        p_resp = 1;
    else
    {
        uint32 atLoginFlag = 0;
        bool isRaceFaction = false;

        switch (itemId)
        {
        case PAID_SERVICE_NAME_CHANGE:
        {
            atLoginFlag = AT_LOGIN_RENAME;
            player->SetAtLoginFlag(AT_LOGIN_RENAME);
            isRaceFaction = true;
            player->GetSession()->WritePurchaseToLogs(sess, "NAME_CHANGE", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_FACTION_CHANGE:
        {
            atLoginFlag = AT_LOGIN_CHANGE_FACTION;
            player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
            isRaceFaction = true;
            player->GetSession()->WritePurchaseToLogs(sess, "FACTION_CHANGE", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_RACE_CHANGE:
        {
            atLoginFlag = AT_LOGIN_CHANGE_RACE;
            player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
            isRaceFaction = true;
            player->GetSession()->WritePurchaseToLogs(sess, "RACE_CHANGE", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_GUILDNAME_CHANGE:
            break;
        case PAID_SERVICE_GOLD_BUY:
        {
            player->ModifyMoney(count * GOLD * 1000);
            player->GetSession()->WritePurchaseToLogs(sess, "GOLD_BUY", 0, count, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_ALCHEMY_LEARH:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_ALCHEMY);

            if (p_resp == 0)
            {
                player->GetSession()->WritePurchaseToLogs(sess, "ALCHEMY_LEARN", 0, 0, cost, uint32(time(nullptr)));
                
                player->removeSpell(28675, SPEC_MASK_ALL, false);
                player->removeSpell(28677, SPEC_MASK_ALL, false);
                player->removeSpell(28672, SPEC_MASK_ALL, false);

            }

            break;
        }
        case PAID_SERVICE_BLACKSMITHING_LEARH:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_BLACKSMITHING);

            if (p_resp == 0)
            {
                player->GetSession()->WritePurchaseToLogs(sess, "BLACKSMITHING_LEARN", 0, 0, cost, uint32(time(nullptr)));
                player->removeSpell(9787, SPEC_MASK_ALL, false);
                player->removeSpell(17041, SPEC_MASK_ALL, false);
                player->removeSpell(17040, SPEC_MASK_ALL, false);
                player->removeSpell(17039, SPEC_MASK_ALL, false);
                player->removeSpell(9788, SPEC_MASK_ALL, false);
            }
            

            break;
        }
        case PAID_SERVICE_ENCHANTING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_ENCHANTING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "ENCHANTING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_ENGINEERIN_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_ENGINEERING);

            if (p_resp == 0)
            {
                player->GetSession()->WritePurchaseToLogs(sess, "ENGINEERING_LEARN", 0, 0, cost, uint32(time(nullptr)));

                player->removeSpell(20222, SPEC_MASK_ALL, false);
                player->removeSpell(20219, SPEC_MASK_ALL, false);
            }

            break;
        }
        case PAID_SERVICE_JEWELCRAFTING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_JEWELCRAFTING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "JEWELCRAFTING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_HERBALISM_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_HERBALISM);

            if (p_resp == 0)
            {
                player->GetSession()->WritePurchaseToLogs(sess, "HERBALISM_LEARN", 0, 0, cost, uint32(time(nullptr)));

                player->removeSpell(2369, SPEC_MASK_ALL, false);
                player->removeSpell(2371, SPEC_MASK_ALL, false);
            }

            break;
        }
        case PAID_SERVICE_LEATHERWORKING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_LEATHERWORKING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "LEATHERWORKING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_MINING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_MINING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "MINING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_SKINNING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_SKINNING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "SKINNING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_TAILORING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_TAILORING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "TAILORING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_FISHING_LEARH:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_FISHING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "FISHING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_INSCRIPTION_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_INSCRIPTION);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "INSCRIPTION_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_FIRST_AID_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_FIRST_AID);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "FIRST_AID_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_COOKING_LEARN:
        {
            p_resp = ShopProfesstionResponse(player, SKILL_COOKING);

            if (p_resp == 0)
                player->GetSession()->WritePurchaseToLogs(sess, "COOKING_LEARN", 0, 0, cost, uint32(time(nullptr)));

            break;
        }
        case PAID_SERVICE_PREMIUM_ONE_DAY:
            //player->GetSession()->SetAccountPremium(TIME_STORE_PREMIUM_BUY_1);
            break;
        case PAID_SERVICE_LEVELUP:
            player->GiveLevel(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
            break;
        default:
            break;
        }

        if (isRaceFaction)
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->SetData(0, uint16(atLoginFlag));
            stmt->SetData(1, player->GetGUID().GetCounter());
            CharacterDatabase.Execute(stmt);
        }

        if (isProfession && p_resp == 0)
            player->GetSession()->SetAccountCurrency(cost, moneyID, false);
    }

    return p_resp;
}
/******************************/

AddonIO::AddonIO() { }
AddonIO::~AddonIO() { }

AddonIO* AddonIO::instance()
{
	static AddonIO instance;
	return &instance;
}

void AddonIO::HandleMessage(Player* player, std::string message)
{
    if (!player)
        return;

    std::vector<std::string> args;
    boost::split(args, message, boost::is_any_of("	"));

    if (args.size() < 2)
        return;

    auto itr = addonMessagesTable.find(args[0]);
    if (itr == addonMessagesTable.end())
        return;

    // Hardcore: shop unavailable until max level
    if (player->IsHardcore() && player->GetLevel() < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        char const* shopPrefixes[] = {
            "ACMSG_SHOP_BALANCE_REQUEST", "ACMSG_PREMIUM_INFO_REQUEST", "ACMSG_PREMIUM_RENEW_REQUEST",
            "ACMSG_SHOP_ITEM_LIST_REQUEST", "ACMSG_SHOP_VERSION", "ACMSG_SHOP_BUY_ITEM",
            "ACMSG_SHOP_SPECIAL_OFFER_LIST_REQUEST", "ACMSG_SHOP_COLLECTION_LOAD_REQUEST", "ACMSG_SHOP_ITEM_COUNT"
        };
        for (char const* p : shopPrefixes)
            if (args[0] == p)
                return;
    }

    std::string body = args[1];
    for (size_t i = 2; i < args.size(); ++i)
    {
        body.push_back('	');
        body.append(args[i]);
    }

    (this->*itr->second)(player, body);
}

void AddonIO::HandleHardcoreCreateSet(Player* player, std::string body)
{
    if (!player)
        return;

    if (body != "0" && body != "1")
        return;

    // Accept only for level-1 characters (AT_LOGIN_FIRST may already be cleared)
    if (player->GetLevel() != 1)
        return;

    uint8 hardcore = (body == "1") ? 1 : 0;
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_HARDCORE);
    stmt->SetData(0, hardcore);
    stmt->SetData(1, player->GetGUID().GetCounter());
    CharacterDatabase.Execute(stmt);
    player->SetHardcore(hardcore != 0);
    LOG_INFO("module", "Hardcore create set: {} hardcore={}", player->GetName(), hardcore);
}

void AddonIO::BroadcastHardcoreDeath(std::string const& payload)
{
    if (payload.empty())
        return;

    std::string message = std::string(ASMSG_HARDCORE_DEATH) + "\t" + payload;
    sWorldSessionMgr->DoForAllOnlinePlayers([&message](Player* receiver)
    {
        if (!receiver || !receiver->GetSession())
            return;
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, ObjectGuid::Empty, receiver->GetGUID(), message, 0);
        receiver->GetSession()->SendPacket(&data);
    });
}

void AddonIO::RemoveGuildFinderApplicationsForPlayer(ObjectGuid playerGuid)
{
    GF_RemoveAllApplicationsForPlayer(playerGuid);
}

/*****************************
********* HANDLERS ***********
******************************/

void AddonIO::HandleTransmogrificationInfoRequest(Player* player, std::string body)
{
    if (body.empty())
        return;
    
    if (!player)
        return;

    ObjectGuid guid = ObjectGuid::Empty;

    try
    {
        uint64 value = std::stoull(body, nullptr, 16);
        guid = ObjectGuid(value);
    }
    catch (std::exception /*ex*/)
    {
        return;
    }

    Player* target = ObjectAccessor::GetPlayer(*player, guid);
    if (!target)
    {
        //TC_LOG_ERROR("network", "ACMSG_TRANSMOGRIFICATION_INFO_REQUEST: No target found from %s", guid.ToString().c_str());
        return;
    }

    std::stringstream response;
    response << "ASMSG_TRANSMOGRIFICATION_INFO_RESPONSE\t" << guid.GetRawValue() << ";" << sTransmogrificationMgr->GenerateTransmogrificationInfoFor(target);
}

void AddonIO::HandleTransmogrificationPrepareRequest(Player* player, std::string body)
{
    if (body.empty())
        return;
    
    if (!player)
        return;

    uint64 guid = player->GetCurrentTransmogrifier();
    /*if (!guid)
        return;*/

    std::vector<std::string> args; // 0 - slot, 1 - transEntry
    boost::split(args, body, boost::is_any_of(":"));

    if (args.size() != 2)
    {
        //TC_LOG_ERROR("network", "ACMSG_TRANSMOGRIFICATION_PREPARE_REQUEST: Received invalid parameters!");
        return;
    }

    uint8 slot = 0;
    uint32 transEntry = 0;

    try
    {
        slot = std::stoi(args[0]) - 1;
        transEntry = std::stoi(args[1]);

        if (slot < EQUIPMENT_SLOT_START || slot > EQUIPMENT_SLOT_END)
            throw std::exception();
    }
    catch (std::exception /*ex*/)
    {
        //TC_LOG_ERROR("network", "ACMSG_TRANSMOGRIFICATION_PREPARE_REQUEST: Received invalid parameters!");
        return;
    }

    sTransmogrificationMgr->HandleTransmogrificationPrepareRequestFrom(player, slot, transEntry);
}

void AddonIO::HandleTransmogrificationRemove(Player* player, std::string body)
{
    if (body.empty())
        return;
    
    if (!player)
        return;

    //uint64 guid = player->GetCurrentTransmogrifier();
    /*if (!guid)
        return;*/

    /*if (player->GetNPCIfCanInteractWith(guid, 0) == nullptr)
        return;*/

    uint8 slot = 0;

    try
    {
        slot = std::stoi(body) - 1;
        if (slot < EQUIPMENT_SLOT_START || slot > EQUIPMENT_SLOT_END)
            throw std::exception();
    }
    catch (std::exception /*ex*/)
    {
        //TC_LOG_ERROR("network", "ACMSG_TRANSMOGRIFICATION_REMOVE: Received invalid parameters!");
        return;
    }

    sTransmogrificationMgr->HandleTransmogrificationRemoveRequestFrom(player, slot);
}

void AddonIO::HandleTransmogrificationApply(Player* player, std::string body)
{
    if (body.empty())
        return;
    
    if (!player)
        return;

    //uint64 guid = player->GetCurrentTransmogrifier();
    /*if (!guid)
        return;*/

    /*if (player->GetNPCIfCanInteractWith(guid, 0) == nullptr)
        return;*/

    std::map<uint8, uint32> data;

    std::vector<std::string> slotEntryPairs;
    boost::split(slotEntryPairs, body, boost::is_any_of(";"));

    if (slotEntryPairs.empty())
        return;

    try
    {
        for (std::vector<std::string>::const_iterator itr = slotEntryPairs.begin(); itr != slotEntryPairs.end(); ++itr)
        {
            std::vector<std::string>  pair;
            boost::split(pair, *itr, boost::is_any_of(":"));
            if (pair.size() != 2)
                continue;

            uint8 slot = std::stoi(pair[0]) - 1;
            uint32 transEntry = std::stoi(pair[1]);

            if (slot < EQUIPMENT_SLOT_START || slot > EQUIPMENT_SLOT_END)
                throw std::exception();

            data.emplace(slot, transEntry);
        }
    }
    catch (std::exception /*ex*/)
    {
        //TC_LOG_ERROR("network", "ACMSG_TRANSMOGRIFICATION_APPLY: Received invalid parameters!");
        return;
    }
 
    sTransmogrificationMgr->HandleTransmogrificationApplyRequestFrom(player, data);
}

void AddonIO::HandleGuildSpellsRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    // Always notify client (even empty payload) so UI can reload perk list vs spellbook tabs.
    std::string response = "ASMSG_GUILD_SPELLS_RESPONSE\t";
    for (auto const& pair : sGuildPerkSpellsStore)
        response += std::to_string(pair.second) + ":" + std::to_string(pair.first) + ",";

    player->SendAddonMessage(response.c_str());
}

void AddonIO::HandleGuildLevelRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    if (Guild* guild = player->GetGuild())
    {
        uint8 lvl = guild->GetLevel() == GUILD_MAX_LEVEL ? (GUILD_MAX_LEVEL - 1) : guild->GetLevel();
        uint32 xp_for_old_lvl = lvl != 0 ? sWorld->GetXpForNextLevel(lvl - 1) : 0;
        uint32 xp_for_next_lvl = sWorld->GetXpForNextLevel(lvl);
        uint32 totalxp = xp_for_next_lvl - xp_for_old_lvl;
        uint32 guildXP = guild->GetCurrentXP();
        // Saturate: current XP can be below the level floor after GM setlevel / reparenting
        uint32 xp = guildXP >= xp_for_old_lvl ? (guildXP - xp_for_old_lvl) : 0;
        if (totalxp > 0 && xp > totalxp)
            xp = totalxp;
        uint32 dailyCap = sWorld->getIntConfig(CONFIG_GUILD_DAILY_XP_CAP);

        player->SendAddonMessage("ASMSG_GUILD_LEVEL_INFO\t{}:{}:{}:{}:{}", guild->GetLevel(), xp, totalxp, guild->GetGuildTodayXP(), dailyCap);
    }
}

void AddonIO::HandleGuildOnlineRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    if (Guild* guild = player->GetGuild())
        player->SendAddonMessage("ASMSG_GUILD_PLAYERS_COUNT\t{}:{}", guild->GetOnlineMembers(), guild->GetMemberCount());
}

void AddonIO::HandleGuildIlvlsRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    if (Guild* guild = player->GetGuild())
    {
        std::string response = "ASMSG_GUILD_PLAYERS_ILVL\t";
        std::unordered_map<uint32, Guild::Member> members = guild->GetMembers();
        for (const auto& itr : members)
            response += itr.second.GetName() + ":" + std::to_string(itr.second.GetAverageLvl()) + "|";

        player->SendAddonMessage(response.c_str());
    }
}

void AddonIO::HandleGuildEmblemRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    if (Guild* guild = player->GetGuild())
    {
        EmblemInfo emblem = guild->GetEmblemInfo();
        player->SendAddonMessage("ASMSG_PLAYER_GUILD_EMBLEM_INFO\t{}:{}:{}:{}:{}", emblem.GetStyle(), emblem.GetColor(), emblem.GetBorderStyle(), emblem.GetBorderColor(), emblem.GetBackgroundColor());
    }
}

void AddonIO::HandleGuildTeamRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    uint8 factionIcon = 1;
    switch (player->getRace())
    {
    case RACE_DRAENEI:
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_DWARF:
    case RACE_NIGHTELF:
        //case RACE_VOIDELF:
        factionIcon = 0;
        [[fallthrough]];
    default:
        break;
    }

    player->SendAddonMessage("ASMSG_GUILD_TEAM\t{}", factionIcon);
}

void AddonIO::HandleGuildFinderBrowse(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    uint32 const playerGuildId = player->GetGuildId();
    std::vector<std::string> rows;
    rows.reserve(64);
    constexpr size_t maxRows = 100;

    sGuildMgr->ForEachGuild([&](Guild* guild)
    {
        if (rows.size() >= maxRows || !guild || guild->GetMemberCount() == 0)
            return;

        if (guild->GetId() == playerGuildId && playerGuildId != 0)
            return;

        EmblemInfo const emblem = guild->GetEmblemInfo();

        std::string comment = GF_EscapeGuildFinderField(guild->GetMOTD());
        if (comment.empty())
            comment = GF_EscapeGuildFinderField(guild->GetInfo());
        if (comment.empty())
            comment.assign(" ");

        std::string name = GF_EscapeGuildFinderField(guild->GetName());
        if (name.empty())
            return;

        if (comment.size() > 240)
            comment.resize(240);

        std::ostringstream line;
        line << guild->GetId() << '|'
             << emblem.GetStyle() << ':' << emblem.GetColor() << ':' << emblem.GetBorderStyle() << ':'
             << emblem.GetBorderColor() << ':' << emblem.GetBackgroundColor() << '|'
             << comment << '|'
             << "31" << '|'
             << static_cast<uint32>(guild->GetLevel()) << '|'
             << name << '|'
             << "false" << '|'
             << "3" << '|'
             << "7" << '|'
             << guild->GetMemberCount();

        rows.push_back(line.str());
    });

    player->SendAddonMessage("ASMSG_GF_BROWSE_UPDATED\t{}", static_cast<unsigned>(rows.size()));

    for (std::string const& row : rows)
        player->SendAddonMessage(std::string("ASMSG_GF_BROWSE_UPDATE\t").append(row));
}

void AddonIO::HandleGuildFinderGetApplications(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    GF_PruneExpiredApplications();

    ObjectGuid const pguid = player->GetGUID();
    std::vector<GuildFinderPendingApplication const*> mine;
    mine.reserve(8);

    for (GuildFinderPendingApplication const& a : GF_Applications)
        if (a.PlayerGuid == pguid)
            mine.push_back(&a);

    uint32 const remaining = (mine.size() >= GF_MAX_PENDING_APPLICATIONS_PER_PLAYER)
        ? 0u
        : (GF_MAX_PENDING_APPLICATIONS_PER_PLAYER - static_cast<uint32>(mine.size()));

    player->SendAddonMessage("ASMSG_GF_MEMBERSHIP_LIST_UPDATED\t{}|{}",
        static_cast<unsigned>(mine.size()), static_cast<unsigned>(remaining));

    time_t const now = GameTime::GetGameTime().count();

    for (GuildFinderPendingApplication const* app : mine)
    {
        Guild const* guild = sGuildMgr->GetGuildById(app->GuildId);

        uint32 const timeSince = static_cast<uint32>(std::max<time_t>(0, now - app->CreatedAt));
        uint32 const timeLeft = (GF_APPLICATION_MAX_AGE_SEC > timeSince)
            ? (GF_APPLICATION_MAX_AGE_SEC - timeSince)
            : 0u;

        std::string row = Acore::StringFormat("{}|", app->GuildId);
        row += GF_EscapeGuildFinderField(app->Comment);
        row.push_back('|');
        row += GF_EscapeGuildFinderField(guild ? guild->GetName() : "[disbanded]");
        row += Acore::StringFormat("|{}|{}|{}|{}|{}",
            app->Availability,
            timeLeft,
            app->ClassRoles,
            timeSince,
            app->Interests);

        player->SendAddonMessage(std::string("ASMSG_GF_MEMBERSHIP_LIST_UPDATE\t").append(row));
    }
}

void AddonIO::HandleGuildFinderAddRecruit(Player* player, std::string body)
{
    if (!player || player->GetGuildId())
        return;

    GF_PruneExpiredApplications();

    std::vector<std::string_view> parts = Acore::Tokenize(body, '|', true);
    if (parts.size() < 5)
        return;

    uint32 classRoles = 0;
    uint32 interests = 0;
    uint32 availability = 0;
    uint32 guildId = 0;

    try
    {
        classRoles = std::stoul(std::string(parts[0]));
        interests = std::stoul(std::string(parts[1]));
        availability = std::stoul(std::string(parts[2]));
        guildId = std::stoul(std::string(parts[3]));
    }
    catch (...)
    {
        return;
    }

    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return;

    ObjectGuid const pguid = player->GetGUID();
    if (GF_HasPendingApplication(pguid, guildId))
        return;

    if (GF_CountPlayerApplications(pguid) >= GF_MAX_PENDING_APPLICATIONS_PER_PLAYER)
        return;

    std::string comment;
    for (size_t i = 4; i < parts.size(); ++i)
    {
        if (i > 4)
            comment.push_back('|');
        comment.append(parts[i]);
    }

    GuildFinderPendingApplication app;
    app.PlayerGuid = pguid;
    app.GuildId = guildId;
    app.ClassRoles = classRoles;
    app.Interests = interests;
    app.Availability = availability;
    app.Comment = GF_EscapeGuildFinderField(comment);
    app.PlayerName = player->GetName();
    app.Level = player->GetLevel();
    app.ClassId = static_cast<uint8>(player->getClass());
    app.CreatedAt = GameTime::GetGameTime().count();

    GF_Applications.push_back(std::move(app));

    GF_NotifyApplicantsChanged(player);
    GF_NotifyGuildApplicantsUpdated(guild);
}

void AddonIO::HandleGuildFinderRemoveRecruit(Player* player, std::string body)
{
    if (!player)
        return;

    GF_PruneExpiredApplications();

    uint32 guildId = 0;
    try
    {
        guildId = std::stoul(body);
    }
    catch (...)
    {
        return;
    }

    ObjectGuid const pguid = player->GetGUID();

    auto itr = std::remove_if(GF_Applications.begin(), GF_Applications.end(),
        [&](GuildFinderPendingApplication const& a)
        {
            return a.PlayerGuid == pguid && a.GuildId == guildId;
        });

    if (itr == GF_Applications.end())
        return;

    GF_Applications.erase(itr, GF_Applications.end());

    GF_NotifyApplicantsChanged(player);

    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        GF_NotifyGuildApplicantsUpdated(guild);
}

void AddonIO::HandleGuildFinderGetRecruits(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    GF_PruneExpiredApplications();

    Guild* guild = player->GetGuild();
    if (!guild || !guild->HasRankRight(player, GR_RIGHT_INVITE))
    {
        player->SendAddonMessage("ASMSG_GF_RECRUIT_LIST_UPDATED\t0");
        return;
    }

    uint32 const gid = guild->GetId();

    std::vector<GuildFinderPendingApplication const*> ours;
    ours.reserve(16);

    for (GuildFinderPendingApplication const& a : GF_Applications)
        if (a.GuildId == gid)
            ours.push_back(&a);

    player->SendAddonMessage("ASMSG_GF_RECRUIT_LIST_UPDATED\t{}", static_cast<unsigned>(ours.size()));

    time_t const now = GameTime::GetGameTime().count();

    for (GuildFinderPendingApplication const* app : ours)
    {
        uint32 const timeSince = static_cast<uint32>(std::max<time_t>(0, now - app->CreatedAt));
        uint32 const timeLeft = (GF_APPLICATION_MAX_AGE_SEC > timeSince)
            ? (GF_APPLICATION_MAX_AGE_SEC - timeSince)
            : 0u;

        std::string row = Acore::StringFormat("{}|{}|{}|{}|{}|{}|{}|{}|",
            app->PlayerGuid.GetCounter(),
            0u,
            static_cast<uint32>(app->Level),
            timeSince,
            app->Availability,
            app->ClassRoles,
            app->Interests,
            timeLeft);

        row += GF_EscapeGuildFinderField(app->PlayerName);
        row.push_back('|');
        row += GF_EscapeGuildFinderField(app->Comment);
        row += Acore::StringFormat("|{}", static_cast<uint32>(app->ClassId));

        player->SendAddonMessage(std::string("ASMSG_GF_RECRUIT_LIST_UPDATE\t").append(row));
    }
}

void AddonIO::HandleGuildFinderDeclineRecruit(Player* player, std::string body)
{
    if (!player)
        return;

    GF_PruneExpiredApplications();

    uint32 applicantLow = 0;
    try
    {
        applicantLow = std::stoul(body);
    }
    catch (...)
    {
        return;
    }

    Guild* guild = player->GetGuild();
    if (!guild || !guild->HasRankRight(player, GR_RIGHT_INVITE))
        return;

    uint32 const gid = guild->GetId();

    auto itr = std::find_if(GF_Applications.begin(), GF_Applications.end(),
        [&](GuildFinderPendingApplication const& a)
        {
            return a.GuildId == gid && a.PlayerGuid.GetCounter() == applicantLow;
        });

    if (itr == GF_Applications.end())
        return;

    ObjectGuid const victimGuid = itr->PlayerGuid;
    GF_Applications.erase(itr);

    if (Player* victim = ObjectAccessor::FindConnectedPlayer(victimGuid))
        GF_NotifyApplicantsChanged(victim);

    GF_NotifyGuildApplicantsUpdated(guild);
}

void AddonIO::HandleGuildFinderPostRequest(Player* player, std::string /*body*/)
{
    if (!player)
        return;

    Guild* guild = player->GetGuild();
    if (!guild)
        return;

    GF_SendPostUpdated(player, guild);
}

void AddonIO::HandleGuildFinderSetGuildPost(Player* player, std::string body)
{
    if (!player)
        return;

    Guild* guild = player->GetGuild();
    if (!guild || guild->GetLeaderGUID() != player->GetGUID())
        return;

    std::vector<std::string_view> parts = Acore::Tokenize(body, '|', true);
    if (parts.size() < 6)
        return;

    GuildFinderGuildListing listing = GF_GetOrCreateListing(guild);

    try
    {
        listing.LevelMode = static_cast<uint8>(std::stoul(std::string(parts[0])));
        listing.ClassRoles = std::stoul(std::string(parts[1]));
        listing.Availability = std::stoul(std::string(parts[2]));
        listing.Interests = std::stoul(std::string(parts[3]));
        listing.Listed = (std::stoul(std::string(parts[4])) != 0);
    }
    catch (...)
    {
        return;
    }

    listing.Comment.clear();
    for (size_t i = 5; i < parts.size(); ++i)
    {
        if (i > 5)
            listing.Comment.push_back('|');
        listing.Comment.append(parts[i]);
    }

    listing.Comment = GF_EscapeGuildFinderField(listing.Comment);

    GF_GuildListings[guild->GetId()] = listing;

    GF_SendPostUpdated(player, guild);

void AddonIO::HandleAverageItemLevelRequest(Player* player, std::string body)
{
    if (!player)
        return;
    
    if (body.empty())
        return;

    ObjectGuid guid = ObjectGuid::Empty;

    try
    {
        uint64 value = std::stoull(body, nullptr, 16);
        guid = ObjectGuid(value);
    }
    catch (std::exception /*ex*/)
    {
        //TC_LOG_ERROR("network", "ACMSG_AVERAGE_ITEM_LEVEL_REQUEST: Received invalid parameters!");
        player->SendAddonMessage("ASMSG_AVERAGE_ITEM_LEVEL_RESPONSE\t-1");
        return;
    }

    Player* target = ObjectAccessor::FindPlayer(guid);
    if (!target || player->IsValidAttackTarget(target))
    {
        player->SendAddonMessage("ASMSG_AVERAGE_ITEM_LEVEL_RESPONSE\t-1");
        return;
    }

    player->SendAddonMessage("ASMSG_AVERAGE_ITEM_LEVEL_RESPONSE\t{}", static_cast<uint32>(std::floor(target->GetAverageItemLevel())));
}

void AddonIO::HandleShopBalanceRequest(Player* player, std::string body)
{
    if (!player)
        return;

    auto sess = player->GetSession();

    player->SendAddonMessage("ASMSG_SHOP_BALANCE_RESPONSE\t{}:{}:{}:{}:{}:{}:{}", sess->GetAccountBalance(), sess->GetAccountVote(), 0, 0, 0, 0, 0);
}

void AddonIO::HandleShopItemListRequest(Player* player, std::string body)
{
    if (!player)
        return;

    if (!sWorld->getBoolConfig(CONFIG_SHOP_ENABLE))
        return;

    auto store = sWorld->GetStoreItem();
    auto specialOffers = sWorld->GetStoreSpecialOffer();

    std::unordered_set<uint32> specialOfferProductIDs;
    for (auto const& offerData : specialOffers)
    {
        if (offerData.second.productID != 0)
            specialOfferProductIDs.insert(offerData.second.productID);
    }
    
    if (!store.empty())
    {
        for (auto it = store.begin(); it != store.end(); ++it)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(it->second.itemEntry);
            bool isPaidService = IsPaidServiceEntry(it->second.itemEntry);

            // Services use synthetic itemEntry ids and have no item_template row.
            if (!proto && !isPaidService)
                continue;

            if (proto && it->second.CategoryID == ARMORY_CATEGORYID)
            {
                if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE && !player->HasSpell(750))
                    continue;

                if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && !player->HasSpell(8737))
                    continue;

                if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_LEATHER && !player->HasSpell(9077))
                    continue;

                if (!(1 << (player->getClass() - 1) & proto->AllowableClass))
                    continue;

                if (!(1 << (player->getRace() - 1) & proto->AllowableRace))
                    continue;
            }

            uint32 price = it->second.price;
            uint8 discount = ResolveStoreDiscount(price, it->second.discount, it->second.discountPrice);
            uint32 discountPrice = ResolveStoreDiscountPriceForClient(price, discount, it->second.discountPrice);
            uint32 storeFlags = it->second.storeFlags;

            if (discount > 0)
                storeFlags |= STORE_ITEM_FLAG_SPECIAL;

            if (specialOfferProductIDs.find(it->first) != specialOfferProductIDs.end())
                storeFlags |= STORE_ITEM_FLAG_SPECIAL;

            player->SendAddonMessage("ASMSG_SHOP_ITEM\t{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}", it->first, it->second.itemEntry, it->second.count, price, discount, discountPrice, it->second.creatureEntry, storeFlags, it->second.CategoryID, it->second.SubCategoryID, it->second.MoneyID);
        }

    }
    
}

void AddonIO::HandleShopVersionRequest(Player* player, std::string body)
{
    if (!player)
        return;

    player->SendAddonMessage("ASMSG_SHOP_VERSION\t{}:{}", sWorld->GetShopVersion(), sWorld->getBoolConfig(CONFIG_SHOP_ENABLE) ? 1 : 0);
}

void AddonIO::HandleShopBuyItemRequest(Player* player, std::string body)
{
    if (!player)
        return;

    if (body.empty())
        return;

    if (!sWorld->getBoolConfig(CONFIG_SHOP_ENABLE))
        return;

    auto sess = player->GetSession();

    uint8 p_resp = 1;
    uint32 item = 0, cost = 0, count = 1, db_count = 1, moneyID = 1, f_cost = 0, atLoginFlag = 0;
    int32 balance = 0;

    try
    {
        std::vector<std::string> par;
        boost::split(par, body, boost::is_any_of(":"));

        if (!par.empty())
        {
            auto store = sWorld->GetStoreItem();
            if (!store.empty())
            {
                for (auto it = store.begin(); it != store.end(); ++it)
                    if (it->first == std::stoi(par[0]))
                    {
                        uint8 discount = ResolveStoreDiscount(it->second.price, it->second.discount, it->second.discountPrice);
                        item = it->second.itemEntry;
                        cost = ResolveStorePriceForPurchase(it->second.price, discount, it->second.discountPrice);
                        db_count = it->second.count;
                        moneyID = it->second.MoneyID;
                        if (par.size() != 2)
                            count = std::stoi(par[1]);
                        break;
                    }
            }

            if (moneyID == 1)
                balance = sess->GetAccountBalance();
            else
                balance = sess->GetAccountVote();

            if (db_count > 1 && db_count != count)
                count = db_count;

            if (db_count == 1) f_cost = cost * count;
            else f_cost = cost;

            if (balance > 0)
                if (balance >= f_cost)
                {
                    switch (par.size())
                    {
                    case 4:
                    {
                        switch (item)
                        {
                        case PAID_SERVICE_ALCHEMY_LEARH:
                        case PAID_SERVICE_BLACKSMITHING_LEARH:
                        case PAID_SERVICE_ENCHANTING_LEARN:
                        case PAID_SERVICE_ENGINEERIN_LEARN:
                        case PAID_SERVICE_JEWELCRAFTING_LEARN:
                        case PAID_SERVICE_HERBALISM_LEARN:
                        case PAID_SERVICE_LEATHERWORKING_LEARN:
                        case PAID_SERVICE_MINING_LEARN:
                        case PAID_SERVICE_SKINNING_LEARN:
                        case PAID_SERVICE_TAILORING_LEARN:
                        case PAID_SERVICE_FISHING_LEARH:
                        case PAID_SERVICE_INSCRIPTION_LEARN:
                        case PAID_SERVICE_FIRST_AID_LEARN:
                        case PAID_SERVICE_COOKING_LEARN:
                            p_resp = ShopPaidService(player, item, count, moneyID, f_cost, true);
                            break;
                        case PAID_SERVICE_NAME_CHANGE:
                        case PAID_SERVICE_FACTION_CHANGE:
                        case PAID_SERVICE_RACE_CHANGE:
                        case PAID_SERVICE_GUILDNAME_CHANGE:
                        case PAID_SERVICE_GOLD_BUY:
                        case PAID_SERVICE_PREMIUM_ONE_DAY:
                        case PAID_SERVICE_LEVELUP:
                            p_resp = ShopPaidService(player, item, count, moneyID, f_cost, false);
                            break;
                        default:
                            p_resp = ShopAddItem(player, player, item, count, moneyID, f_cost);
                            break;
                        }

                        break;
                    }
                    case 6:
                    {
                        switch (item)
                        {
                        case PAID_SERVICE_NAME_CHANGE:
                        case PAID_SERVICE_FACTION_CHANGE:
                        case PAID_SERVICE_RACE_CHANGE:
                        case PAID_SERVICE_GUILDNAME_CHANGE:
                        case PAID_SERVICE_GOLD_BUY:
                        case PAID_SERVICE_ALCHEMY_LEARH:
                        case PAID_SERVICE_BLACKSMITHING_LEARH:
                        case PAID_SERVICE_ENCHANTING_LEARN:
                        case PAID_SERVICE_ENGINEERIN_LEARN:
                        case PAID_SERVICE_JEWELCRAFTING_LEARN:
                        case PAID_SERVICE_HERBALISM_LEARN:
                        case PAID_SERVICE_LEATHERWORKING_LEARN:
                        case PAID_SERVICE_MINING_LEARN:
                        case PAID_SERVICE_SKINNING_LEARN:
                        case PAID_SERVICE_TAILORING_LEARN:
                        case PAID_SERVICE_FISHING_LEARH:
                        case PAID_SERVICE_INSCRIPTION_LEARN:
                        case PAID_SERVICE_COOKING_LEARN:
                        case PAID_SERVICE_FIRST_AID_LEARN:
                        case PAID_SERVICE_PREMIUM_ONE_DAY:
                        case PAID_SERVICE_LEVELUP:
                            break;
                        default:
                        {
                            Player* target;
                            std::string targetName;
                            //uint32 parseGUID = MAKE_NEW_GUID(atol(par[3].c_str()), 0, HighGuid::Player);
                            //uint32 parseGUID = ObjectGuid::Create<LowGuid::Player>(atol(par[3].c_str()));
                            ObjectGuid parseGUID = ObjectGuid::Create<HighGuid::Player>(atol(par[3].c_str()));
                            if (sCharacterCache->GetCharacterNameByGuid(parseGUID, targetName))
                                target = ObjectAccessor::FindPlayer(parseGUID);

                            if (target == nullptr)
                            {
                                p_resp = 2; //ERROR_RECEIVER_NOT_FOUND
                                break;
                            }

                            if (player == target)
                            {
                                p_resp = 3; //ERROR_CANNOT_GIFT_TO_SELF
                                break;
                            }

                            p_resp = ShopAddItem(player, target, item, count, moneyID, f_cost, par[5].c_str());

                            break;
                        }
                        }

                        break;
                    }
                    }
                }
        }

        player->SendAddonMessage("ASMSG_SHOP_BUY_ITEM_RESPONSE\t{}:{}", p_resp, item);
    }
    catch (std::exception /*ex*/)
    {
        return;
    }
}

void AddonIO::HandleShopSpecialOfferListRequest(Player* player, std::string body)
{
    if (!player)
        return;

    auto offer = sWorld->GetStoreSpecialOffer();
    auto details = sWorld->GetStoreSpecialDetails();

    if (offer.empty())
        return;

    for (auto const& it : offer)
    {
        std::ostringstream infoResponse;
        infoResponse << "ASMSG_SHOP_SPECIAL_OFFER_INFO\t"
                     << it.first << "|"
                     << it.second.background << "|"
                     << it.second.headline << "|"
                     << it.second.title << "|"
                     << it.second.description << "|"
                     << it.second.time << "|"
                     << it.second.productID << "|"
                     << it.second.itemEntry << "|"
                     << it.second.price << "|0|0|0";
        player->SendAddonMessage(infoResponse.str());

        std::ostringstream detailsItems;
        auto detailsRange = details.equal_range(it.second.details);
        for (auto detailIt = detailsRange.first; detailIt != detailsRange.second; ++detailIt)
        {
            detailsItems << detailIt->second.itemID << "<" << detailIt->second.role << "><" << detailIt->second.count << ">:";
        }

        std::ostringstream detailsResponse;
        detailsResponse << "ASMSG_SHOP_SPECIAL_OFFER_DETAILS\t"
                        << it.first << "|"
                        << it.second.title << "|"
                        << it.second.detailsTitle << "|"
                        << it.second.price << "|"
                        << detailsItems.str();
        player->SendAddonMessage(detailsResponse.str());
    }
}

void AddonIO::HandleShopCollectionLoadRequest(Player* player, std::string body)
{
    if (!player)
        return;

    auto store = sWorld->GetStoreItem();

    if (!store.empty())
        for (auto it = store.begin(); it != store.end(); ++it)
            if (it->second.creatureEntry != 0)
                player->GetSession()->HandleShopCreatureOpcode(it->second.creatureEntry);
}

void AddonIO::HandleShopItemCountRequest(Player* player, std::string body)
{
    if (!player)
        return;

    if (!sWorld->getBoolConfig(CONFIG_SHOP_ENABLE))
        return;

    player->SendAddonMessage("ASMSG_SHOP_ITEM_COUNT\t{}", sWorld->GetStoreItems());
}


void AddonIO::HandlePromoCodeRewardRequest(Player* player, std::string body)
{
    if (!player)
        return;

    if (body.empty())
    {
        LOG_WARN("shop", "HandlePromoCodeRewardRequest: Empty promo code from player {} (Account: {})",
            player->GetName(), player->GetSession()->GetAccountId());
        player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t1");
        return;
    }

    boost::trim(body);
    if (body.empty())
    {
        player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t1");
        return;
    }

    LOG_INFO("shop", "HandlePromoCodeRewardRequest: Player {} (Account: {}) requested rewards for promo code: {}",
        player->GetName(), player->GetSession()->GetAccountId(), body);

    uint32 codeId = 0;
    PromotionCodes const* promoCode = sPromotionCodeMgr->GetPromoCode(body, codeId);
    if (!promoCode)
    {
        LOG_WARN("shop", "HandlePromoCodeRewardRequest: Promo code '{}' not found for player {} (Account: {})",
            body, player->GetName(), player->GetSession()->GetAccountId());
        player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t1");
        return;
    }

    if (promoCode->exist_count == 0)
    {
        LOG_WARN("shop", "HandlePromoCodeRewardRequest: Promo code '{}' has no remaining uses for player {} (Account: {})",
            body, player->GetName(), player->GetSession()->GetAccountId());
        player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t2");
        return;
    }

    if (!sPromotionCodeMgr->CanUsePromoCode(body, player))
    {
        LOG_WARN("shop", "HandlePromoCodeRewardRequest: Player {} (Account: {}) already used promo code '{}'",
            player->GetName(), player->GetSession()->GetAccountId(), body);
        player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t3");
        return;
    }

    std::ostringstream response;
    bool hasRewards = false;

    if (promoCode->item_1 > 0)
    {
        uint32 count = (promoCode->item_count_1 > 1) ? promoCode->item_count_1 : 1;
        response << "0:" << promoCode->item_1 << ":" << count;
        hasRewards = true;
    }

    if (promoCode->item_2 > 0)
    {
        if (hasRewards)
            response << "|";
        uint32 count = (promoCode->item_count_2 > 1) ? promoCode->item_count_2 : 1;
        response << "0:" << promoCode->item_2 << ":" << count;
        hasRewards = true;
    }

    if (promoCode->item_3 > 0)
    {
        if (hasRewards)
            response << "|";
        uint32 count = (promoCode->item_count_3 > 1) ? promoCode->item_count_3 : 1;
        response << "0:" << promoCode->item_3 << ":" << count;
        hasRewards = true;
    }

    if (promoCode->money > 0)
    {
        if (hasRewards)
            response << "|";
        response << "2:2:" << (promoCode->money / 10000);
        hasRewards = true;
    }

    if (promoCode->coin > 0)
    {
        if (hasRewards)
            response << "|";
        response << "1:1:" << promoCode->coin;
        hasRewards = true;
    }

    bool canActivate = sPromotionCodeMgr->CanUsePromoCode(body, player);
    if (!hasRewards)
        response << "0:0:0";

    response << "|" << (canActivate ? 1 : 0);
    player->SendAddonMessage("ASMSG_PROMOCODE_REWARD\t{}", response.str());

    LOG_INFO("shop", "HandlePromoCodeRewardRequest: Sent rewards for promo code '{}' to player {} (Account: {}). Can activate: {}",
        body, player->GetName(), player->GetSession()->GetAccountId(), canActivate ? 1 : 0);
}

void AddonIO::HandlePromoCodeSubmitRequest(Player* player, std::string body)
{
    if (!player)
        return;

    if (body.empty())
    {
        LOG_WARN("shop", "HandlePromoCodeSubmitRequest: Empty promo code from player {} (Account: {})",
            player->GetName(), player->GetSession()->GetAccountId());
        player->SendAddonMessage("ASMSG_PROMOCODE_SUBMIT\t1");
        return;
    }

    boost::trim(body);
    if (body.empty())
    {
        player->SendAddonMessage("ASMSG_PROMOCODE_SUBMIT\t1");
        return;
    }

    LOG_INFO("shop", "HandlePromoCodeSubmitRequest: Player {} (Account: {}) trying to activate promo code: {}",
        player->GetName(), player->GetSession()->GetAccountId(), body);

    bool success = sPromotionCodeMgr->CheckedEnteredCodeByPlayer(body, player, 0);
    if (success)
    {
        uint32 codeId = 0;
        PromotionCodes const* promoCode = sPromotionCodeMgr->GetPromoCode(body, codeId);

        std::string successMessage = "Промокод успешно активирован!";
        if (promoCode)
        {
            std::vector<std::string> rewards;
            if (promoCode->money > 0)
                rewards.push_back(Acore::StringFormat("{} золота", promoCode->money / 10000));
            if (promoCode->coin > 0)
                rewards.push_back(Acore::StringFormat("{} бонусов", promoCode->coin));
            if (promoCode->item_1 > 0 || promoCode->item_2 > 0 || promoCode->item_3 > 0)
                rewards.push_back("предметы");
            if (promoCode->honor > 0)
                rewards.push_back(Acore::StringFormat("{} очков чести", promoCode->honor));
            if (promoCode->arena > 0)
                rewards.push_back(Acore::StringFormat("{} очков арены", promoCode->arena));

            if (!rewards.empty())
            {
                std::string rewardsStr;
                for (size_t i = 0; i < rewards.size(); ++i)
                {
                    if (i > 0)
                    {
                        if (i == rewards.size() - 1)
                            rewardsStr += " и ";
                        else
                            rewardsStr += ", ";
                    }
                    rewardsStr += rewards[i];
                }
                successMessage = Acore::StringFormat("Промокод активирован! Получено: {}.", rewardsStr);
            }
        }

        player->SendAddonMessage("ASMSG_PROMOCODE_SUBMIT\t1:{}", successMessage);
        LOG_INFO("shop", "HandlePromoCodeSubmitRequest: Promo code '{}' successfully activated for player {} (Account: {})",
            body, player->GetName(), player->GetSession()->GetAccountId());
    }
    else
    {
        uint32 codeId = 0;
        PromotionCodes const* promoCode = sPromotionCodeMgr->GetPromoCode(body, codeId);
        uint32 errorId = 1;

        if (!promoCode)
        {
            errorId = 1;
            LOG_WARN("shop", "HandlePromoCodeSubmitRequest: Promo code '{}' not found for player {} (Account: {})",
                body, player->GetName(), player->GetSession()->GetAccountId());
        }
        else if (promoCode->exist_count == 0)
        {
            errorId = 2;
            LOG_WARN("shop", "HandlePromoCodeSubmitRequest: Promo code '{}' has no remaining uses for player {} (Account: {})",
                body, player->GetName(), player->GetSession()->GetAccountId());
        }
        else
        {
            errorId = 3;
            LOG_WARN("shop", "HandlePromoCodeSubmitRequest: Promo code '{}' already used by player {} (Account: {})",
                body, player->GetName(), player->GetSession()->GetAccountId());
        }

        player->SendAddonMessage("ASMSG_PROMOCODE_SUBMIT\t{}", errorId);
    }
}
