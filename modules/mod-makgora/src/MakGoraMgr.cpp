/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraMgr.h"
#include "MakGoraAddon.h"
#include "MakGoraConfig.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "WorldSession.h"
#include "WorldSessionMgr.h"

MakGoraMgr* MakGoraMgr::instance()
{
    static MakGoraMgr mgr;
    return &mgr;
}

void MakGoraMgr::LoadFromDB()
{
    _players.clear();
    _ritualsByPlayer.clear();
    _leaders[TEAM_ALLIANCE] = {};
    _leaders[TEAM_HORDE] = {};

    if (QueryResult result = CharacterDatabase.Query("SELECT team, guid, name, claimed_at, reign_count FROM makgora_leader"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint8 team = fields[0].Get<uint8>();
            if (team > TEAM_HORDE)
                continue;

            MakGora::LeaderRecord& record = _leaders[team];
            record.guid = fields[1].Get<uint32>();
            record.name = fields[2].Get<std::string>();
            record.claimedAt = time_t(fields[3].Get<uint32>());
            record.reignCount = fields[4].Get<uint32>();
        } while (result->NextRow());
    }

    LOG_INFO("module", ">> Mak'Gora: loaded faction leaders (Alliance guid {}, Horde guid {})",
        _leaders[TEAM_ALLIANCE].guid, _leaders[TEAM_HORDE].guid);
}

void MakGoraMgr::SaveLeaders()
{
    for (uint8 team = TEAM_ALLIANCE; team <= TEAM_HORDE; ++team)
    {
        MakGora::LeaderRecord const& record = _leaders[team];
        std::string name = record.name;
        CharacterDatabase.EscapeString(name);
        CharacterDatabase.DirectExecute(
            "REPLACE INTO makgora_leader (team, guid, name, claimed_at, reign_count) VALUES ({}, {}, '{}', {}, {})",
            uint32(team), record.guid, name, uint32(record.claimedAt), record.reignCount);
    }
}

void MakGoraMgr::SavePlayer(ObjectGuid::LowType guid, MakGora::PlayerRecord const& record)
{
    CharacterDatabase.Execute(
        "REPLACE INTO makgora_player (guid, eligible, champion_defeated, hardcore_bound, permadeath, cosmetics, updated_at) "
        "VALUES ({}, {}, {}, {}, {}, {}, {})",
        guid,
        record.eligible ? 1 : 0,
        record.championDefeated ? 1 : 0,
        record.hardcoreBound ? 1 : 0,
        record.permadeath ? 1 : 0,
        record.cosmetics,
        uint32(GameTime::GetGameTime().count()));
}

MakGora::LeaderRecord const* MakGoraMgr::GetLeader(TeamId teamId) const
{
    if (teamId > TEAM_HORDE)
        return nullptr;
    return &_leaders[teamId];
}

void MakGoraMgr::SetLeader(TeamId teamId, Player* player)
{
    if (!player || teamId > TEAM_HORDE)
        return;

    MakGora::LeaderRecord& record = _leaders[teamId];
    record.guid = player->GetGUID().GetCounter();
    record.name = player->GetName();
    record.claimedAt = GameTime::GetGameTime().count();
    ++record.reignCount;
    SaveLeaders();

    if (sMakGoraConfig().AnnounceWorld())
        Announce(MakGora::STRING_CORONATION, player->GetName(), teamId == TEAM_ALLIANCE ? "Alliance" : "Horde");
}

void MakGoraMgr::ClearLeader(TeamId teamId)
{
    if (teamId > TEAM_HORDE)
        return;

    _leaders[teamId] = {};
    SaveLeaders();
}

void MakGoraMgr::EnsurePlayerLoaded(ObjectGuid::LowType guid)
{
    if (_players.find(guid) != _players.end())
        return;

    MakGora::PlayerRecord record;
    if (QueryResult result = CharacterDatabase.Query(
            "SELECT eligible, champion_defeated, hardcore_bound, permadeath, cosmetics FROM makgora_player WHERE guid = {}", guid))
    {
        Field* fields = result->Fetch();
        record.eligible = fields[0].Get<uint8>() != 0;
        record.championDefeated = fields[1].Get<uint8>() != 0;
        record.hardcoreBound = fields[2].Get<uint8>() != 0;
        record.permadeath = fields[3].Get<uint8>() != 0;
        record.cosmetics = fields[4].Get<uint32>();
    }

    _players[guid] = record;
}

MakGora::PlayerRecord MakGoraMgr::GetPlayerRecord(ObjectGuid::LowType guid) const
{
    auto itr = _players.find(guid);
    if (itr != _players.end())
        return itr->second;
    return {};
}

void MakGoraMgr::SetPlayerRecord(ObjectGuid::LowType guid, MakGora::PlayerRecord const& record)
{
    _players[guid] = record;
    SavePlayer(guid, record);
}

bool MakGoraMgr::IsEligible(Player const* player) const
{
    if (!player)
        return false;

    auto itr = _players.find(player->GetGUID().GetCounter());
    return itr != _players.end() && itr->second.eligible;
}

bool MakGoraMgr::MeetsLevelGate(Player const* player) const
{
    if (!player)
        return false;

    uint8 level = player->GetLevel();
    return level >= sMakGoraConfig().GetMinLevel() && level <= sMakGoraConfig().GetMaxLevel();
}

bool MakGoraMgr::IsFactionLeader(Player const* player) const
{
    if (!player)
        return false;

    TeamId teamId = player->GetTeamId();
    if (teamId > TEAM_HORDE)
        return false;

    return _leaders[teamId].guid == player->GetGUID().GetCounter();
}

bool MakGoraMgr::IsPermadeath(Player const* player) const
{
    if (!player)
        return false;

    auto itr = _players.find(player->GetGUID().GetCounter());
    return itr != _players.end() && itr->second.permadeath;
}

bool MakGoraMgr::TryBeginRitual(Player* challenger, Player* opponent, bool hardcore, std::string& error)
{
    if (!challenger || !opponent)
    {
        error = "Invalid participants.";
        return false;
    }

    if (!sMakGoraConfig().IsEnabled())
    {
        error = "Mak'Gora is disabled.";
        return false;
    }

    if (!MeetsLevelGate(challenger) || !MeetsLevelGate(opponent))
    {
        error = "Level gate not met.";
        return false;
    }

    if (sMakGoraConfig().RequireSameFaction() && challenger->GetTeamId() != opponent->GetTeamId())
    {
        error = "Same faction required.";
        return false;
    }

    EnsurePlayerLoaded(challenger->GetGUID().GetCounter());
    EnsurePlayerLoaded(opponent->GetGUID().GetCounter());

    if (!IsEligible(challenger) || !IsEligible(opponent))
    {
        error = "Both champions must complete the Mak'Gora quest path.";
        return false;
    }

    if (FindRitual(challenger->GetGUID()) || FindRitual(opponent->GetGUID()))
    {
        error = "A ritual is already in progress.";
        return false;
    }

    TeamId teamId = challenger->GetTeamId();
    MakGora::LeaderRecord const* leader = GetLeader(teamId);
    MakGora::PlayerRecord challengerRec = GetPlayerRecord(challenger->GetGUID().GetCounter());

    if (leader && leader->guid == 0 && !challengerRec.championDefeated && sMakGoraConfig().GetChampionEntry() != 0)
    {
        error = "Defeat the elite champion first.";
        return false;
    }

    MakGora::RitualSession session;
    session.challenger = challenger->GetGUID();
    session.opponent = opponent->GetGUID();
    session.teamId = teamId;
    session.state = MakGora::RitualState::Pending;
    session.hardcore = hardcore || challengerRec.hardcoreBound
        || GetPlayerRecord(opponent->GetGUID().GetCounter()).hardcoreBound;
    session.startedAt = GameTime::GetGameTime().count();

    _ritualsByPlayer[challenger->GetGUID()] = session;
    _ritualsByPlayer[opponent->GetGUID()] = session;

    Announce(MakGora::STRING_RITUAL_BEGIN, challenger->GetName(), opponent->GetName());
    SyncAddonState(challenger);
    SyncAddonState(opponent);
    return true;
}

void MakGoraMgr::MarkRitualArena(ObjectGuid a, ObjectGuid b)
{
    auto mark = [&](ObjectGuid guid)
    {
        auto itr = _ritualsByPlayer.find(guid);
        if (itr != _ritualsByPlayer.end())
            itr->second.state = MakGora::RitualState::Arena;
    };
    mark(a);
    mark(b);
}

std::optional<MakGora::RitualSession> MakGoraMgr::FindRitual(ObjectGuid guid) const
{
    auto itr = _ritualsByPlayer.find(guid);
    if (itr == _ritualsByPlayer.end())
        return std::nullopt;
    return itr->second;
}

void MakGoraMgr::ResolveRitual(Player* winner, Player* loser, MakGora::RitualOutcome outcome)
{
    if (!winner || !loser)
        return;

    TeamId teamId = winner->GetTeamId();
    std::string winnerName = winner->GetName();
    std::string loserName = loser->GetName();
    CharacterDatabase.EscapeString(winnerName);
    CharacterDatabase.EscapeString(loserName);

    CharacterDatabase.Execute(
        "INSERT INTO makgora_history (team, winner_guid, loser_guid, winner_name, loser_name, outcome, happened_at) "
        "VALUES ({}, {}, {}, '{}', '{}', {}, {})",
        uint32(teamId),
        winner->GetGUID().GetCounter(),
        loser->GetGUID().GetCounter(),
        winnerName,
        loserName,
        uint32(outcome),
        uint32(GameTime::GetGameTime().count()));

    MakGora::LeaderRecord const* leader = GetLeader(teamId);
    bool titleFight = !leader || leader->guid == 0
        || leader->guid == winner->GetGUID().GetCounter()
        || leader->guid == loser->GetGUID().GetCounter();

    if (titleFight && outcome != MakGora::RitualOutcome::Interrupted && outcome != MakGora::RitualOutcome::Fled)
        SetLeader(teamId, winner);

    if (outcome == MakGora::RitualOutcome::Permadeath)
        ApplyPermadeath(loser);

    CancelRitual(winner->GetGUID(), outcome);
}

void MakGoraMgr::CancelRitual(ObjectGuid guid, MakGora::RitualOutcome /*outcome*/)
{
    auto itr = _ritualsByPlayer.find(guid);
    if (itr == _ritualsByPlayer.end())
        return;

    ObjectGuid other = itr->second.challenger == guid ? itr->second.opponent : itr->second.challenger;
    _ritualsByPlayer.erase(guid);
    _ritualsByPlayer.erase(other);
}

void MakGoraMgr::ApplyPermadeath(Player* loser)
{
    if (!loser || !sMakGoraConfig().PermadeathOnLoss())
        return;

    EnsurePlayerLoaded(loser->GetGUID().GetCounter());
    MakGora::PlayerRecord record = GetPlayerRecord(loser->GetGUID().GetCounter());
    record.permadeath = true;
    SetPlayerRecord(loser->GetGUID().GetCounter(), record);

    if (!loser->isDead())
        loser->KillPlayer();

    if (sMakGoraConfig().AnnounceWorld())
        Announce(MakGora::STRING_PERMADEATH, loser->GetName());

    ChatHandler(loser->GetSession()).PSendSysMessage(MakGora::STRING_GHOST_FOREVER);
    SyncAddonState(loser);
}

void MakGoraMgr::Announce(uint32 stringId, std::string_view arg1, std::string_view arg2) const
{
    WorldSessionMgr::SessionMap const& sessions = sWorldSessionMgr->GetAllSessions();
    for (auto const& [id, session] : sessions)
    {
        if (!session || !session->GetPlayer() || !session->GetPlayer()->IsInWorld())
            continue;

        ChatHandler handler(session);
        if (!arg1.empty() && !arg2.empty())
            handler.PSendSysMessage(stringId, std::string(arg1), std::string(arg2));
        else if (!arg1.empty())
            handler.PSendSysMessage(stringId, std::string(arg1));
        else
            handler.PSendSysMessage(stringId);
    }
}

void MakGoraMgr::SyncAddonState(Player* player) const
{
    if (!player)
        return;

    MakGoraAddon::SendState(player);
}
