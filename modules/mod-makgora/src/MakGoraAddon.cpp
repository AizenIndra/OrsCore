/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraAddon.h"
#include "MakGoraConfig.h"
#include "MakGoraMgr.h"
#include "Chat.h"
#include "Player.h"
#include "StringFormat.h"
#include "Tokenize.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace MakGoraAddon
{
void Send(Player* player, std::string_view payload)
{
    if (!player || !player->GetSession())
        return;

    std::string message = Acore::StringFormat("{}\t{}", sMakGoraConfig().GetAddonPrefix(), payload);
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, player, player, message);
    player->GetSession()->SendPacket(&data);
}

void SendState(Player* player)
{
    if (!player)
        return;

    sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
    MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
    MakGora::LeaderRecord const* leader = sMakGoraMgr->GetLeader(player->GetTeamId());
    bool isLeader = sMakGoraMgr->IsFactionLeader(player);
    auto ritual = sMakGoraMgr->FindRitual(player->GetGUID());

    Send(player, Acore::StringFormat(
        "STATE\t{}\t{}\t{}\t{}\t{}\t{}\t{}",
        sMakGoraConfig().IsEnabled() ? 1 : 0,
        record.eligible ? 1 : 0,
        record.hardcoreBound ? 1 : 0,
        record.permadeath ? 1 : 0,
        isLeader ? 1 : 0,
        leader ? leader->name : "",
        ritual ? uint32(ritual->state) : 0));
}

bool HandleIncoming(Player* player, std::string& msg)
{
    if (!player)
        return false;

    std::string_view prefix = sMakGoraConfig().GetAddonPrefix();
    if (msg.size() < prefix.size() + 1 || msg.compare(0, prefix.size(), prefix) != 0 || msg[prefix.size()] != '\t')
        return false;

    std::string_view body(msg.c_str() + prefix.size() + 1);
    std::vector<std::string_view> parts = Acore::Tokenize(body, '\t', false);
    if (parts.empty())
        return true;

    if (parts[0] == "PING")
    {
        Send(player, "PONG");
        return true;
    }

    if (parts[0] == "SYNC")
    {
        SendState(player);
        return true;
    }

    if (parts[0] == "BIND" && parts.size() >= 2)
    {
        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
        record.hardcoreBound = parts[1] == "1";
        sMakGoraMgr->SetPlayerRecord(player->GetGUID().GetCounter(), record);
        ChatHandler(player->GetSession()).PSendSysMessage(MakGora::STRING_HARDCORE_TOGGLE, record.hardcoreBound ? "ON" : "OFF");
        SendState(player);
        return true;
    }

    return true;
}
} // namespace MakGoraAddon
