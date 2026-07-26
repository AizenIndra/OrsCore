/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiDirector.h"
#include "AiFactionScore.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "AiZoneAnalyzer.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"

class AiWorld_PlayerScript : public PlayerScript
{
public:
    AiWorld_PlayerScript() : PlayerScript("AiWorld_PlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_UPDATE_ZONE,
        PLAYERHOOK_ON_PVP_KILL
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sAiWorldConfig().IsEnabled() || !player)
            return;

        uint32 zoneId = player->GetZoneId();
        if (!sAiWorldConfig().IsWatchedZone(zoneId))
            return;

        sAiZoneAnalyzer->OnPlayerEnterZone(player->GetGUID(), zoneId,
            AiWorld::TeamIdToSide(player->GetTeamId()), false);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!sAiWorldConfig().IsEnabled() || !player)
            return;

        sAiZoneAnalyzer->OnPlayerLeaveZone(player->GetGUID(), player->GetZoneId());
    }

    void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (!sAiWorldConfig().IsEnabled() || !player)
            return;

        sAiZoneAnalyzer->OnPlayerLeaveZone(player->GetGUID(), 0);
        if (sAiWorldConfig().IsWatchedZone(newZone))
            sAiZoneAnalyzer->OnPlayerEnterZone(player->GetGUID(), newZone,
                AiWorld::TeamIdToSide(player->GetTeamId()), false);
    }

    void OnPlayerPVPKill(Player* killer, Player* killed) override
    {
        if (!sAiWorldConfig().IsEnabled() || !killer || !killed)
            return;

        uint32 zoneId = killed->GetZoneId();
        if (!sAiWorldConfig().IsWatchedZone(zoneId))
            return;

        AiWorld::FactionSide side = AiWorld::TeamIdToSide(killer->GetTeamId());
        sAiZoneAnalyzer->OnPvpKill(zoneId, side);
        sAiFactionScore->AddKill(side);
        sAiWorldMemory->RecordPlayerAction(killer->GetGUID().GetCounter(), zoneId, "pvp_kill", side);
    }
};

class AiWorld_WorldScript : public WorldScript
{
public:
    AiWorld_WorldScript() : WorldScript("AiWorld_WorldScript", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP,
        WORLDHOOK_ON_SHUTDOWN,
        WORLDHOOK_ON_UPDATE
    }) { }

    void OnBeforeConfigLoad(bool reload) override
    {
        sAiWorldConfig().Initialize(reload);
    }

    void OnStartup() override
    {
        if (!sAiWorldConfig().IsEnabled())
        {
            LOG_INFO("module", ">> AiWorld module loaded (disabled in config)");
            return;
        }

        sAiDirector->Initialize();
        LOG_INFO("module", ">> AiWorld module started");
    }

    void OnShutdown() override
    {
        sAiDirector->Shutdown();
    }

    void OnUpdate(uint32 diff) override
    {
        sAiDirector->Update(diff);
    }
};

void AddSC_AiWorldCore()
{
    new AiWorld_PlayerScript();
    new AiWorld_WorldScript();
}
