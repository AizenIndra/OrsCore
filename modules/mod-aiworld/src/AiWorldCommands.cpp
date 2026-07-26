/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiDirector.h"
#include "AiEventScheduler.h"
#include "AiWorldCommon.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "AiZoneAnalyzer.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Player.h"

using namespace Acore::ChatCommands;

class AiWorld_CommandScript : public CommandScript
{
public:
    AiWorld_CommandScript() : CommandScript("AiWorld_CommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable simulateTable =
        {
            { "imbalance", HandleSimulateImbalance, SEC_ADMINISTRATOR, Console::Yes },
            { "clear",     HandleSimulateClear,     SEC_ADMINISTRATOR, Console::Yes },
        };

        static ChatCommandTable aiworldTable =
        {
            { "status",   HandleStatus,   SEC_PLAYER,         Console::Yes },
            { "analyze",  HandleAnalyze,  SEC_GAMEMASTER,     Console::Yes },
            { "event",    HandleEvent,    SEC_GAMEMASTER,     Console::Yes },
            { "tension",  HandleTension,  SEC_GAMEMASTER,     Console::Yes },
            { "memory",   HandleMemory,   SEC_GAMEMASTER,     Console::Yes },
            { "territory",HandleTerritory,SEC_ADMINISTRATOR,  Console::Yes },
            { "simulate", simulateTable },
            { "reload",   HandleReload,   SEC_ADMINISTRATOR,  Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "aiworld", aiworldTable },
        };

        return commandTable;
    }

    static bool HandleStatus(ChatHandler* handler)
    {
        if (!sAiWorldConfig().IsEnabled())
        {
            handler->PSendSysMessage(AiWorld::STRING_DISABLED);
            return true;
        }

        handler->SendSysMessage(sAiDirector->BuildStatusText());
        return true;
    }

    static bool HandleAnalyze(ChatHandler* handler, Optional<uint32> zoneId)
    {
        uint32 zone = zoneId.value_or(AiWorld::ZONE_HILLSBRAD_FOOTHILLS);
        sAiDirector->ForceAnalyze(zone);
        handler->PSendSysMessage("AiWorld: forced analyze for zone {}", zone);
        handler->SendSysMessage(sAiDirector->BuildStatusText());
        return true;
    }

    static bool HandleEvent(ChatHandler* handler, Optional<uint32> zoneId)
    {
        uint32 zone = zoneId.value_or(AiWorld::ZONE_HILLSBRAD_FOOTHILLS);
        bool ok = sAiEventScheduler->StartEvent(AiWorld::EVENT_HILLSBRAD_SKIRMISH, zone,
            AiWorld::FactionSide::Neutral, "gm_command");
        handler->PSendSysMessage("AiWorld: start event {}", ok ? "ok" : "failed");
        return true;
    }

    static bool HandleTension(ChatHandler* handler, Optional<uint32> value)
    {
        if (!value)
        {
            handler->PSendSysMessage("World tension: {}", sAiDirector->GetWorldTension());
            return true;
        }

        sAiDirector->SetWorldTension(*value);
        handler->PSendSysMessage("World tension set to {}", sAiDirector->GetWorldTension());
        return true;
    }

    static bool HandleMemory(ChatHandler* handler, Optional<uint32> zoneId)
    {
        uint32 zone = zoneId.value_or(AiWorld::ZONE_HILLSBRAD_FOOTHILLS);
        auto history = sAiWorldMemory->GetRecentHistory(zone, 10);
        handler->PSendSysMessage("AiWorld memory for zone {} ({} events):", zone, history.size());
        for (auto const& rec : history)
            handler->PSendSysMessage("  [{}] {} winner={} {}", rec.createdAt, rec.eventType,
                AiWorld::SideName(rec.winner), rec.payloadJson);
        return true;
    }

    static bool HandleTerritory(ChatHandler* handler, uint32 zoneId, std::string sideName)
    {
        AiWorld::FactionSide side = AiWorld::FactionSide::Neutral;
        if (sideName == "alliance" || sideName == "Alliance")
            side = AiWorld::FactionSide::Alliance;
        else if (sideName == "horde" || sideName == "Horde")
            side = AiWorld::FactionSide::Horde;
        else if (sideName == "contested" || sideName == "Contested" || sideName == "neutral")
            side = AiWorld::FactionSide::Neutral;
        else
        {
            handler->SendSysMessage("Usage: .aiworld territory <zone> <alliance|horde|contested>");
            return false;
        }

        AiWorld::DirectorAction action;
        action.type = AiWorld::DirectorActionType::ChangeTerritory;
        action.zoneId = zoneId;
        action.beneficiary = side;
        action.reason = "gm_territory";
        sAiDirector->ApplyAction(action);
        handler->PSendSysMessage("Territory zone {} set to {}", zoneId, AiWorld::SideName(side));
        return true;
    }

    static bool HandleSimulateImbalance(ChatHandler* handler, std::string sideName, Optional<uint32> magnitude)
    {
        AiWorld::FactionSide side = AiWorld::FactionSide::Horde;
        if (sideName == "alliance" || sideName == "Alliance")
            side = AiWorld::FactionSide::Alliance;
        else if (sideName == "horde" || sideName == "Horde")
            side = AiWorld::FactionSide::Horde;
        else
        {
            handler->SendSysMessage("Usage: .aiworld simulate imbalance <alliance|horde> [magnitude]");
            return false;
        }

        uint32 mag = magnitude.value_or(8);
        sAiDirector->SimulateImbalance(AiWorld::ZONE_HILLSBRAD_FOOTHILLS, side, mag);
        handler->PSendSysMessage("Simulated {} dominance magnitude {} in Hillsbrad. Run .aiworld analyze",
            AiWorld::SideName(side), mag);
        return true;
    }

    static bool HandleSimulateClear(ChatHandler* handler)
    {
        sAiZoneAnalyzer->ClearSimulation(AiWorld::ZONE_HILLSBRAD_FOOTHILLS);
        handler->SendSysMessage("Cleared Hillsbrad simulation bots.");
        return true;
    }

    static bool HandleReload(ChatHandler* handler)
    {
        sAiWorldMemory->LoadFromDB();
        handler->SendSysMessage("AiWorld memory reloaded from DB.");
        return true;
    }
};

void AddSC_AiWorldCommands()
{
    new AiWorld_CommandScript();
}
