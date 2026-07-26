/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_LLM_CLIENT_H
#define MOD_AIWORLD_LLM_CLIENT_H

#include "AiWorldCommon.h"
#include <string>

class AiLlmClient
{
public:
    static AiLlmClient* instance();

    [[nodiscard]] bool IsAvailable() const;
    void NotifyFailure();
    void NotifySuccess();

    // Blocking HTTP call — must run on worker thread only.
    [[nodiscard]] AiWorld::LlmProposal ProposeScenario(AiWorld::ZoneSnapshot const& snap,
        std::string const& recentEventsJson) const;

    [[nodiscard]] std::string GenerateDialogue(std::string const& context) const;

private:
    AiLlmClient() = default;

    [[nodiscard]] bool ParseEndpoint(std::string& host, std::string& port, std::string& basePath) const;
    [[nodiscard]] std::string HttpPostJson(std::string const& path, std::string const& body) const;
    [[nodiscard]] AiWorld::LlmProposal ParseProposal(std::string const& response) const;
    [[nodiscard]] static std::string ExtractJsonObject(std::string const& text);

    mutable uint32 _failCount = 0;
    mutable uint32 _disabledUntil = 0;
};

#define sAiLlmClient AiLlmClient::instance()

#endif
