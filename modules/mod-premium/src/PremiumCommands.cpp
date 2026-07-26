/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Ported from OrstetCore cs_vip.cpp
 */

#include "PremiumCommon.h"
#include "PremiumConfig.h"
#include "PremiumMgr.h"
#include "Chat.h"
#include "CommandScript.h"
#include "GameTime.h"
#include "Group.h"
#include "Language.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellMgr.h"
#include "Util.h"
#include "WorldSessionMgr.h"

using namespace Acore::ChatCommands;

class Premium_CommandScript : public CommandScript
{
public:
    Premium_CommandScript() : CommandScript("Premium_CommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable vipTable =
        {
            { "buff",         HandleBuff,         SEC_PLAYER,       Console::No },
            { "debuff",       HandleDebuff,       SEC_PLAYER,       Console::No },
            { "bank",         HandleBank,         SEC_PLAYER,       Console::No },
            { "mail",         HandleMail,         SEC_PLAYER,       Console::No },
            { "repair",       HandleRepair,       SEC_PLAYER,       Console::No },
            { "resettalents", HandleResetTalents, SEC_PLAYER,       Console::No },
            { "taxi",         HandleTaxi,         SEC_PLAYER,       Console::No },
            { "home",         HandleHome,         SEC_PLAYER,       Console::No },
            { "capital",      HandleCapital,      SEC_PLAYER,       Console::No },
            { "changerace",   HandleChangeRace,   SEC_PLAYER,       Console::No },
            { "customize",    HandleCustomize,    SEC_PLAYER,       Console::No },
            { "app",          HandleAppear,       SEC_PLAYER,       Console::No },
            { "summon",       HandleSummon,       SEC_PLAYER,       Console::No },
            { "textcolor",    HandleTextColor,    SEC_PLAYER,       Console::No },
            { "free1day",     HandleFreeDay,      SEC_PLAYER,       Console::No },
            { "buy",          HandleBuy,          SEC_PLAYER,       Console::No },
            { "balance",      HandleBalance,      SEC_PLAYER,       Console::No },
            { "addbonus",     HandleAddBonus,     SEC_GAMEMASTER,   Console::Yes },
            { "set",          HandleSet,          SEC_GAMEMASTER,   Console::Yes },
            { "del",          HandleDel,          SEC_GAMEMASTER,   Console::Yes },
            { "",             HandleHelp,         SEC_PLAYER,       Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "vip", vipTable },
        };
        return commandTable;
    }

private:
    static bool EnsureEnabled(ChatHandler* handler)
    {
        if (sPremiumConfig().IsEnabled())
            return true;
        handler->SendSysMessage("Premium system disabled.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool CheckVip(ChatHandler* handler, Player* player)
    {
        if (!player)
            return false;

        if (player->IsHardcore())
        {
            handler->SendSysMessage("Hardcore characters cannot use premium commands.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sPremiumMgr->IsPremium(player))
        {
            handler->SendSysMessage(Premium::STRING_NOT_VIP);
            handler->SetSentErrorMessage(true);
            return false;
        }
        return true;
    }

    static bool CheckCmd(ChatHandler* handler, PremiumConfigOption opt)
    {
        if (sPremiumConfig().IsCmdEnabled(opt))
            return true;
        handler->SendSysMessage(Premium::STRING_CMD_DISABLED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool CheckState(ChatHandler* handler, Player* player)
    {
        if (player->GetMap()->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(Premium::STRING_BG);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player->HasStealthAura())
        {
            handler->SendSysMessage(Premium::STRING_STEALTH);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player->isDead() || player->HasUnitFlag2(UNIT_FLAG2_FEIGN_DEATH))
        {
            handler->SendSysMessage(Premium::STRING_DEAD);
            handler->SetSentErrorMessage(true);
            return false;
        }
        return true;
    }

    static bool CheckCombatFlight(ChatHandler* handler, Player* player)
    {
        if (player->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }
        return true;
    }

    static bool HandleHelp(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;

        Player* player = handler->GetPlayer();
        if (!player || !player->GetSession())
            return false;

        uint32 accountId = player->GetSession()->GetAccountId();
        bool isVip = sPremiumMgr->IsPremium(accountId);
        uint32 bonuses = sPremiumMgr->GetBonuses(accountId);
        bool freeAvail = !sPremiumMgr->HasClaimedFreeDay(accountId);

        handler->PSendSysMessage("Premium.Status: {}", isVip ? "active" : "inactive");
        handler->PSendSysMessage("Premium.Balance: {}", bonuses);
        handler->PSendSysMessage("Premium.FreeDay: {}", freeAvail ? "available" : "claimed");
        handler->PSendSysMessage("Premium.Price.1: {}", sPremiumConfig().GetPrice1Day());
        handler->PSendSysMessage("Premium.Price.7: {}", sPremiumConfig().GetPrice7Days());
        handler->PSendSysMessage("Premium.Price.31: {}", sPremiumConfig().GetPrice31Days());

        if (isVip)
        {
            time_t end = sPremiumMgr->GetEndTime(accountId);
            time_t now = GameTime::GetGameTime().count();
            if (end > now)
            {
                std::string left = secsToTimeString(uint64(end - now), true);
                handler->PSendSysMessage(Premium::STRING_TIME_LEFT, left);
                handler->PSendSysMessage("Premium.TimeLeft: {}", left);
            }
        }

        // Always emitted so the client addon can open for non-VIP players.
        handler->SendSysMessage("VIP panel ready.");
        if (isVip)
        {
            handler->SendSysMessage("VIP commands:");
            handler->SendSysMessage(" .vip buff|debuff|bank|mail|repair|resettalents|taxi|home|capital");
            handler->SendSysMessage(" .vip changerace|customize|app|summon|textcolor <0-10>");
        }
        handler->SendSysMessage(" .vip free1day | .vip buy 1|7|31 | .vip balance");
        if (handler->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            handler->SendSysMessage(" .vip set|del|addbonus");
        return true;
    }

    static void RefreshOnlinePremium(uint32 accountId)
    {
        sWorldSessionMgr->DoForAllOnlinePlayers([&](Player* p)
        {
            if (p && p->GetSession() && p->GetSession()->GetAccountId() == accountId)
            {
                sPremiumMgr->LoadAccount(accountId);
                sPremiumMgr->ApplyMounts(p);
            }
        });
    }

    static bool HandleBuff(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdBuff))
            return true;

        uint32 const buffs[] = { 25898, 48470, 53307, 48074, 48162, 48170, 57623, 43002, 47440 };
        for (uint32 spellId : buffs)
            if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
                player->CastSpell(player, info, true);
        return true;
    }

    static bool HandleDebuff(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdDebuff)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;

        player->RemoveAurasDueToSpell(15007);
        player->RemoveAurasDueToSpell(26013);
        return true;
    }

    static bool HandleBank(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdBank)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        handler->GetSession()->SendShowBank(player->GetGUID());
        return true;
    }

    static bool HandleMail(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdMail)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        handler->GetSession()->SendShowMailBox(player->GetGUID());
        return true;
    }

    static bool HandleRepair(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdRepair)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->DurabilityRepairAll(false, 0, false);
        handler->PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink(player));
        return true;
    }

    static bool HandleResetTalents(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdResetTalents)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->resetTalents(true);
        player->SendTalentsInfoData(false);
        handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->GetNameLink(player));
        return true;
    }

    static bool HandleTaxi(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdTaxi)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->SetTaxiCheater(true);
        handler->PSendSysMessage(LANG_YOU_GIVE_TAXIS, handler->GetNameLink(player));
        return true;
    }

    static bool HandleHome(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdHome)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->RemoveSpellCooldown(8690, true);
        player->CastSpell(player, 8690, false);
        return true;
    }

    static bool HandleCapital(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdCapital)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->CastSpell(player, player->GetTeamId() == TEAM_HORDE ? 3567 : 3561, true);
        return true;
    }

    static bool HandleChangeRace(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdChangeRace)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
        handler->SendSysMessage(Premium::STRING_CHANGE_RACE);
        return true;
    }

    static bool HandleCustomize(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdCustomize)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;
        player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
        handler->SendSysMessage(Premium::STRING_CUSTOMIZE);
        return true;
    }

    static bool HandleTextColor(ChatHandler* handler, uint32 colorId)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!player)
            return false;
        if (!sPremiumMgr->IsPremium(player) && !player->IsGameMaster())
        {
            handler->SendSysMessage(Premium::STRING_NOT_VIP);
            return true;
        }
        if (colorId > 10)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }
        sPremiumMgr->SetChatTextColor(player->GetSession()->GetAccountId(), uint8(colorId));
        handler->PSendSysMessage("VIP text color set to {}.", colorId);
        return true;
    }

    static bool HandleFreeDay(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        if (!CheckCmd(handler, PremiumConfigOption::CmdFreeDay))
            return true;

        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        if (sPremiumMgr->IsPremium(player))
        {
            handler->SendSysMessage("VIP already active.");
            return true;
        }

        uint32 accountId = player->GetSession()->GetAccountId();
        if (sPremiumMgr->HasClaimedFreeDay(accountId))
        {
            handler->SendSysMessage("Free VIP was already claimed on this account.");
            return true;
        }

        sPremiumMgr->Extend(accountId, 7 * DAY);
        sPremiumMgr->MarkFreeDayClaimed(accountId);
        sPremiumMgr->ApplyMounts(player);
        handler->SendSysMessage("Free VIP for 7 days granted.");
        handler->SendSysMessage("Premium.FreeDay: claimed");
        handler->PSendSysMessage("Premium.Status: active");
        return true;
    }

    static bool HandleBuy(ChatHandler* handler, uint32 days)
    {
        if (!EnsureEnabled(handler))
            return true;
        if (!CheckCmd(handler, PremiumConfigOption::CmdBuy))
            return true;

        Player* player = handler->GetPlayer();
        if (!player || !player->GetSession())
            return false;

        uint32 price = sPremiumConfig().GetPriceForDays(days);
        if (!price)
        {
            handler->SendSysMessage("Usage: .vip buy 1|7|31");
            return false;
        }

        uint32 accountId = player->GetSession()->GetAccountId();
        uint32 balance = sPremiumMgr->GetBonuses(accountId);
        if (!sPremiumMgr->TrySpendBonuses(accountId, price))
        {
            handler->PSendSysMessage("Not enough bonuses. Need: {}, you have: {}.", price, balance);
            handler->PSendSysMessage("Premium.Balance: {}", balance);
            return true;
        }

        sPremiumMgr->Extend(accountId, time_t(days) * DAY);
        sPremiumMgr->ApplyMounts(player);
        uint32 left = sPremiumMgr->GetBonuses(accountId);
        handler->PSendSysMessage("VIP purchased for {} day(s). Bonuses left: {}.", days, left);
        handler->PSendSysMessage("Premium.Balance: {}", left);
        handler->SendSysMessage("Premium.Status: active");
        RefreshOnlinePremium(accountId);
        return true;
    }

    static bool HandleBalance(ChatHandler* handler)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!player || !player->GetSession())
            return false;

        uint32 bonuses = sPremiumMgr->GetBonuses(player->GetSession()->GetAccountId());
        handler->PSendSysMessage("Premium.Balance: {}", bonuses);
        handler->PSendSysMessage("Your bonus balance: {}.", bonuses);
        return true;
    }

    static bool HandleAddBonus(ChatHandler* handler, uint32 amount, Optional<uint32> accountIdOpt)
    {
        if (!EnsureEnabled(handler))
            return true;
        if (!amount)
        {
            handler->SendSysMessage("Usage: .vip addbonus <amount> [accountId]");
            return false;
        }

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 accountId = accountIdOpt.value_or(0);
        if (!accountId)
        {
            if (!target || !target->GetSession())
                return false;
            accountId = target->GetSession()->GetAccountId();
        }

        sPremiumMgr->AddBonuses(accountId, amount);
        uint32 left = sPremiumMgr->GetBonuses(accountId);
        handler->PSendSysMessage("Added {} bonuses to account {}. Balance: {}.", amount, accountId, left);
        return true;
    }

    static bool HandleSet(ChatHandler* handler, uint32 days, Optional<uint32> accountIdOpt)
    {
        if (!EnsureEnabled(handler))
            return true;
        if (!days || days > 365)
        {
            handler->SendSysMessage("Days must be 1..365.");
            return false;
        }

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 accountId = accountIdOpt.value_or(0);
        if (!accountId)
        {
            if (!target || !target->GetSession())
                return false;
            accountId = target->GetSession()->GetAccountId();
        }

        time_t end = GameTime::GetGameTime().count() + time_t(days) * DAY;
        sPremiumMgr->SetOrUpdate(accountId, end);

        RefreshOnlinePremium(accountId);

        handler->PSendSysMessage("VIP set for account {} for {} day(s).", accountId, days);
        LOG_INFO("module", "Premium: account {} set for {} days by {}",
            accountId, days, handler->GetSession() ? handler->GetSession()->GetAccountId() : 0);
        return true;
    }

    static bool HandleDel(ChatHandler* handler, Optional<uint32> accountIdOpt)
    {
        if (!EnsureEnabled(handler))
            return true;

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 accountId = accountIdOpt.value_or(0);
        if (!accountId)
        {
            if (!target || !target->GetSession())
                return false;
            accountId = target->GetSession()->GetAccountId();
        }

        if (!sPremiumMgr->IsPremium(accountId))
        {
            handler->SendSysMessage(Premium::STRING_TARGET_NOT_VIP);
            return true;
        }

        sPremiumMgr->Remove(accountId);
        sWorldSessionMgr->DoForAllOnlinePlayers([&](Player* p)
        {
            if (p && p->GetSession() && p->GetSession()->GetAccountId() == accountId)
                sPremiumMgr->RemoveMounts(p);
        });

        handler->PSendSysMessage("VIP removed for account {}.", accountId);
        return true;
    }

    static bool HandleAppear(ChatHandler* handler, Optional<PlayerIdentifier> targetId)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdAppear)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;

        if (!targetId)
            targetId = PlayerIdentifier::FromTarget(handler);
        if (!targetId || !targetId->IsConnected())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_EXIST_OR_OFFLINE);
            return false;
        }

        Player* target = targetId->GetConnectedPlayer();
        if (!target || target == player)
        {
            handler->SendSysMessage(LANG_CANT_TELEPORT_SELF);
            return false;
        }
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;
        if (!CheckCombatFlight(handler, target) || !CheckState(handler, target))
            return true;
        if (player->GetGroup() && player->GetGroup() != target->GetGroup())
        {
            handler->SendSysMessage(Premium::STRING_GROUP);
            return true;
        }

        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        else
            player->SaveRecallPosition();

        float x, y, z;
        target->GetContactPoint(player, x, y, z);
        player->TeleportTo(target->GetMapId(), x, y, z, player->GetAbsoluteAngle(target), TELE_TO_GM_MODE);
        player->SetPhaseMask(target->GetPhaseMask(), true);
        return true;
    }

    static bool HandleSummon(ChatHandler* handler, Optional<PlayerIdentifier> targetId)
    {
        if (!EnsureEnabled(handler))
            return true;
        Player* player = handler->GetPlayer();
        if (!CheckVip(handler, player) || !CheckCmd(handler, PremiumConfigOption::CmdSummon)
            || !CheckCombatFlight(handler, player) || !CheckState(handler, player))
            return true;

        if (!targetId)
            targetId = PlayerIdentifier::FromTarget(handler);
        if (!targetId || !targetId->IsConnected())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_EXIST_OR_OFFLINE);
            return false;
        }

        Player* target = targetId->GetConnectedPlayer();
        if (!target || target == player)
        {
            handler->SendSysMessage(LANG_CANT_TELEPORT_SELF);
            return false;
        }
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;
        if (!CheckCombatFlight(handler, target) || !CheckState(handler, target))
            return true;
        if (!player->GetGroup() || player->GetGroup() != target->GetGroup())
        {
            handler->SendSysMessage(Premium::STRING_GROUP);
            return true;
        }

        if (target->IsInFlight())
        {
            target->GetMotionMaster()->MovementExpired();
            target->CleanupAfterTaxiFlight();
        }
        else
            target->SaveRecallPosition();

        float x, y, z;
        player->GetClosePoint(x, y, z, target->GetObjectSize());
        target->TeleportTo(player->GetMapId(), x, y, z, target->GetOrientation(), TELE_TO_GM_MODE, player);
        target->SetPhaseMask(player->GetPhaseMask(), true);
        return true;
    }
};

void AddSC_PremiumCommands()
{
    new Premium_CommandScript();
}
