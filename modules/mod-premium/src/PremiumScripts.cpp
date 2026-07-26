/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "PremiumCommon.h"
#include "PremiumConfig.h"
#include "PremiumMgr.h"
#include "Chat.h"
#include "DBCStructure.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Util.h"
#include <algorithm>

class Premium_PlayerScript : public PlayerScript
{
public:
    Premium_PlayerScript() : PlayerScript("Premium_PlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_GIVE_REPUTATION,
        PLAYERHOOK_ON_VICTIM_REWARD_AFTER,
        PLAYERHOOK_ON_UPDATE_GATHERING_SKILL,
        PLAYERHOOK_ON_UPDATE_CRAFTING_SKILL,
        PLAYERHOOK_ON_REWARD_RANK_POINTS,
        PLAYERHOOK_ON_RANK_BUFF_STACKS,
        PLAYERHOOK_CAN_PLAYER_USE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sPremiumConfig().IsEnabled() || !player)
            return;

        sPremiumMgr->SyncPlayer(player);

        if (!sPremiumMgr->IsPremium(player))
            return;

        time_t end = sPremiumMgr->GetEndTime(player->GetSession()->GetAccountId());
        time_t now = GameTime::GetGameTime().count();
        if (end > now)
        {
            time_t left = end - now;
            if (left <= 5 * MINUTE)
                ChatHandler(player->GetSession()).PSendSysMessage(Premium::STRING_TIME_NEAR_END);
            else
                ChatHandler(player->GetSession()).PSendSysMessage(Premium::STRING_TIME_LEFT,
                    secsToTimeString(uint64(left), true));
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!player || !player->GetSession())
            return;
        sPremiumMgr->Invalidate(player->GetSession()->GetAccountId());
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 xpSource) override
    {
        if (!sPremiumConfig().IsEnabled() || !sPremiumMgr->IsPremium(player) || !amount)
            return;

        float rate = 1.0f;
        switch (xpSource)
        {
            case XPSOURCE_KILL:
                rate = sPremiumConfig().GetRateXpKill();
                break;
            case XPSOURCE_QUEST:
            case XPSOURCE_QUEST_DF:
            case XPSOURCE_EXPLORE:
            case XPSOURCE_BATTLEGROUND:
            default:
                rate = sPremiumConfig().GetRateXpQuest();
                break;
        }

        if (rate != 1.0f)
            amount = uint32(float(amount) * rate);
    }

    void OnPlayerRewardRankPoints(Player* player, uint32& amount) override
    {
        if (!sPremiumConfig().IsEnabled() || !sPremiumMgr->IsPremium(player) || !amount)
            return;

        float rate = sPremiumConfig().GetRateRankReward();
        if (rate == 1.0f)
            return;

        amount = static_cast<uint32>(std::max(0.0f, float(amount) * rate));
    }

    void OnPlayerRankBuffStacks(Player* player, int& stacks) override
    {
        if (!sPremiumConfig().IsEnabled() || stacks <= 0)
            return;

        // Non-premium players receive half of rank-based instance buffs.
        if (!sPremiumMgr->IsPremium(player))
            stacks /= 2;
    }

    void OnPlayerGiveReputation(Player* player, int32 /*factionID*/, float& amount,
        ReputationSource /*repSource*/) override
    {
        if (!sPremiumConfig().IsEnabled() || !sPremiumMgr->IsPremium(player))
            return;
        float rate = sPremiumConfig().GetRateReputation();
        if (rate != 1.0f)
            amount *= rate;
    }

    void OnPlayerVictimRewardAfter(Player* player, Player* /*victim*/, uint32& /*killer_title*/,
        int32& /*victim_rank*/, float& honor_f) override
    {
        if (!sPremiumConfig().IsEnabled() || !sPremiumMgr->IsPremium(player))
            return;
        float rate = sPremiumConfig().GetRateHonor();
        if (rate != 1.0f)
            honor_f *= rate;
    }

    void OnPlayerUpdateGatheringSkill(Player* player, uint32 /*skill_id*/, uint32 /*current*/,
        uint32 /*gray*/, uint32 /*green*/, uint32 /*yellow*/, uint32& gain) override
    {
        ApplySkillGain(player, gain);
    }

    void OnPlayerUpdateCraftingSkill(Player* player, SkillLineAbilityEntry const* /*skill*/,
        uint32 /*current_level*/, uint32& gain) override
    {
        ApplySkillGain(player, gain);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg) override
    {
        return Colorize(player, type, language, msg);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg,
        Player* /*receiver*/) override
    {
        return Colorize(player, type, language, msg);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg,
        Group* /*group*/) override
    {
        return Colorize(player, type, language, msg);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg,
        Guild* /*guild*/) override
    {
        return Colorize(player, type, language, msg);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg,
        Channel* /*channel*/) override
    {
        return Colorize(player, type, language, msg);
    }

private:
    static void ApplySkillGain(Player* player, uint32& gain)
    {
        if (!sPremiumConfig().IsEnabled() || !sPremiumMgr->IsPremium(player) || !gain)
            return;
        float rate = sPremiumConfig().GetRateSkillGain();
        if (rate != 1.0f)
            gain = std::max<uint32>(1, uint32(float(gain) * rate));
    }

    static char const* ColorCode(uint8 id)
    {
        switch (id)
        {
            case 0:  return "";
            case 1:  return "|cFFFFD700";
            case 2:  return "|cFFFF4444";
            case 3:  return "|cFF00FF00";
            case 4:  return "|cFF00BFFF";
            case 5:  return "|cFFB048F8";
            case 6:  return "|cFFFF8000";
            case 7:  return "|cFFFF69B4";
            case 8:  return "|cFF00FFFF";
            case 9:  return "|cFFFFFFFF";
            case 10: return "|cFFE94560";
            default: return "|cFFFFD700";
        }
    }

    static bool ShouldColorize(uint32 type, uint32 language)
    {
        if (language == LANG_ADDON)
            return false;
        switch (type)
        {
            case CHAT_MSG_SAY:
            case CHAT_MSG_YELL:
            case CHAT_MSG_PARTY:
            case CHAT_MSG_PARTY_LEADER:
            case CHAT_MSG_RAID:
            case CHAT_MSG_RAID_LEADER:
            case CHAT_MSG_RAID_WARNING:
            case CHAT_MSG_GUILD:
            case CHAT_MSG_OFFICER:
            case CHAT_MSG_WHISPER:
            case CHAT_MSG_CHANNEL:
                return true;
            default:
                return false;
        }
    }

    static bool Colorize(Player* player, uint32 type, uint32 language, std::string& msg)
    {
        if (!sPremiumConfig().IsEnabled() || !player || msg.empty() || !ShouldColorize(type, language))
            return true;
        if (!sPremiumMgr->IsPremium(player))
            return true;

        uint8 colorId = sPremiumMgr->GetChatTextColor(player->GetSession()->GetAccountId());
        if (!colorId)
            return true;

        char const* code = ColorCode(colorId);
        if (!code || !*code)
            return true;

        msg = std::string(code) + msg + "|r";
        return true;
    }
};

class Premium_WorldScript : public WorldScript
{
public:
    Premium_WorldScript() : WorldScript("Premium_WorldScript", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) { }

    void OnBeforeConfigLoad(bool reload) override
    {
        sPremiumConfig().Initialize(reload);
    }

    void OnStartup() override
    {
        if (!sPremiumConfig().IsEnabled())
        {
            LOG_INFO("module", ">> Premium module loaded (disabled in config)");
            return;
        }
        LOG_INFO("module", ">> Premium module started");
    }
};

void AddSC_PremiumCore()
{
    new Premium_PlayerScript();
    new Premium_WorldScript();
}
