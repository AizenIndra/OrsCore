/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_MAKGORA_MGR_H
#define MOD_MAKGORA_MGR_H

#include "MakGoraCommon.h"
#include "ObjectGuid.h"
#include <optional>
#include <unordered_map>

class Player;

class MakGoraMgr
{
public:
    static MakGoraMgr* instance();

    void LoadFromDB();
    void SaveLeaders();
    void SavePlayer(ObjectGuid::LowType guid, MakGora::PlayerRecord const& record);

    [[nodiscard]] MakGora::LeaderRecord const* GetLeader(TeamId teamId) const;
    void SetLeader(TeamId teamId, Player* player);
    void ClearLeader(TeamId teamId);

    [[nodiscard]] MakGora::PlayerRecord GetPlayerRecord(ObjectGuid::LowType guid) const;
    void SetPlayerRecord(ObjectGuid::LowType guid, MakGora::PlayerRecord const& record);
    void EnsurePlayerLoaded(ObjectGuid::LowType guid);

    [[nodiscard]] bool IsEligible(Player const* player) const;
    [[nodiscard]] bool MeetsLevelGate(Player const* player) const;
    [[nodiscard]] bool IsFactionLeader(Player const* player) const;
    [[nodiscard]] bool IsPermadeath(Player const* player) const;

    bool TryBeginRitual(Player* challenger, Player* opponent, bool hardcore, std::string& error);
    void MarkRitualArena(ObjectGuid a, ObjectGuid b);
    std::optional<MakGora::RitualSession> FindRitual(ObjectGuid guid) const;
    void ResolveRitual(Player* winner, Player* loser, MakGora::RitualOutcome outcome);
    void CancelRitual(ObjectGuid guid, MakGora::RitualOutcome outcome = MakGora::RitualOutcome::Interrupted);

    void ApplyPermadeath(Player* loser);
    void Announce(uint32 stringId, std::string_view arg1 = {}, std::string_view arg2 = {}) const;
    void SyncAddonState(Player* player) const;

private:
    MakGoraMgr() = default;

    MakGora::LeaderRecord _leaders[2];
    std::unordered_map<ObjectGuid::LowType, MakGora::PlayerRecord> _players;
    std::unordered_map<ObjectGuid, MakGora::RitualSession> _ritualsByPlayer;
};

#define sMakGoraMgr MakGoraMgr::instance()

#endif
