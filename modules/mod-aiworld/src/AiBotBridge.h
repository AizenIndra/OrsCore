/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_BOT_BRIDGE_H
#define MOD_AIWORLD_BOT_BRIDGE_H

#include "AiWorldCommon.h"
#include "SharedDefines.h"
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

class Player;

class AiBotBridge
{
public:
    static AiBotBridge* instance();

    void Update(uint32 diff);
    bool RequestDeploy(AiWorld::FactionSide side, uint32 zoneId, uint32 count, std::string_view reason);
    bool RequestDefend(uint32 zoneId, AiWorld::FactionSide side, std::string_view reason);
    bool RequestAssist(ObjectGuid playerGuid, uint32 zoneId, AiWorld::FactionSide side);

    [[nodiscard]] std::vector<AiWorld::BotOrder> GetPendingOrders() const;
    [[nodiscard]] uint32 GetPendingCount() const;
    [[nodiscard]] bool IsPlayerBotsAvailable() const;

private:
    AiBotBridge() = default;

    void ProcessOrder(AiWorld::BotOrder const& order);
    uint32 DeployBotsToZone(AiWorld::FactionSide side, uint32 zoneId, uint32 count);
    uint32 AssistPlayer(ObjectGuid playerGuid, AiWorld::FactionSide side, uint32 count);
    bool PrepareBotForZone(Player* bot, uint32 mapId, float x, float y, float z, float o);
    [[nodiscard]] static TeamId SideToTeamId(AiWorld::FactionSide side);

    mutable std::mutex _mutex;
    std::deque<AiWorld::BotOrder> _orders;
};

#define sAiBotBridge AiBotBridge::instance()

#endif
