/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_CONFIG_H
#define MOD_AIWORLD_CONFIG_H

#include "ConfigValueCache.h"
#include <string>
#include <unordered_set>
#include <vector>

enum class AiWorldConfigOption
{
    Enable,
    DirectorIntervalSec,
    MetricsSampleSec,
    Zones,
    ImbalanceThreshold,
    ControlFlipThreshold,
    ActionCooldownSec,
    BotsEnable,
    BotsMaxDeploy,
    EventsMaxConcurrent,
    EventDurationSec,
    LlmEnable,
    LlmEndpoint,
    LlmModel,
    LlmTimeoutMs,
    LlmMaxConcurrent,
    LlmFailCooldownSec,
    TaskMaxQueue,
    MemoryRetentionDays,
    HistoryCacheLimit,
    AnnounceWorld,
    HillsbradEnable,
    HillsbradMapId,
    HillsbradX,
    HillsbradY,
    HillsbradZ,
    HillsbradO,
    SkirmishCreatureEntry,
    SkirmishCreatureCount,
    NUM_CONFIGS
};

class AiWorldConfigData : public ConfigValueCache<AiWorldConfigOption>
{
public:
    AiWorldConfigData();

    void BuildConfigCache() override;

    [[nodiscard]] bool IsEnabled() const;
    [[nodiscard]] uint32 GetDirectorIntervalSec() const;
    [[nodiscard]] uint32 GetMetricsSampleSec() const;
    [[nodiscard]] std::unordered_set<uint32> const& GetWatchedZones() const { return _watchedZones; }
    [[nodiscard]] bool IsWatchedZone(uint32 zoneId) const;
    [[nodiscard]] uint32 GetImbalanceThreshold() const;
    [[nodiscard]] uint32 GetControlFlipThreshold() const;
    [[nodiscard]] uint32 GetActionCooldownSec() const;
    [[nodiscard]] bool IsBotsEnabled() const;
    [[nodiscard]] uint32 GetBotsMaxDeploy() const;
    [[nodiscard]] uint32 GetEventsMaxConcurrent() const;
    [[nodiscard]] uint32 GetEventDurationSec() const;
    [[nodiscard]] bool IsLlmEnabled() const;
    [[nodiscard]] std::string_view GetLlmEndpoint() const;
    [[nodiscard]] std::string_view GetLlmModel() const;
    [[nodiscard]] uint32 GetLlmTimeoutMs() const;
    [[nodiscard]] uint32 GetLlmMaxConcurrent() const;
    [[nodiscard]] uint32 GetLlmFailCooldownSec() const;
    [[nodiscard]] uint32 GetTaskMaxQueue() const;
    [[nodiscard]] uint32 GetMemoryRetentionDays() const;
    [[nodiscard]] uint32 GetHistoryCacheLimit() const;
    [[nodiscard]] bool AnnounceWorld() const;
    [[nodiscard]] bool IsHillsbradEnabled() const;
    [[nodiscard]] uint32 GetHillsbradMapId() const;
    [[nodiscard]] float GetHillsbradX() const;
    [[nodiscard]] float GetHillsbradY() const;
    [[nodiscard]] float GetHillsbradZ() const;
    [[nodiscard]] float GetHillsbradO() const;
    [[nodiscard]] uint32 GetSkirmishCreatureEntry() const;
    [[nodiscard]] uint32 GetSkirmishCreatureCount() const;

private:
    void RebuildWatchedZones();

    std::unordered_set<uint32> _watchedZones;
};

AiWorldConfigData& sAiWorldConfig();

#endif
