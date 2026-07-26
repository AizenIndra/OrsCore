/*
 * Online Rewards System
 * Tracks player online time and provides rewards through LuckyWheel
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "Chat.h"
#include <ctime>
#include <unordered_map>

// Online time requirement is defined in AddonIO.cpp (REQUIRED_ONLINE_TIME)

class OnlineRewardsPlayerScript : public PlayerScript
{
public:
    OnlineRewardsPlayerScript() : PlayerScript("OnlineRewardsPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_UPDATE,
        PLAYERHOOK_ON_SAVE
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        LoadOnlineTimeData(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!player)
            return;

        SaveOnlineTimeData(player);
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!player)
            return;

        static std::unordered_map<ObjectGuid, uint32> updateTimers;

        ObjectGuid guid = player->GetGUID();
        updateTimers[guid] += diff;

        if (updateTimers[guid] >= 1000)
        {
            updateTimers[guid] = 0;
            UpdateOnlineTime(player);
        }
    }

    void OnPlayerSave(Player* player) override
    {
        if (!player)
            return;

        SaveOnlineTimeData(player);
    }

private:
    void LoadOnlineTimeData(Player* player)
    {
        if (!player)
            return;

        ObjectGuid::LowType guid = player->GetGUID().GetCounter();
        uint32 currentTime = time(nullptr);

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ONLINE_REWARDS);
        stmt->SetData(0, guid);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            Field* fields = result->Fetch();
            uint32 totalOnlineTime = fields[0].Get<uint32>();
            uint32 lastRewardTime = fields[1].Get<uint32>();
            uint32 lastLoginTime = fields[2].Get<uint32>();

            player->GetSession()->SetOnlineRewardData(totalOnlineTime, lastRewardTime, lastLoginTime);
        }
        else
        {
            CharacterDatabasePreparedStatement* insertStmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ONLINE_REWARDS);
            insertStmt->SetData(0, guid);
            insertStmt->SetData(1, 0);
            insertStmt->SetData(2, 0);
            insertStmt->SetData(3, currentTime);
            CharacterDatabase.Execute(insertStmt);

            player->GetSession()->SetOnlineRewardData(0, 0, currentTime);
        }

        CharacterDatabasePreparedStatement* updateStmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ONLINE_REWARDS_LOGIN);
        updateStmt->SetData(0, currentTime);
        updateStmt->SetData(1, guid);
        CharacterDatabase.Execute(updateStmt);
    }

    void SaveOnlineTimeData(Player* player)
    {
        if (!player)
            return;

        ObjectGuid::LowType guid = player->GetGUID().GetCounter();
        uint32 totalOnlineTime = player->GetSession()->GetTotalOnlineTime();
        uint32 lastRewardTime = player->GetSession()->GetLastRewardTime();

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ONLINE_REWARDS);
        stmt->SetData(0, totalOnlineTime);
        stmt->SetData(1, lastRewardTime);
        stmt->SetData(2, guid);
        CharacterDatabase.Execute(stmt);
    }

    void UpdateOnlineTime(Player* player)
    {
        if (!player)
            return;

        if (!player->IsInWorld() || player->IsBeingTeleported())
            return;

        player->GetSession()->IncrementOnlineTime(1);
    }
};

void AddSC_OnlineRewardsPlayer()
{
    new OnlineRewardsPlayerScript();
}
