-- Mak'Gora client stub (WotLK 3.3.5)
-- Talks to mod-makgora via AddonMessage prefix configured as "MakGora".

local ADDON_PREFIX = "MakGora"

local frame = CreateFrame("Frame")
frame:RegisterEvent("CHAT_MSG_ADDON")
frame:RegisterEvent("PLAYER_LOGIN")

local function Send(payload)
    SendAddonMessage(ADDON_PREFIX, payload, "WHISPER", UnitName("player"))
end

local function OnAddonMessage(prefix, message, channel, sender)
    if prefix ~= ADDON_PREFIX then
        return
    end

    local cmd, rest = strsplit("\t", message, 2)
    if cmd == "PONG" then
        DEFAULT_CHAT_FRAME:AddMessage("|cffc41e3a[Mak'Gora]|r addon link OK")
    elseif cmd == "STATE" then
        -- enabled, eligible, bind, permadeath, isLeader, leaderName, ritualState
        local enabled, eligible, bind, permadeath, isLeader, leaderName, ritualState = strsplit("\t", rest)
        DEFAULT_CHAT_FRAME:AddMessage(string.format(
            "|cffc41e3a[Mak'Gora]|r state enabled=%s eligible=%s bind=%s dead=%s leader=%s (%s) ritual=%s",
            enabled, eligible, bind, permadeath, isLeader, leaderName or "?", ritualState or "?"
        ))
    end
end

frame:SetScript("OnEvent", function(_, event, ...)
    if event == "PLAYER_LOGIN" then
        if RegisterAddonMessagePrefix then
            RegisterAddonMessagePrefix(ADDON_PREFIX)
        end
        Send("SYNC")
    elseif event == "CHAT_MSG_ADDON" then
        OnAddonMessage(...)
    end
end)

SLASH_MAKGORA1 = "/makgora"
SlashCmdList["MAKGORA"] = function(msg)
    msg = strtrim(msg or ""):lower()
    if msg == "sync" or msg == "" then
        Send("SYNC")
    elseif msg == "ping" then
        Send("PING")
    elseif msg == "bind" then
        Send("BIND\t1")
    elseif msg == "unbind" then
        Send("BIND\t0")
    else
        DEFAULT_CHAT_FRAME:AddMessage("|cffc41e3a[Mak'Gora]|r /makgora sync|ping|bind|unbind")
    end
end
