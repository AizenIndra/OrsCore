/*
 * Online Rewards System
 * Tracks player online time and provides rewards through LuckyWheel
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "Chat.h"

// Required online time for reward (1 hour = 3600 seconds)
#define REQUIRED_ONLINE_TIME 10

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

        // Load online time data from database
        LoadOnlineTimeData(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!player)
            return;

        // Save online time data to database
        SaveOnlineTimeData(player);
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!player)
            return;

        // Update online time every second (when diff accumulates)
        static std::unordered_map<ObjectGuid, uint32> updateTimers;
        
        ObjectGuid guid = player->GetGUID();
        updateTimers[guid] += diff;

        // Update every 1 second (1000ms)
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

        // Save online time data periodically
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

            // Store in player's session for quick access
            player->GetSession()->SetOnlineRewardData(totalOnlineTime, lastRewardTime, lastLoginTime);
        }
        else
        {
            // Create new record
            CharacterDatabasePreparedStatement* insertStmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ONLINE_REWARDS);
            insertStmt->SetData(0, guid);
            insertStmt->SetData(1, 0); // total_online_time
            insertStmt->SetData(2, 0); // last_reward_time
            insertStmt->SetData(3, currentTime); // last_login_time
            CharacterDatabase.Execute(insertStmt);

            player->GetSession()->SetOnlineRewardData(0, 0, currentTime);
        }

        // Update last login time
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

        // Only count time when player is actually in world (not loading, not in character selection)
        if (!player->IsInWorld() || player->IsBeingTeleported())
            return;

        // Increment online time
        player->GetSession()->IncrementOnlineTime(1);
    }
};

void AddSC_OnlineRewardsPlayer()
{
    new OnlineRewardsPlayerScript();
}
