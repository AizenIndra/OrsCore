/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiBotBridge.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "Chat.h"
#include "GameTime.h"
#include "Group.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "StringFormat.h"
#include <algorithm>
#include <vector>

#ifdef MOD_PLAYERBOTS
#include "Playerbots.h"
#endif

AiBotBridge* AiBotBridge::instance()
{
    static AiBotBridge bridge;
    return &bridge;
}

bool AiBotBridge::IsPlayerBotsAvailable() const
{
#ifdef MOD_PLAYERBOTS
    return true;
#else
    return false;
#endif
}

TeamId AiBotBridge::SideToTeamId(AiWorld::FactionSide side)
{
    if (side == AiWorld::FactionSide::Alliance)
        return TEAM_ALLIANCE;
    if (side == AiWorld::FactionSide::Horde)
        return TEAM_HORDE;
    return TEAM_NEUTRAL;
}

void AiBotBridge::Update(uint32 /*diff*/)
{
    if (!sAiWorldConfig().IsEnabled() || !sAiWorldConfig().IsBotsEnabled())
        return;

    std::deque<AiWorld::BotOrder> local;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        local.swap(_orders);
    }

    uint32 now = uint32(GameTime::GetGameTime().count());
    for (auto const& order : local)
    {
        if (order.expireUnix && now > order.expireUnix)
            continue;
        ProcessOrder(order);
    }
}

bool AiBotBridge::RequestDeploy(AiWorld::FactionSide side, uint32 zoneId, uint32 count, std::string_view reason)
{
    if (!sAiWorldConfig().IsBotsEnabled())
    {
        LOG_DEBUG("module", "AiWorld: bot deploy skipped (bots disabled)");
        return false;
    }

    uint32 capped = std::min(count, sAiWorldConfig().GetBotsMaxDeploy());
    AiWorld::BotOrder order;
    order.zoneId = zoneId;
    order.orderType = AiWorld::BotOrderType::Deploy;
    order.side = side;
    order.count = capped;
    order.expireUnix = uint32(GameTime::GetGameTime().count()) + 600;
    order.reason = std::string(reason);

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _orders.push_back(order);
    }

    if (sAiWorldConfig().AnnounceWorld())
        ChatHandler(nullptr).SendWorldText(AiWorld::STRING_BOTS_DEPLOY, AiWorld::SideName(side), capped, zoneId);

    sAiWorldMemory->RecordEvent(zoneId, "bot_deploy", 0, side, AiWorld::FactionSide::Neutral,
        std::string(reason));

    LOG_INFO("module", "AiWorld: queued bot deploy {} x{} to zone {} ({})",
        AiWorld::SideName(side), capped, zoneId, reason);
    return true;
}

bool AiBotBridge::RequestDefend(uint32 zoneId, AiWorld::FactionSide side, std::string_view reason)
{
    AiWorld::BotOrder order;
    order.zoneId = zoneId;
    order.orderType = AiWorld::BotOrderType::Defend;
    order.side = side;
    order.count = std::min<uint32>(4, sAiWorldConfig().GetBotsMaxDeploy());
    order.expireUnix = uint32(GameTime::GetGameTime().count()) + 600;
    order.reason = std::string(reason);

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _orders.push_back(order);
    }

    sAiWorldMemory->RecordEvent(zoneId, "bot_defend", 0, side, AiWorld::FactionSide::Neutral,
        std::string(reason));
    return true;
}

bool AiBotBridge::RequestAssist(ObjectGuid playerGuid, uint32 zoneId, AiWorld::FactionSide side)
{
    AiWorld::BotOrder order;
    order.botGuid = playerGuid;
    order.zoneId = zoneId;
    order.orderType = AiWorld::BotOrderType::Assist;
    order.side = side;
    order.count = std::min<uint32>(2, sAiWorldConfig().GetBotsMaxDeploy());
    order.expireUnix = uint32(GameTime::GetGameTime().count()) + 300;
    order.reason = "assist_player";

    std::lock_guard<std::mutex> lock(_mutex);
    _orders.push_back(order);
    return true;
}

std::vector<AiWorld::BotOrder> AiBotBridge::GetPendingOrders() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return { _orders.begin(), _orders.end() };
}

uint32 AiBotBridge::GetPendingCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return uint32(_orders.size());
}

bool AiBotBridge::PrepareBotForZone(Player* bot, uint32 mapId, float x, float y, float z, float o)
{
    if (!bot || !bot->IsInWorld())
        return false;

    if (bot->IsBeingTeleported() || bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue())
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    if (!bot->TeleportTo(mapId, x, y, z, o))
        return false;

#ifdef MOD_PLAYERBOTS
    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        botAI->ChangeStrategy("-rpg,+grind,+pvp", BOT_STATE_NON_COMBAT);
        botAI->Reset(true);
    }

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
        sRandomPlayerbotMgr.Refresh(bot);
#endif
    return true;
}

uint32 AiBotBridge::DeployBotsToZone(AiWorld::FactionSide side, uint32 zoneId, uint32 count)
{
#ifndef MOD_PLAYERBOTS
    (void)side;
    (void)zoneId;
    (void)count;
    LOG_WARN("module", "AiWorld BotBridge: MOD_PLAYERBOTS not compiled; order logged only");
    return 0;
#else
    if (!count)
        return 0;

    TeamId teamId = SideToTeamId(side);
    if (teamId == TEAM_NEUTRAL)
        return 0;

    uint32 mapId = sAiWorldConfig().GetHillsbradMapId();
    float x = sAiWorldConfig().GetHillsbradX();
    float y = sAiWorldConfig().GetHillsbradY();
    float z = sAiWorldConfig().GetHillsbradZ();
    float o = sAiWorldConfig().GetHillsbradO();

    // Prefer candidates already outside the target zone so we actually move reinforcements in.
    std::vector<Player*> preferred;
    std::vector<Player*> fallback;

    PlayerBotMap bots = sRandomPlayerbotMgr.GetAllBots();
    for (auto const& [guid, bot] : bots)
    {
        if (!bot || !bot->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr.IsRandomBot(bot))
            continue;
        if (bot->GetTeamId() != teamId)
            continue;
        if (bot->IsBeingTeleported() || bot->InBattleground() || bot->InArena())
            continue;
        if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
            continue;

        if (bot->GetZoneId() != zoneId)
            preferred.push_back(bot);
        else
            fallback.push_back(bot);
    }

    std::vector<Player*> selected;
    selected.reserve(count);
    for (Player* bot : preferred)
    {
        if (selected.size() >= count)
            break;
        selected.push_back(bot);
    }
    for (Player* bot : fallback)
    {
        if (selected.size() >= count)
            break;
        selected.push_back(bot);
    }

    uint32 moved = 0;
    for (size_t i = 0; i < selected.size(); ++i)
    {
        float ox = x + float(i) * 2.5f;
        float oy = y + float(i % 3) * 2.0f;
        if (PrepareBotForZone(selected[i], mapId, ox, oy, z, o))
            ++moved;
    }

    LOG_INFO("module", "AiWorld BotBridge: deployed {} / {} {} bots to zone {}",
        moved, count, AiWorld::SideName(side), zoneId);
    return moved;
#endif
}

uint32 AiBotBridge::AssistPlayer(ObjectGuid playerGuid, AiWorld::FactionSide side, uint32 count)
{
#ifndef MOD_PLAYERBOTS
    (void)playerGuid;
    (void)side;
    (void)count;
    return 0;
#else
    Player* target = ObjectAccessor::FindPlayer(playerGuid);
    if (!target || !target->IsInWorld())
        return 0;

    TeamId teamId = SideToTeamId(side);
    if (teamId == TEAM_NEUTRAL)
        teamId = target->GetTeamId();

    PlayerBotMap bots = sRandomPlayerbotMgr.GetAllBots();
    uint32 moved = 0;
    for (auto const& [guid, bot] : bots)
    {
        if (moved >= count)
            break;
        if (!bot || !bot->IsInWorld() || bot == target)
            continue;
        if (!sRandomPlayerbotMgr.IsRandomBot(bot))
            continue;
        if (bot->GetTeamId() != teamId)
            continue;
        if (bot->IsBeingTeleported() || bot->InBattleground() || bot->InArena())
            continue;
        if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
            continue;

        float ox = target->GetPositionX() + float(moved) * 2.0f;
        float oy = target->GetPositionY() + float(moved % 2) * 2.0f;
        if (!PrepareBotForZone(bot, target->GetMapId(), ox, oy, target->GetPositionZ(), target->GetOrientation()))
            continue;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        {
            botAI->SetMaster(target);
            botAI->ChangeStrategy("+follow,+grind,+pvp", BOT_STATE_NON_COMBAT);
        }
        ++moved;
    }

    LOG_INFO("module", "AiWorld BotBridge: assisted {} with {} bots", target->GetName(), moved);
    return moved;
#endif
}

void AiBotBridge::ProcessOrder(AiWorld::BotOrder const& order)
{
    LOG_INFO("module", "AiWorld BotBridge: process order type={} side={} zone={} count={} reason={}",
        uint32(order.orderType), AiWorld::SideName(order.side), order.zoneId, order.count, order.reason);

    uint32 moved = 0;
    switch (order.orderType)
    {
        case AiWorld::BotOrderType::Deploy:
        case AiWorld::BotOrderType::Defend:
        case AiWorld::BotOrderType::Attack:
            moved = DeployBotsToZone(order.side, order.zoneId, order.count);
            break;
        case AiWorld::BotOrderType::Assist:
            moved = AssistPlayer(order.botGuid, order.side, order.count ? order.count : 1);
            break;
        case AiWorld::BotOrderType::Group:
            // Group forming left for a later phase; deploy nearby as a soft stand-in.
            moved = DeployBotsToZone(order.side, order.zoneId, order.count);
            break;
        default:
            break;
    }

    sAiWorldMemory->RecordEvent(order.zoneId, "bot_order", uint32(order.orderType), order.side,
        AiWorld::FactionSide::Neutral,
        Acore::StringFormat("{{\"reason\":\"{}\",\"moved\":{}}}", order.reason, moved));
}
