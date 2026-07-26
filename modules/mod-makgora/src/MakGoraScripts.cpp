/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraAddon.h"
#include "MakGoraConfig.h"
#include "MakGoraMgr.h"
#include "Chat.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"

class MakGora_PlayerScript : public PlayerScript
{
public:
    MakGora_PlayerScript() : PlayerScript("MakGora_PlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_DUEL_START,
        PLAYERHOOK_ON_DUEL_END,
        PLAYERHOOK_ON_PLAYER_JUST_DIED,
        PLAYERHOOK_ON_PLAYER_RELEASED_GHOST,
        PLAYERHOOK_ON_PLAYER_RESURRECT,
        PLAYERHOOK_CAN_REPOP_AT_GRAVEYARD,
        PLAYERHOOK_ON_BEFORE_SEND_CHAT_MESSAGE
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return;

        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        sMakGoraMgr->SyncAddonState(player);

        if (sMakGoraMgr->IsPermadeath(player))
        {
            ChatHandler(player->GetSession()).PSendSysMessage(MakGora::STRING_GHOST_FOREVER);
            if (!player->isDead())
                player->KillPlayer();
        }
    }

    void OnPlayerDuelStart(Player* player1, Player* player2) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player1 || !player2)
            return;

        auto ritual = sMakGoraMgr->FindRitual(player1->GetGUID());
        if (!ritual)
            return;

        if ((ritual->challenger == player1->GetGUID() && ritual->opponent == player2->GetGUID())
            || (ritual->challenger == player2->GetGUID() && ritual->opponent == player1->GetGUID()))
            sMakGoraMgr->MarkRitualArena(player1->GetGUID(), player2->GetGUID());
    }

    void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
    {
        if (!sMakGoraConfig().IsEnabled() || !winner || !loser)
            return;

        auto ritual = sMakGoraMgr->FindRitual(winner->GetGUID());
        if (!ritual)
            return;

        MakGora::RitualOutcome outcome = MakGora::RitualOutcome::Interrupted;
        switch (type)
        {
            case DUEL_WON:
                outcome = ritual->hardcore ? MakGora::RitualOutcome::Permadeath : MakGora::RitualOutcome::TitleTransfer;
                break;
            case DUEL_FLED:
                outcome = MakGora::RitualOutcome::Fled;
                break;
            default:
                outcome = MakGora::RitualOutcome::Interrupted;
                break;
        }

        sMakGoraMgr->ResolveRitual(winner, loser, outcome);
    }

    void OnPlayerJustDied(Player* player) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return;

        if (sMakGoraMgr->IsPermadeath(player))
            ChatHandler(player->GetSession()).PSendSysMessage(MakGora::STRING_GHOST_FOREVER);
    }

    void OnPlayerReleasedGhost(Player* player) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return;

        if (sMakGoraMgr->IsPermadeath(player))
            ChatHandler(player->GetSession()).PSendSysMessage(MakGora::STRING_GHOST_FOREVER);
    }

    void OnPlayerResurrect(Player* player, float /*restore_percent*/, bool& /*applySickness*/) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return;

        if (!sMakGoraMgr->IsPermadeath(player))
            return;

        // Block exploit resurrect paths for permanent ghosts.
        player->KillPlayer();
        ChatHandler(player->GetSession()).PSendSysMessage(MakGora::STRING_GHOST_FOREVER);
    }

    bool OnPlayerCanRepopAtGraveyard(Player* player) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return true;

        // Allow release-to-ghost, but afterlife content will redirect later.
        return true;
    }

    void OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg) override
    {
        if (!sMakGoraConfig().IsEnabled() || !player)
            return;

        if (lang != LANG_ADDON)
            return;

        if (type != CHAT_MSG_WHISPER && type != CHAT_MSG_GUILD && type != CHAT_MSG_PARTY && type != CHAT_MSG_RAID
            && type != CHAT_MSG_RAID_LEADER && type != CHAT_MSG_RAID_WARNING && type != CHAT_MSG_BATTLEGROUND
            && type != CHAT_MSG_BATTLEGROUND_LEADER && type != CHAT_MSG_CHANNEL)
            return;

        if (MakGoraAddon::HandleIncoming(player, msg))
            return; // handled; allow original packet through for client echo if needed
    }
};

class MakGora_WorldScript : public WorldScript
{
public:
    MakGora_WorldScript() : WorldScript("MakGora_WorldScript", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) { }

    void OnBeforeConfigLoad(bool reload) override
    {
        sMakGoraConfig().Initialize(reload);
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
    }

    void OnStartup() override
    {
        if (!sMakGoraConfig().IsEnabled())
        {
            LOG_INFO("module", ">> Mak'Gora module loaded (disabled in config)");
            return;
        }

        sMakGoraMgr->LoadFromDB();
        LOG_INFO("module", ">> Mak'Gora module started");
    }
};

void AddSC_MakGoraCore()
{
    new MakGora_PlayerScript();
    new MakGora_WorldScript();
}
