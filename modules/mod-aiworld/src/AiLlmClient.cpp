/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiLlmClient.h"
#include "AiWorldConfig.h"
#include "AiWorldMemory.h"
#include "GameTime.h"
#include "Log.h"
#include "StringFormat.h"
#include <boost/asio/ip/tcp.hpp>
#include <cctype>
#include <sstream>

AiLlmClient* AiLlmClient::instance()
{
    static AiLlmClient client;
    return &client;
}

bool AiLlmClient::IsAvailable() const
{
    if (!sAiWorldConfig().IsLlmEnabled())
        return false;
    return uint32(GameTime::GetGameTime().count()) >= _disabledUntil;
}

void AiLlmClient::NotifyFailure()
{
    ++_failCount;
    if (_failCount >= 3)
    {
        _disabledUntil = uint32(GameTime::GetGameTime().count()) + sAiWorldConfig().GetLlmFailCooldownSec();
        _failCount = 0;
        LOG_WARN("module", "AiWorld LLM circuit breaker open until {}", _disabledUntil);
    }
}

void AiLlmClient::NotifySuccess()
{
    _failCount = 0;
}

bool AiLlmClient::ParseEndpoint(std::string& host, std::string& port, std::string& basePath) const
{
    std::string endpoint(sAiWorldConfig().GetLlmEndpoint());
    basePath.clear();

    auto strip = endpoint.find("://");
    if (strip != std::string::npos)
        endpoint = endpoint.substr(strip + 3);

    auto slash = endpoint.find('/');
    if (slash != std::string::npos)
    {
        basePath = endpoint.substr(slash);
        endpoint = endpoint.substr(0, slash);
    }

    auto colon = endpoint.find(':');
    if (colon == std::string::npos)
    {
        host = endpoint;
        port = "11434";
    }
    else
    {
        host = endpoint.substr(0, colon);
        port = endpoint.substr(colon + 1);
    }

    return !host.empty();
}

std::string AiLlmClient::HttpPostJson(std::string const& path, std::string const& body) const
{
    std::string host, port, basePath;
    if (!ParseEndpoint(host, port, basePath))
        return {};

    std::string fullPath = basePath.empty() ? path : (basePath + path);
    try
    {
        boost::asio::ip::tcp::iostream stream;
        stream.connect(host, port);
        if (!stream)
        {
            LOG_ERROR("module", "AiWorld LLM connect failed {}:{}", host, port);
            return {};
        }

        stream << "POST " << fullPath << " HTTP/1.1\r\n"
               << "Host: " << host << "\r\n"
               << "Content-Type: application/json\r\n"
               << "Content-Length: " << body.size() << "\r\n"
               << "Connection: close\r\n\r\n"
               << body;

        std::string httpVersion;
        unsigned status = 0;
        std::string statusMsg;
        stream >> httpVersion >> status;
        std::getline(stream, statusMsg);

        std::string header;
        while (std::getline(stream, header) && header != "\r")
            ;

        std::ostringstream response;
        response << stream.rdbuf();
        if (status < 200 || status >= 300)
        {
            LOG_ERROR("module", "AiWorld LLM HTTP {}", status);
            return {};
        }
        return response.str();
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("module", "AiWorld LLM exception: {}", e.what());
        return {};
    }
}

std::string AiLlmClient::ExtractJsonObject(std::string const& text)
{
    auto start = text.find('{');
    auto end = text.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start)
        return {};
    return text.substr(start, end - start + 1);
}

static std::string ExtractStringField(std::string const& json, std::string const& key)
{
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos)
        return {};
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return {};
    pos = json.find('"', pos);
    if (pos == std::string::npos)
        return {};
    ++pos;
    std::string out;
    for (; pos < json.size(); ++pos)
    {
        if (json[pos] == '\\' && pos + 1 < json.size())
        {
            out.push_back(json[pos + 1]);
            ++pos;
            continue;
        }
        if (json[pos] == '"')
            break;
        out.push_back(json[pos]);
    }
    return out;
}

static uint32 ExtractUintField(std::string const& json, std::string const& key)
{
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos)
        return 0;
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return 0;
    ++pos;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos])))
        ++pos;
    return uint32(std::strtoul(json.c_str() + pos, nullptr, 10));
}

AiWorld::LlmProposal AiLlmClient::ParseProposal(std::string const& response) const
{
    AiWorld::LlmProposal proposal;
    std::string responseField = ExtractStringField(response, "response");
    std::string json = ExtractJsonObject(responseField.empty() ? response : responseField);
    if (json.empty())
        return proposal;

    std::string action = ExtractStringField(json, "action");
    if (action == "ChangeTerritory")
        proposal.action = AiWorld::DirectorActionType::ChangeTerritory;
    else if (action == "DeployBots")
        proposal.action = AiWorld::DirectorActionType::DeployBots;
    else if (action == "StartEvent")
        proposal.action = AiWorld::DirectorActionType::StartEvent;
    else if (action == "Announce")
        proposal.action = AiWorld::DirectorActionType::Announce;
    else if (action == "AdjustTension")
        proposal.action = AiWorld::DirectorActionType::AdjustTension;
    else
        return proposal;

    std::string beneficiary = ExtractStringField(json, "beneficiary");
    if (beneficiary == "Alliance")
        proposal.beneficiary = AiWorld::FactionSide::Alliance;
    else if (beneficiary == "Horde")
        proposal.beneficiary = AiWorld::FactionSide::Horde;
    else
        proposal.beneficiary = AiWorld::FactionSide::Neutral;

    proposal.botCount = ExtractUintField(json, "botCount");
    proposal.eventTemplateId = ExtractUintField(json, "eventTemplateId");
    proposal.reason = ExtractStringField(json, "reason");
    proposal.dialogue = ExtractStringField(json, "dialogue");
    proposal.valid = true;
    return proposal;
}

AiWorld::LlmProposal AiLlmClient::ProposeScenario(AiWorld::ZoneSnapshot const& snap,
    std::string const& recentEventsJson) const
{
    AiWorld::LlmProposal empty;
    if (!IsAvailable())
        return empty;

    std::string cacheKey = Acore::StringFormat("propose:{}:{}:{}:{}:{}",
        snap.zoneId, snap.alliancePlayers, snap.hordePlayers, snap.controlScore, snap.tension);
    if (auto cached = sAiWorldMemory->GetLlmCache(cacheKey))
        return ParseProposal(*cached);

    std::string prompt = Acore::StringFormat(
        "You are an MMORPG world director. Reply with ONLY JSON: "
        "{{\"action\":\"ChangeTerritory|DeployBots|StartEvent|Announce\",\"beneficiary\":\"Alliance|Horde|Neutral\","
        "\"botCount\":0,\"eventTemplateId\":0,\"reason\":\"short\",\"dialogue\":null}}. "
        "Zone {} AlliancePlayers {} HordePlayers {} AllianceBots {} HordeBots {} "
        "PvPAlli {} PvPHorde {} control {} tension {} score {} recent {}.",
        snap.zoneId, snap.alliancePlayers, snap.hordePlayers, snap.allianceBots, snap.hordeBots,
        snap.pvpKillsAlliance, snap.pvpKillsHorde, AiWorld::TerritoryName(snap.control),
        snap.tension, snap.controlScore, recentEventsJson);

    std::string model(sAiWorldConfig().GetLlmModel());
    std::string escapedPrompt;
    for (char c : prompt)
    {
        if (c == '"' || c == '\\')
            escapedPrompt.push_back('\\');
        if (c == '\n')
        {
            escapedPrompt += "\\n";
            continue;
        }
        escapedPrompt.push_back(c);
    }

    std::string body = Acore::StringFormat(
        "{{\"model\":\"{}\",\"prompt\":\"{}\",\"stream\":false,\"format\":\"json\"}}",
        model, escapedPrompt);

    std::string response = HttpPostJson("/api/generate", body);
    if (response.empty())
        return empty;

    sAiWorldMemory->SetLlmCache(cacheKey, response, 600);
    return ParseProposal(response);
}

std::string AiLlmClient::GenerateDialogue(std::string const& context) const
{
    if (!IsAvailable())
        return {};

    std::string escaped;
    for (char c : context)
    {
        if (c == '"' || c == '\\')
            escaped.push_back('\\');
        escaped.push_back(c);
    }

    std::string body = Acore::StringFormat(
        "{{\"model\":\"{}\",\"prompt\":\"Write one short NPC line for: {}. Reply with plain text.\",\"stream\":false}}",
        std::string(sAiWorldConfig().GetLlmModel()), escaped);

    std::string response = HttpPostJson("/api/generate", body);
    return ExtractStringField(response, "response");
}
