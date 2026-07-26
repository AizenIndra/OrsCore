/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiWorldConfig.h"
#include "AiWorldCommon.h"
#include "Tokenize.h"
#include <cstdlib>

AiWorldConfigData::AiWorldConfigData()
    : ConfigValueCache(AiWorldConfigOption::NUM_CONFIGS)
{
}

void AiWorldConfigData::BuildConfigCache()
{
    SetConfigValue<bool>(AiWorldConfigOption::Enable, "AiWorld.Enable", false);
    SetConfigValue<uint32>(AiWorldConfigOption::DirectorIntervalSec, "AiWorld.Director.IntervalSec", 300);
    SetConfigValue<uint32>(AiWorldConfigOption::MetricsSampleSec, "AiWorld.Metrics.SampleSec", 30);
    SetConfigValue<std::string>(AiWorldConfigOption::Zones, "AiWorld.Zones", std::string("267"));
    SetConfigValue<uint32>(AiWorldConfigOption::ImbalanceThreshold, "AiWorld.Director.ImbalanceThreshold", 3);
    SetConfigValue<uint32>(AiWorldConfigOption::ControlFlipThreshold, "AiWorld.Director.ControlFlipThreshold", 50);
    SetConfigValue<uint32>(AiWorldConfigOption::ActionCooldownSec, "AiWorld.Director.ActionCooldownSec", 600);
    SetConfigValue<bool>(AiWorldConfigOption::BotsEnable, "AiWorld.Bots.Enable", true);
    SetConfigValue<uint32>(AiWorldConfigOption::BotsMaxDeploy, "AiWorld.Bots.MaxDeploy", 8);
    SetConfigValue<uint32>(AiWorldConfigOption::EventsMaxConcurrent, "AiWorld.Events.MaxConcurrent", 2);
    SetConfigValue<uint32>(AiWorldConfigOption::EventDurationSec, "AiWorld.Events.DurationSec", 900);
    SetConfigValue<bool>(AiWorldConfigOption::LlmEnable, "AiWorld.Llm.Enable", false);
    SetConfigValue<std::string>(AiWorldConfigOption::LlmEndpoint, "AiWorld.Llm.Endpoint",
        std::string("http://127.0.0.1:11434"));
    SetConfigValue<std::string>(AiWorldConfigOption::LlmModel, "AiWorld.Llm.Model", std::string("llama3.2"));
    SetConfigValue<uint32>(AiWorldConfigOption::LlmTimeoutMs, "AiWorld.Llm.TimeoutMs", 8000);
    SetConfigValue<uint32>(AiWorldConfigOption::LlmMaxConcurrent, "AiWorld.Llm.MaxConcurrent", 1);
    SetConfigValue<uint32>(AiWorldConfigOption::LlmFailCooldownSec, "AiWorld.Llm.FailCooldownSec", 300);
    SetConfigValue<uint32>(AiWorldConfigOption::TaskMaxQueue, "AiWorld.Task.MaxQueue", 32);
    SetConfigValue<uint32>(AiWorldConfigOption::MemoryRetentionDays, "AiWorld.Memory.RetentionDays", 30);
    SetConfigValue<uint32>(AiWorldConfigOption::HistoryCacheLimit, "AiWorld.Memory.HistoryCacheLimit", 100);
    SetConfigValue<bool>(AiWorldConfigOption::AnnounceWorld, "AiWorld.AnnounceWorld", true);
    SetConfigValue<bool>(AiWorldConfigOption::HillsbradEnable, "AiWorld.Prototype.Hillsbrad.Enable", true);
    SetConfigValue<uint32>(AiWorldConfigOption::HillsbradMapId, "AiWorld.Prototype.Hillsbrad.MapId", 0);
    SetConfigValue<float>(AiWorldConfigOption::HillsbradX, "AiWorld.Prototype.Hillsbrad.X", -300.0f);
    SetConfigValue<float>(AiWorldConfigOption::HillsbradY, "AiWorld.Prototype.Hillsbrad.Y", -700.0f);
    SetConfigValue<float>(AiWorldConfigOption::HillsbradZ, "AiWorld.Prototype.Hillsbrad.Z", 55.0f);
    SetConfigValue<float>(AiWorldConfigOption::HillsbradO, "AiWorld.Prototype.Hillsbrad.O", 0.0f);
    SetConfigValue<uint32>(AiWorldConfigOption::SkirmishCreatureEntry, "AiWorld.Prototype.Hillsbrad.CreatureEntry", 0);
    SetConfigValue<uint32>(AiWorldConfigOption::SkirmishCreatureCount, "AiWorld.Prototype.Hillsbrad.CreatureCount", 0);

    RebuildWatchedZones();
}

void AiWorldConfigData::RebuildWatchedZones()
{
    _watchedZones.clear();
    std::string zones = std::string(GetConfigValue(AiWorldConfigOption::Zones));
    for (std::string_view token : Acore::Tokenize(zones, ',', false))
    {
        if (token.empty())
            continue;
        uint32 zoneId = uint32(std::strtoul(std::string(token).c_str(), nullptr, 10));
        if (zoneId)
            _watchedZones.insert(zoneId);
    }

    if (_watchedZones.empty())
        _watchedZones.insert(AiWorld::ZONE_HILLSBRAD_FOOTHILLS);
}

bool AiWorldConfigData::IsEnabled() const
{
    return GetConfigValue<bool>(AiWorldConfigOption::Enable);
}

uint32 AiWorldConfigData::GetDirectorIntervalSec() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::DirectorIntervalSec);
}

uint32 AiWorldConfigData::GetMetricsSampleSec() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::MetricsSampleSec);
}

bool AiWorldConfigData::IsWatchedZone(uint32 zoneId) const
{
    return _watchedZones.find(zoneId) != _watchedZones.end();
}

uint32 AiWorldConfigData::GetImbalanceThreshold() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::ImbalanceThreshold);
}

uint32 AiWorldConfigData::GetControlFlipThreshold() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::ControlFlipThreshold);
}

uint32 AiWorldConfigData::GetActionCooldownSec() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::ActionCooldownSec);
}

bool AiWorldConfigData::IsBotsEnabled() const
{
    return GetConfigValue<bool>(AiWorldConfigOption::BotsEnable);
}

uint32 AiWorldConfigData::GetBotsMaxDeploy() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::BotsMaxDeploy);
}

uint32 AiWorldConfigData::GetEventsMaxConcurrent() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::EventsMaxConcurrent);
}

uint32 AiWorldConfigData::GetEventDurationSec() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::EventDurationSec);
}

bool AiWorldConfigData::IsLlmEnabled() const
{
    return GetConfigValue<bool>(AiWorldConfigOption::LlmEnable);
}

std::string_view AiWorldConfigData::GetLlmEndpoint() const
{
    return GetConfigValue(AiWorldConfigOption::LlmEndpoint);
}

std::string_view AiWorldConfigData::GetLlmModel() const
{
    return GetConfigValue(AiWorldConfigOption::LlmModel);
}

uint32 AiWorldConfigData::GetLlmTimeoutMs() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::LlmTimeoutMs);
}

uint32 AiWorldConfigData::GetLlmMaxConcurrent() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::LlmMaxConcurrent);
}

uint32 AiWorldConfigData::GetLlmFailCooldownSec() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::LlmFailCooldownSec);
}

uint32 AiWorldConfigData::GetTaskMaxQueue() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::TaskMaxQueue);
}

uint32 AiWorldConfigData::GetMemoryRetentionDays() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::MemoryRetentionDays);
}

uint32 AiWorldConfigData::GetHistoryCacheLimit() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::HistoryCacheLimit);
}

bool AiWorldConfigData::AnnounceWorld() const
{
    return GetConfigValue<bool>(AiWorldConfigOption::AnnounceWorld);
}

bool AiWorldConfigData::IsHillsbradEnabled() const
{
    return GetConfigValue<bool>(AiWorldConfigOption::HillsbradEnable);
}

uint32 AiWorldConfigData::GetHillsbradMapId() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::HillsbradMapId);
}

float AiWorldConfigData::GetHillsbradX() const
{
    return GetConfigValue<float>(AiWorldConfigOption::HillsbradX);
}

float AiWorldConfigData::GetHillsbradY() const
{
    return GetConfigValue<float>(AiWorldConfigOption::HillsbradY);
}

float AiWorldConfigData::GetHillsbradZ() const
{
    return GetConfigValue<float>(AiWorldConfigOption::HillsbradZ);
}

float AiWorldConfigData::GetHillsbradO() const
{
    return GetConfigValue<float>(AiWorldConfigOption::HillsbradO);
}

uint32 AiWorldConfigData::GetSkirmishCreatureEntry() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::SkirmishCreatureEntry);
}

uint32 AiWorldConfigData::GetSkirmishCreatureCount() const
{
    return GetConfigValue<uint32>(AiWorldConfigOption::SkirmishCreatureCount);
}

AiWorldConfigData& sAiWorldConfig()
{
    static AiWorldConfigData instance;
    return instance;
}
