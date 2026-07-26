/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraConfig.h"
#include "MakGoraMgr.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Player.h"

using namespace Acore::ChatCommands;

class MakGora_CommandScript : public CommandScript
{
public:
    MakGora_CommandScript() : CommandScript("MakGora_CommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable makgoraTable =
        {
            { "status",   HandleStatus,   SEC_PLAYER,       Console::No },
            { "eligible", HandleEligible, SEC_GAMEMASTER,   Console::No },
            { "bind",     HandleBind,     SEC_GAMEMASTER,   Console::No },
            { "ritual",   HandleRitual,   SEC_GAMEMASTER,   Console::No },
            { "leader",   HandleLeader,   SEC_ADMINISTRATOR, Console::No },
            { "reload",   HandleReload,   SEC_ADMINISTRATOR, Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "makgora", makgoraTable },
        };

        return commandTable;
    }

    static bool HandleStatus(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!sMakGoraConfig().IsEnabled())
        {
            handler->PSendSysMessage(MakGora::STRING_DISABLED);
            return true;
        }

        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        MakGora::LeaderRecord const* leader = sMakGoraMgr->GetLeader(player->GetTeamId());
        if (leader && leader->guid)
            handler->PSendSysMessage(MakGora::STRING_CURRENT_LEADER,
                player->GetTeamId() == TEAM_ALLIANCE ? "Alliance" : "Horde", leader->name);
        else
            handler->PSendSysMessage(MakGora::STRING_NO_LEADER);

        MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
        handler->PSendSysMessage("Eligible: {} | Hardcore bind: {} | Permadeath: {} | Leader: {}",
            record.eligible ? "yes" : "no",
            record.hardcoreBound ? "yes" : "no",
            record.permadeath ? "yes" : "no",
            sMakGoraMgr->IsFactionLeader(player) ? "yes" : "no");
        sMakGoraMgr->SyncAddonState(player);
        return true;
    }

    static bool HandleEligible(ChatHandler* handler, Optional<PlayerIdentifier> target)
    {
        if (!target)
            target = PlayerIdentifier::FromTargetOrSelf(handler);
        if (!target || !target->IsConnected())
            return false;

        Player* player = target->GetConnectedPlayer();
        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
        record.eligible = true;
        record.championDefeated = true; // GM shortcut for testing title path
        sMakGoraMgr->SetPlayerRecord(player->GetGUID().GetCounter(), record);
        handler->PSendSysMessage(MakGora::STRING_ELIGIBLE_SET, player->GetName());
        sMakGoraMgr->SyncAddonState(player);
        return true;
    }

    static bool HandleBind(ChatHandler* handler, Optional<bool> enable)
    {
        Player* player = handler->GetPlayer();
        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
        record.hardcoreBound = enable.value_or(!record.hardcoreBound);
        sMakGoraMgr->SetPlayerRecord(player->GetGUID().GetCounter(), record);
        handler->PSendSysMessage(MakGora::STRING_HARDCORE_TOGGLE, record.hardcoreBound ? "ON" : "OFF");
        sMakGoraMgr->SyncAddonState(player);
        return true;
    }

    static bool HandleRitual(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        Player* target = handler->getSelectedPlayer();
        if (!target || target == player)
        {
            handler->SendSysMessage("Select another player as the Mak'Gora opponent.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string error;
        if (!sMakGoraMgr->TryBeginRitual(player, target, true, error))
        {
            handler->PSendSysMessage("{}", error);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("Ritual armed. Start a duel to begin the arena phase.");
        return true;
    }

    static bool HandleLeader(ChatHandler* handler, Optional<PlayerIdentifier> target)
    {
        if (!target)
            target = PlayerIdentifier::FromTargetOrSelf(handler);
        if (!target || !target->IsConnected())
            return false;

        Player* player = target->GetConnectedPlayer();
        sMakGoraMgr->SetLeader(player->GetTeamId(), player);
        return true;
    }

    static bool HandleReload(ChatHandler* handler)
    {
        sMakGoraMgr->LoadFromDB();
        handler->SendSysMessage("Mak'Gora leaders reloaded from DB.");
        return true;
    }
};

void AddSC_MakGoraCommands()
{
    new MakGora_CommandScript();
}
