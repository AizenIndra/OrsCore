----------------------------------------------------------------------
-- Ors Premium v1.1.0 -- VIP panel for mod-premium (WotLK 3.3.5a)
-- /premium  /vip
----------------------------------------------------------------------
local PREFIX = "ORSPREMIUM"

local _tm = {}
local _tf = CreateFrame("Frame")
_tf:SetScript("OnUpdate", function(_, dt)
    local i = 1
    while i <= #_tm do
        _tm[i].t = _tm[i].t - dt
        if _tm[i].t <= 0 then
            _tm[i].fn()
            table.remove(_tm, i)
        else
            i = i + 1
        end
    end
end)
local function After(s, fn) _tm[#_tm + 1] = { t = s, fn = fn } end

----------------------------------------------------------------------
-- STATE
----------------------------------------------------------------------
local P = {
    frame = nil,
    checking = false,
    vipActive = false,
    vipTime = "",
    balance = 0,
    freeDay = "claimed",
    prices = { [1] = 10, [7] = 350, [31] = 600 },
    _appName = "",
    pendingOut = {},
    cooldowns = {},
    cdButtons = {},
    ui = {},
}

local CD_TIMES = {
    [".vip app"] = 900,
    [".vip summon"] = 900,
    [".vip home"] = 900,
}

local CMDS = {
    { cmd = ".vip bank", icon = "Interface\\Icons\\INV_Misc_Coin_02",
      name = "Банк", tip = "Открыть банк без NPC" },
    { cmd = ".vip mail", icon = "Interface\\Icons\\INV_Letter_15",
      name = "Почта", tip = "Открыть почту без NPC" },
    { cmd = ".vip repair", icon = "Interface\\Icons\\Trade_BlackSmithing",
      name = "Ремонт", tip = "Бесплатный ремонт всей экипировки" },
    { cmd = ".vip resettalents", icon = "Interface\\Icons\\Ability_Paladin_JudgementRed",
      name = "Сброс талантов", tip = "Мгновенный бесплатный сброс талантов" },
    { cmd = ".vip taxi", icon = "Interface\\Icons\\Ability_Mount_Gryphon_01",
      name = "Все полёты", tip = "Открыть все точки полётов на карте" },
    { cmd = ".vip home", icon = "Interface\\Icons\\INV_Misc_Rune_01",
      name = "Хартстоун", tip = "Телепорт к точке привязки" },
    { cmd = ".vip capital", icon = "Interface\\Icons\\Spell_Arcane_TeleportStormwind",
      name = "В столицу", tip = "Орда: Оргриммар\nАльянс: Штормград" },
    { cmd = ".vip buff", icon = "Interface\\Icons\\Spell_Holy_WordFortitude",
      name = "Баффы", tip = "Наложить стандартный набор VIP-баффов" },
    { cmd = ".vip debuff", icon = "Interface\\Icons\\Spell_Holy_SenseUndead",
      name = "Снять дебафф", tip = "Снять слабость воскрешения / дезертирство" },
    { cmd = ".vip changerace", icon = "Interface\\Icons\\Achievement_Character_Human_Male",
      name = "Смена расы", tip = "Флаг смены расы (нужен релог)" },
    { cmd = ".vip customize", icon = "Interface\\Icons\\INV_Misc_Comb_02",
      name = "Внешность", tip = "Флаг кастомизации (нужен релог)" },
}

local COLOR_LIST = {
    { id = 0, name = "Выкл", hex = "888888" },
    { id = 1, name = "Золотой", hex = "FFD700" },
    { id = 2, name = "Красный", hex = "FF4444" },
    { id = 3, name = "Зелёный", hex = "00FF00" },
    { id = 4, name = "Голубой", hex = "00BFFF" },
    { id = 5, name = "Фиолет", hex = "B048F8" },
    { id = 6, name = "Оранж", hex = "FF8000" },
    { id = 7, name = "Розовый", hex = "FF69B4" },
    { id = 8, name = "Бирюза", hex = "00FFFF" },
    { id = 9, name = "Белый", hex = "FFFFFF" },
    { id = 10, name = "Алый", hex = "E94560" },
}

----------------------------------------------------------------------
-- HELPERS
----------------------------------------------------------------------
local function BD(f, r, g, b, a, er, eg, eb, ea)
    f:SetBackdrop({
        bgFile = "Interface\\Tooltips\\UI-Tooltip-Background",
        edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
        tile = true, tileSize = 16, edgeSize = 16,
        insets = { left = 4, right = 4, top = 4, bottom = 4 },
    })
    f:SetBackdropColor(r or .08, g or .08, b or .16, a or .95)
    f:SetBackdropBorderColor(er or .5, eg or .4, eb or .1, ea or .7)
end

local function Exec(cmd)
    SendChatMessage(cmd, "SAY")
end

local function Msg(txt)
    DEFAULT_CHAT_FRAME:AddMessage("|cFFFFD700[Premium]|r " .. txt)
end

local function RequireVip(action)
    if not P.vipActive then
        Msg("|cFFFF4444Нужен активный VIP.|r |cFF888888Купите в магазине выше.|r")
        return false
    end
    action()
    return true
end

local function StartCooldown(cmd)
    if CD_TIMES[cmd] then
        P.cooldowns[cmd] = GetTime() + CD_TIMES[cmd]
    end
end

local function GetCooldownRemaining(cmd)
    if P.cooldowns[cmd] then
        local rem = P.cooldowns[cmd] - GetTime()
        if rem > 0 then return rem end
        P.cooldowns[cmd] = nil
    end
    return 0
end

local function FormatCD(secs)
    local m = math.floor(secs / 60)
    local s = math.floor(secs % 60)
    if m > 0 then
        return string.format("%dм %dс", m, s)
    end
    return string.format("%dс", s)
end

local function IsFriend(name)
    if not name or name == "" then return false end
    local lower = name:lower()
    for i = 1, GetNumFriends() do
        local n = GetFriendInfo(i)
        if n and n:lower() == lower then return true end
    end
    return false
end

local function IsFriendOnline(name)
    local lower = name:lower()
    for i = 1, GetNumFriends() do
        local n, _, _, _, connected = GetFriendInfo(i)
        if n and n:lower() == lower and connected then
            return true
        end
    end
    return false
end

local function SendVVMsg(target, msgType)
    SendAddonMessage(PREFIX, msgType, "WHISPER", target)
end

local function IsProtocolMsg(msg)
    if not msg then return false end
    if msg:find("^Premium%.", 1) then return true end
    if msg:find("VIP panel ready", 1, true) then return true end
    if msg:find("VIP commands are available", 1, true) then return true end
    if msg:find("Premium time remaining:", 1, true) then return true end
    if msg:find("Осталось премиум%-времени:", 1) then return true end
    return false
end

----------------------------------------------------------------------
-- SYSTEM MESSAGE PARSING
----------------------------------------------------------------------
local function ParseSystemMessage(msg)
    if not msg or msg == "" then return end

    local status = msg:match("^Premium%.Status:%s*(%S+)")
    if status then
        P.vipActive = (status == "active")
        return
    end

    local bal = msg:match("^Premium%.Balance:%s*(%d+)")
    if bal then
        P.balance = tonumber(bal) or 0
        return
    end

    local free = msg:match("^Premium%.FreeDay:%s*(%S+)")
    if free then
        P.freeDay = free
        return
    end

    local p1 = msg:match("^Premium%.Price%.1:%s*(%d+)")
    if p1 then P.prices[1] = tonumber(p1) or P.prices[1]; return end

    local p7 = msg:match("^Premium%.Price%.7:%s*(%d+)")
    if p7 then P.prices[7] = tonumber(p7) or P.prices[7]; return end

    local p31 = msg:match("^Premium%.Price%.31:%s*(%d+)")
    if p31 then P.prices[31] = tonumber(p31) or P.prices[31]; return end

    local left = msg:match("^Premium%.TimeLeft:%s*(.+)")
        or msg:match("Premium time remaining:%s*(.+)")
        or msg:match("Осталось премиум%-времени:%s*(.+)")
    if left then
        P.vipTime = left
        return
    end
end

local function IsPanelReady(msg)
    return msg:find("VIP panel ready", 1, true)
        or msg:find("VIP commands are available", 1, true)
end

function P:UpdateUI()
    if not self.frame then return end

    if self.ui.timeTx then
        self.ui.timeTx:SetText(self.vipTime ~= "" and ("|cFF80C0FF" .. self.vipTime .. "|r") or "")
    end

    if self.ui.statusTx then
        if self.vipActive then
            self.ui.statusTx:SetText("|TInterface\\RaidFrame\\ReadyCheck-Ready:14|t  VIP активен")
            self.ui.statusTx:SetTextColor(.3, 1, .3)
        else
            self.ui.statusTx:SetText("|TInterface\\RaidFrame\\ReadyCheck-NotReady:14|t  Нет VIP")
            self.ui.statusTx:SetTextColor(1, .3, .3)
        end
    end

    if self.ui.balanceTx then
        self.ui.balanceTx:SetText("|cFFFFD700Баланс:|r |cFFFFFFFF" .. self.balance .. "|r бонусов")
    end

    if self.ui.freeBtn then
        local avail = (self.freeDay == "available")
        self.ui.freeBtn:SetEnabled(avail)
        if avail then
            self.ui.freeBtn._tx:SetText("|cFF00FF00Бесплатно 7 дней|r")
            BD(self.ui.freeBtn, .08, .16, .08, 1, .20, .60, .20, .6)
        else
            self.ui.freeBtn._tx:SetText("|cFF666666Уже получено|r")
            BD(self.ui.freeBtn, .10, .10, .10, 1, .30, .30, .30, .4)
        end
    end

    if self.ui.buyBtns then
        for days, btn in pairs(self.ui.buyBtns) do
            local price = self.prices[days] or 0
            btn._tx:SetText("|cFFE0E0E0" .. days .. "д|r |cFFFFD700" .. price .. "|r")
        end
    end

    -- Shop only without VIP; VIP panel only with VIP.
    if self.vipActive then
        if self.ui.shopPanel then self.ui.shopPanel:Hide() end
        if self.ui.vipPanel then self.ui.vipPanel:Show() end
        self.frame:SetHeight(620)
    else
        if self.ui.shopPanel then self.ui.shopPanel:Show() end
        if self.ui.vipPanel then self.ui.vipPanel:Hide() end
        self.frame:SetHeight(250)
    end
end

----------------------------------------------------------------------
-- CONFIRMATION POPUPS
----------------------------------------------------------------------
StaticPopupDialogs["ORSPREMIUM_TP_CONFIRM"] = {
    text = "|cFFFFD700[Premium]|r\n\n|cFF00FF00%s|r хочет телепортироваться к вам.\n\nРазрешить?",
    button1 = "Разрешить",
    button2 = "Отклонить",
    timeout = 30, whileDead = true, hideOnEscape = true,
    OnAccept = function(self)
        local name = self.data
        if name then SendVVMsg(name, "TPACK") end
        Msg("|cFF00FF00Телепорт разрешён для|r |cFFFFFFFF" .. name .. "|r")
    end,
    OnCancel = function(self)
        local name = self.data
        if name then SendVVMsg(name, "TPDNY") end
        Msg("|cFFFF4444Телепорт отклонён для|r |cFFFFFFFF" .. name .. "|r")
    end,
}

StaticPopupDialogs["ORSPREMIUM_SUM_CONFIRM"] = {
    text = "|cFFFFD700[Premium]|r\n\n|cFF00FF00%s|r хочет призвать вас к себе.\n\nСогласиться?",
    button1 = "Принять",
    button2 = "Отклонить",
    timeout = 30, whileDead = true, hideOnEscape = true,
    OnAccept = function(self)
        local name = self.data
        if name then SendVVMsg(name, "SUMACK") end
        Msg("|cFF00FF00Вы приняли призыв от|r |cFFFFFFFF" .. name .. "|r")
    end,
    OnCancel = function(self)
        local name = self.data
        if name then SendVVMsg(name, "SUMDNY") end
        Msg("|cFFFF4444Призыв от|r |cFFFFFFFF" .. name .. "|r |cFFFF4444отклонён.|r")
    end,
}

StaticPopupDialogs["ORSPREMIUM_WAITING"] = {
    text = "|cFFFFD700[Premium]|r\n\nОжидание ответа от |cFF00FF00%s|r...\n|cFF888888(30 сек)|r",
    button1 = "Отмена",
    timeout = 30, whileDead = true, hideOnEscape = true,
    OnAccept = function(self)
        local name = self.data
        if name then P.pendingOut[name] = nil end
    end,
}

----------------------------------------------------------------------
-- TELEPORT / SUMMON
----------------------------------------------------------------------
local function RequestTeleport(name)
    if not RequireVip(function() end) then return end
    if not name or name == "" then Msg("|cFFFF4444Введите имя игрока!|r"); return end
    if name:lower() == UnitName("player"):lower() then
        Msg("|cFFFF4444Нельзя телепортироваться к себе!|r"); return
    end
    if not IsFriend(name) then
        Msg("|cFFFF4444" .. name .. " не в вашем списке друзей!|r"); return
    end
    if not IsFriendOnline(name) then
        Msg("|cFFFF4444" .. name .. " сейчас не в сети!|r"); return
    end

    local rem = GetCooldownRemaining(".vip app")
    if rem > 0 then
        Msg("|cFFFF4444Телепорт на кулдауне:|r " .. FormatCD(rem)); return
    end

    P.pendingOut[name] = { type = "TP", time = GetTime() }
    SendVVMsg(name, "TPREQ")
    local popup = StaticPopup_Show("ORSPREMIUM_WAITING", name)
    if popup then popup.data = name end
    Msg("|cFF80C0FFЗапрос телепорта отправлен игроку|r |cFFFFFFFF" .. name .. "|r")
end

local function RequestSummon(name)
    if not RequireVip(function() end) then return end
    if not name or name == "" then Msg("|cFFFF4444Введите имя игрока!|r"); return end
    if name:lower() == UnitName("player"):lower() then
        Msg("|cFFFF4444Нельзя призвать себя!|r"); return
    end
    if not IsFriend(name) then
        Msg("|cFFFF4444" .. name .. " не в вашем списке друзей!|r"); return
    end
    if not IsFriendOnline(name) then
        Msg("|cFFFF4444" .. name .. " сейчас не в сети!|r"); return
    end

    local rem = GetCooldownRemaining(".vip summon")
    if rem > 0 then
        Msg("|cFFFF4444Призыв на кулдауне:|r " .. FormatCD(rem)); return
    end

    P.pendingOut[name] = { type = "SUM", time = GetTime() }
    SendVVMsg(name, "SUMREQ")
    local popup = StaticPopup_Show("ORSPREMIUM_WAITING", name)
    if popup then popup.data = name end
    Msg("|cFFD0A0FFЗапрос призыва отправлен игроку|r |cFFFFFFFF" .. name .. "|r")
end

local function OnAddonMsg(prefix, msg, channel, sender)
    if prefix ~= PREFIX then return end
    if sender == UnitName("player") then return end

    if msg == "TPREQ" then
        local popup = StaticPopup_Show("ORSPREMIUM_TP_CONFIRM", sender)
        if popup then popup.data = sender end
    elseif msg == "TPACK" then
        StaticPopup_Hide("ORSPREMIUM_WAITING")
        if P.pendingOut[sender] and P.pendingOut[sender].type == "TP" then
            Msg("|cFF00FF00" .. sender .. " разрешил телепорт!|r Перемещение...")
            Exec(".vip app " .. sender)
            StartCooldown(".vip app")
            P.pendingOut[sender] = nil
        end
    elseif msg == "TPDNY" then
        StaticPopup_Hide("ORSPREMIUM_WAITING")
        P.pendingOut[sender] = nil
        Msg("|cFFFF4444" .. sender .. " отклонил ваш запрос телепорта.|r")
    elseif msg == "SUMREQ" then
        local popup = StaticPopup_Show("ORSPREMIUM_SUM_CONFIRM", sender)
        if popup then popup.data = sender end
    elseif msg == "SUMACK" then
        StaticPopup_Hide("ORSPREMIUM_WAITING")
        if P.pendingOut[sender] and P.pendingOut[sender].type == "SUM" then
            Msg("|cFF00FF00" .. sender .. " принял призыв!|r Призыв...")
            Exec(".vip summon " .. sender)
            StartCooldown(".vip summon")
            P.pendingOut[sender] = nil
        end
    elseif msg == "SUMDNY" then
        StaticPopup_Hide("ORSPREMIUM_WAITING")
        P.pendingOut[sender] = nil
        Msg("|cFFFF4444" .. sender .. " отклонил ваш призыв.|r")
    end
end

----------------------------------------------------------------------
-- BUTTON BUILDERS
----------------------------------------------------------------------
local function CmdButton(parent, w, h, data)
    local b = CreateFrame("Button", nil, parent)
    b:SetSize(w, h)
    BD(b, .10, .10, .20, 1, .40, .32, .06, .55)

    local ic = b:CreateTexture(nil, "ARTWORK")
    ic:SetSize(26, 26)
    ic:SetPoint("LEFT", 6, 0)
    ic:SetTexture(data.icon)
    ic:SetTexCoord(.08, .92, .08, .92)

    local tx = b:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    tx:SetPoint("LEFT", ic, "RIGHT", 6, 0)
    tx:SetPoint("RIGHT", -4, 0)
    tx:SetJustifyH("LEFT")
    tx:SetText("|cFFE0E0E0" .. data.name .. "|r")
    b._tx = tx
    b._name = data.name
    b._cmd = data.cmd

    local cdTx = b:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    cdTx:SetPoint("RIGHT", -6, 0)
    cdTx:SetTextColor(1, .3, .3)
    cdTx:SetText("")
    b._cdTx = cdTx

    b:SetScript("OnEnter", function(s)
        local rem = GetCooldownRemaining(s._cmd)
        if rem > 0 then
            BD(s, .20, .08, .08, 1, .60, .20, .20, .6)
        else
            BD(s, .30, .24, .04, 1, 1, .84, 0, 1)
        end
        s._tx:SetText("|cFFFFFFFF" .. data.name .. "|r")
        GameTooltip:SetOwner(s, "ANCHOR_RIGHT")
        GameTooltip:ClearLines()
        GameTooltip:AddLine("|cFFFFD700" .. data.name .. "|r")
        GameTooltip:AddLine(data.tip, .8, .8, .8, true)
        if rem > 0 then
            GameTooltip:AddLine(" ")
            GameTooltip:AddLine("|cFFFF4444КД: " .. FormatCD(rem) .. "|r")
        end
        GameTooltip:AddLine(" ")
        GameTooltip:AddLine("|cFF666666" .. data.cmd .. "|r")
        GameTooltip:Show()
    end)
    b:SetScript("OnLeave", function(s)
        BD(s, .10, .10, .20, 1, .40, .32, .06, .55)
        s._tx:SetText("|cFFE0E0E0" .. data.name .. "|r")
        GameTooltip:Hide()
    end)
    b:SetScript("OnClick", function(s)
        if not RequireVip(function() end) then return end
        local rem = GetCooldownRemaining(s._cmd)
        if rem > 0 then
            Msg("|cFFFF4444" .. s._name .. " на кулдауне:|r " .. FormatCD(rem))
            return
        end
        Exec(data.cmd)
        StartCooldown(data.cmd)
        BD(s, 0, .35, 0, 1, .2, .8, .2, 1)
        After(.4, function()
            if s and s.SetBackdropColor then
                BD(s, .10, .10, .20, 1, .40, .32, .06, .55)
            end
        end)
    end)

    if CD_TIMES[data.cmd] then
        P.cdButtons[data.cmd] = b
    end
    return b
end

local function ShopButton(parent, w, h, label)
    local b = CreateFrame("Button", nil, parent)
    b:SetSize(w, h)
    BD(b, .10, .10, .18, 1, .45, .35, .08, .55)

    local tx = b:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    tx:SetPoint("CENTER")
    tx:SetText(label)
    b._tx = tx

    b:SetScript("OnEnter", function(s)
        BD(s, .22, .18, .04, 1, 1, .84, 0, 1)
    end)
    b:SetScript("OnLeave", function(s)
        BD(s, .10, .10, .18, 1, .45, .35, .08, .55)
    end)
    return b
end

local cdUpdater = CreateFrame("Frame")
cdUpdater:SetScript("OnUpdate", function()
    for cmd, btn in pairs(P.cdButtons) do
        local rem = GetCooldownRemaining(cmd)
        if rem > 0 then
            btn._cdTx:SetText("|cFFFF6666" .. FormatCD(rem) .. "|r")
            btn._tx:SetText("|cFF666666" .. btn._name .. "|r")
        else
            btn._cdTx:SetText("")
            btn._tx:SetText("|cFFE0E0E0" .. btn._name .. "|r")
        end
    end
end)

----------------------------------------------------------------------
-- BUILD PANEL
----------------------------------------------------------------------
function P:Build()
    if self.frame then return end

    local W = 320
    local f = CreateFrame("Frame", "OrsPremiumFrame", UIParent)
    f:SetSize(W, 680)
    f:SetPoint("CENTER", 0, 40)
    f:SetMovable(true)
    f:EnableMouse(true)
    f:RegisterForDrag("LeftButton")
    f:SetScript("OnDragStart", f.StartMoving)
    f:SetScript("OnDragStop", f.StopMovingOrSizing)
    f:SetFrameStrata("HIGH")
    f:SetClampedToScreen(true)
    BD(f, .06, .06, .12, .97, .80, .65, .12, .85)
    tinsert(UISpecialFrames, "OrsPremiumFrame")

    local tb = CreateFrame("Frame", nil, f)
    tb:SetSize(W - 8, 34)
    tb:SetPoint("TOP", 0, -4)
    BD(tb, .04, .03, .09, 1, .60, .48, .06, .45)

    local tt = tb:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
    tt:SetPoint("LEFT", 12, 0)
    tt:SetText("|cFFFFD700Ors |cFFE94560Premium|r")

    self.ui.timeTx = tb:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    self.ui.timeTx:SetPoint("RIGHT", -30, 0)
    self.ui.timeTx:SetTextColor(.5, .75, 1)

    local cl = CreateFrame("Button", nil, tb, "UIPanelCloseButton")
    cl:SetSize(24, 24)
    cl:SetPoint("RIGHT", -1, 0)
    cl:SetScript("OnClick", function() f:Hide() end)

    local y = -42

    local sb = CreateFrame("Frame", nil, f)
    sb:SetSize(W - 20, 22)
    sb:SetPoint("TOP", 0, y)
    BD(sb, 0, .12, 0, .85, .15, .55, .15, .5)

    self.ui.statusTx = sb:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    self.ui.statusTx:SetPoint("CENTER")
    self.ui.statusTx:SetText("|cFF888888Загрузка...|r")
    y = y - 28

    local contentY = y

    local shopPanel = CreateFrame("Frame", nil, f)
    shopPanel:SetPoint("TOPLEFT", 0, contentY)
    shopPanel:SetPoint("BOTTOMRIGHT", 0, 18)
    self.ui.shopPanel = shopPanel

    local shopTitle = shopPanel:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    shopTitle:SetPoint("TOPLEFT", 14, 0)
    shopTitle:SetText("|cFFFFD700Магазин VIP|r")

    self.ui.balanceTx = shopPanel:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    self.ui.balanceTx:SetPoint("TOPLEFT", 14, -18)
    self.ui.balanceTx:SetText("|cFFFFD700Баланс:|r |cFFFFFFFF0|r бонусов")

    self.ui.freeBtn = ShopButton(shopPanel, W - 20, 26, "|cFF00FF00Бесплатно 7 дней|r")
    self.ui.freeBtn:SetPoint("TOP", 0, -40)
    self.ui.freeBtn:SetScript("OnClick", function()
        Exec(".vip free1day")
    end)

    self.ui.buyBtns = {}
    local buyW = 93
    local buyGap = 4
    local buyDays = { 1, 7, 31 }
    for i, days in ipairs(buyDays) do
        local btn = ShopButton(shopPanel, buyW, 26, "|cFFE0E0E0" .. days .. "д|r |cFFFFD700" .. self.prices[days] .. "|r")
        btn:SetPoint("TOPLEFT", 10 + (i - 1) * (buyW + buyGap), -74)
        btn:SetScript("OnClick", function()
            Exec(".vip buy " .. days)
        end)
        self.ui.buyBtns[days] = btn
    end

    -- VIP features: only when premium is active (same anchor as shop)
    local vipPanel = CreateFrame("Frame", nil, f)
    vipPanel:SetPoint("TOPLEFT", 0, contentY)
    vipPanel:SetPoint("BOTTOMRIGHT", 0, 18)
    self.ui.vipPanel = vipPanel

    local sShop = vipPanel:CreateTexture(nil, "ARTWORK")
    sShop:SetHeight(1)
    sShop:SetPoint("TOPLEFT", 8, 0)
    sShop:SetPoint("TOPRIGHT", -8, 0)
    sShop:SetTexture("Interface\\Buttons\\WHITE8x8")
    sShop:SetVertexColor(.80, .65, .12, .5)

    local vy = -8
    local cmdTitle = vipPanel:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    cmdTitle:SetPoint("TOPLEFT", 14, vy)
    cmdTitle:SetText("|cFFE0E0E0VIP-команды|r")
    vy = vy - 6

    local bW, bH, gap = 143, 36, 5
    for i, d in ipairs(CMDS) do
        local row = math.floor((i - 1) / 2)
        local col = (i - 1) % 2
        local btn = CmdButton(vipPanel, bW, bH, d)
        btn:SetPoint("TOPLEFT", 10 + col * (bW + gap), vy - row * (bH + gap))
    end
    local totalRows = math.ceil(#CMDS / 2)
    vy = vy - totalRows * (bH + gap) - 8

    local s2 = vipPanel:CreateTexture(nil, "ARTWORK")
    s2:SetHeight(1)
    s2:SetPoint("TOPLEFT", 8, vy)
    s2:SetPoint("TOPRIGHT", -8, vy)
    s2:SetTexture("Interface\\Buttons\\WHITE8x8")
    s2:SetVertexColor(.6, .4, .8, .4)

    local secTitle = vipPanel:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    secTitle:SetPoint("TOPLEFT", 14, vy - 4)
    secTitle:SetText("|cFFD0A0FFТелепорт / Призыв|r  |cFF666666(друзья + подтверждение)|r")

    local nameRow = CreateFrame("Frame", nil, vipPanel)
    nameRow:SetSize(W - 20, 30)
    nameRow:SetPoint("TOPLEFT", 10, vy - 22)
    BD(nameRow, .08, .05, .14, .9, .35, .25, .45, .4)

    local nIco = nameRow:CreateTexture(nil, "ARTWORK")
    nIco:SetSize(20, 20)
    nIco:SetPoint("LEFT", 6, 0)
    nIco:SetTexture("Interface\\FriendsFrame\\UI-Toast-FriendOnlineIcon")

    local nLbl = nameRow:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    nLbl:SetPoint("LEFT", nIco, "RIGHT", 4, 0)
    nLbl:SetText("|cFFD0A0FFИмя:|r")

    local inp = CreateFrame("EditBox", "OrsPremium_Input", nameRow, "InputBoxTemplate")
    inp:SetSize(120, 20)
    inp:SetPoint("LEFT", nLbl, "RIGHT", 4, -1)
    inp:SetAutoFocus(false)
    inp:SetMaxLetters(24)
    inp:SetFontObject("GameFontHighlightSmall")
    inp:SetScript("OnEscapePressed", function(s) s:ClearFocus() end)
    inp:SetScript("OnEditFocusLost", function(s) s:HighlightText(0, 0) end)
    inp:SetScript("OnTextChanged", function(s) P._appName = s:GetText() or "" end)
    inp:SetScript("OnEnterPressed", function(s) s:ClearFocus() end)

    local tgtBtn = CreateFrame("Button", nil, nameRow)
    tgtBtn:SetSize(22, 22)
    tgtBtn:SetPoint("RIGHT", -6, 0)
    tgtBtn:SetNormalTexture("Interface\\CURSOR\\Crosshairs")
    tgtBtn:SetScript("OnClick", function()
        if UnitIsPlayer("target") then
            local n = UnitName("target")
            if n then
                inp:SetText(n)
                P._appName = n
            end
        else
            Msg("|cFF888888Выделите игрока как цель.|r")
        end
    end)
    tgtBtn:SetScript("OnEnter", function(s)
        GameTooltip:SetOwner(s, "ANCHOR_RIGHT")
        GameTooltip:AddLine("|cFFFFFFFFЗаполнить из цели|r")
        GameTooltip:Show()
    end)
    tgtBtn:SetScript("OnLeave", function() GameTooltip:Hide() end)

    local btnRow = CreateFrame("Frame", nil, vipPanel)
    btnRow:SetSize(W - 20, 36)
    btnRow:SetPoint("TOPLEFT", 10, vy - 56)

    local tpBtn = CreateFrame("Button", nil, btnRow)
    tpBtn:SetSize(143, 34)
    tpBtn:SetPoint("LEFT", 0, 0)
    BD(tpBtn, .06, .10, .22, 1, .20, .40, .80, .6)
    local tpIco = tpBtn:CreateTexture(nil, "ARTWORK")
    tpIco:SetSize(22, 22)
    tpIco:SetPoint("LEFT", 8, 0)
    tpIco:SetTexture("Interface\\Icons\\Ability_Hunter_Pathfinding")
    tpIco:SetTexCoord(.08, .92, .08, .92)
    local tpTx = tpBtn:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    tpTx:SetPoint("LEFT", tpIco, "RIGHT", 6, 0)
    tpTx:SetText("|cFF80C0FFТелепорт к игроку|r")
    tpBtn._tx = tpTx
    tpBtn:SetScript("OnEnter", function(s)
        BD(s, .15, .20, .40, 1, .40, .60, 1, 1)
        s._tx:SetText("|cFFFFFFFFТелепорт к игроку|r")
        GameTooltip:SetOwner(s, "ANCHOR_BOTTOM")
        GameTooltip:AddLine("|cFF80C0FFТелепорт к другу|r")
        GameTooltip:AddLine("Нужны друзья и подтверждение.\nСервер: .vip app", .8, .8, .8, true)
        local rem = GetCooldownRemaining(".vip app")
        if rem > 0 then GameTooltip:AddLine("|cFFFF4444Осталось: " .. FormatCD(rem) .. "|r") end
        GameTooltip:Show()
    end)
    tpBtn:SetScript("OnLeave", function(s)
        BD(s, .06, .10, .22, 1, .20, .40, .80, .6)
        s._tx:SetText("|cFF80C0FFТелепорт к игроку|r")
        GameTooltip:Hide()
    end)
    tpBtn:SetScript("OnClick", function()
        local n = strtrim(P._appName or "")
        if n == "" and UnitIsPlayer("target") then
            n = UnitName("target") or ""
            P._appName = n
            OrsPremium_Input:SetText(n)
        end
        RequestTeleport(n)
    end)

    local smBtn = CreateFrame("Button", nil, btnRow)
    smBtn:SetSize(143, 34)
    smBtn:SetPoint("LEFT", tpBtn, "RIGHT", 5, 0)
    BD(smBtn, .14, .06, .18, 1, .55, .25, .65, .6)
    local smIco = smBtn:CreateTexture(nil, "ARTWORK")
    smIco:SetSize(22, 22)
    smIco:SetPoint("LEFT", 8, 0)
    smIco:SetTexture("Interface\\Icons\\Spell_Shadow_Twilight")
    smIco:SetTexCoord(.08, .92, .08, .92)
    local smTx = smBtn:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    smTx:SetPoint("LEFT", smIco, "RIGHT", 6, 0)
    smTx:SetText("|cFFD0A0FFПризвать к себе|r")
    smBtn._tx = smTx
    smBtn:SetScript("OnEnter", function(s)
        BD(s, .28, .12, .35, 1, .80, .45, 1, 1)
        s._tx:SetText("|cFFFFFFFFПризвать к себе|r")
        GameTooltip:SetOwner(s, "ANCHOR_BOTTOM")
        GameTooltip:AddLine("|cFFD0A0FFПризвать друга|r")
        GameTooltip:AddLine("Нужны друзья и подтверждение.\nСервер: .vip summon", .8, .8, .8, true)
        local rem = GetCooldownRemaining(".vip summon")
        if rem > 0 then GameTooltip:AddLine("|cFFFF4444Осталось: " .. FormatCD(rem) .. "|r") end
        GameTooltip:Show()
    end)
    smBtn:SetScript("OnLeave", function(s)
        BD(s, .14, .06, .18, 1, .55, .25, .65, .6)
        s._tx:SetText("|cFFD0A0FFПризвать к себе|r")
        GameTooltip:Hide()
    end)
    smBtn:SetScript("OnClick", function()
        local n = strtrim(P._appName or "")
        if n == "" and UnitIsPlayer("target") then
            n = UnitName("target") or ""
            P._appName = n
            OrsPremium_Input:SetText(n)
        end
        RequestSummon(n)
    end)

    local colorY = vy - 100
    local s3 = vipPanel:CreateTexture(nil, "ARTWORK")
    s3:SetHeight(1)
    s3:SetPoint("TOPLEFT", 8, colorY)
    s3:SetPoint("TOPRIGHT", -8, colorY)
    s3:SetTexture("Interface\\Buttons\\WHITE8x8")
    s3:SetVertexColor(.8, .7, .2, .4)

    local colorTitle = vipPanel:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    colorTitle:SetPoint("TOPLEFT", 14, colorY - 4)
    colorTitle:SetText("|cFFFFD700Цвет чата VIP|r  |cFF666666(.vip textcolor)|r")

    local colBtnW, colBtnH, colGap = 53, 22, 3
    local colStartY = colorY - 22
    local colsPerRow = 5
    for i, col in ipairs(COLOR_LIST) do
        local row = math.floor((i - 1) / colsPerRow)
        local colIdx = (i - 1) % colsPerRow
        local cb = CreateFrame("Button", nil, vipPanel)
        cb:SetSize(colBtnW, colBtnH)
        cb:SetPoint("TOPLEFT", 10 + colIdx * (colBtnW + colGap), colStartY - row * (colBtnH + colGap))
        BD(cb, .08, .08, .16, 1, .40, .32, .06, .55)

        local ct = cb:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
        ct:SetPoint("CENTER")
        ct:SetText("|cFF" .. col.hex .. col.name .. "|r")

        cb:SetScript("OnClick", function()
            if not RequireVip(function() end) then return end
            Exec(".vip textcolor " .. col.id)
            if col.id == 0 then
                Msg("|cFFFFD700Цвет чата отключён.|r")
            else
                Msg("|cFFFFD700Цвет чата: |cFF" .. col.hex .. col.name .. "|r")
            end
        end)
        cb:SetScript("OnEnter", function(s)
            BD(s, .30, .24, .04, 1, 1, .84, 0, 1)
            if col.id > 0 then
                GameTooltip:SetOwner(s, "ANCHOR_RIGHT")
                GameTooltip:AddLine("|cFF" .. col.hex .. "[VIP] Пример сообщения|r")
                GameTooltip:AddLine("|cFF888888Нажмите для выбора|r")
                GameTooltip:Show()
            end
        end)
        cb:SetScript("OnLeave", function(s)
            BD(s, .08, .08, .16, 1, .40, .32, .06, .55)
            GameTooltip:Hide()
        end)
    end

    shopPanel:Show()
    vipPanel:Hide()

    local ft = f:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
    ft:SetPoint("BOTTOM", 0, 6)
    ft:SetText("|cFF444444mod-premium|r")

    f:SetHeight(250)
    f:Hide()
    self.frame = f
    self:UpdateUI()
end

function P:Show()
    if not self.frame then
        local ok, err = pcall(function() self:Build() end)
        if not ok then
            Msg("|cFFFF4444Ошибка UI:|r " .. tostring(err))
            return
        end
    end
    if not self.frame then return end
    self:UpdateUI()
    self.frame:Show()
end

function P:Toggle()
    if self.frame and self.frame:IsShown() then
        self.frame:Hide()
        return
    end
    self.checking = true
    self:Show()
    Exec(".vip")
end

function P:OnSystemMessage(msg)
    ParseSystemMessage(msg)
    self:UpdateUI()

    if self.checking and IsPanelReady(msg) then
        self.checking = false
    end
end

----------------------------------------------------------------------
-- EVENTS
----------------------------------------------------------------------
local ev = CreateFrame("Frame")
ev:RegisterEvent("PLAYER_LOGIN")
ev:RegisterEvent("CHAT_MSG_SYSTEM")
ev:RegisterEvent("CHAT_MSG_ADDON")
ev:SetScript("OnEvent", function(_, event, a1, a2, a3, a4)
    if event == "PLAYER_LOGIN" then
        ChatFrame_AddMessageEventFilter("CHAT_MSG_SAY", function(_, _, msg)
            if msg and msg:sub(1, 4) == ".vip" then return true end
        end)
        ChatFrame_AddMessageEventFilter("CHAT_MSG_SYSTEM", function(_, _, msg)
            if IsProtocolMsg(msg) then return true end
            if P.checking and msg then
                if msg:find("VIP", 1, true) or msg:find("premium", 1, true)
                    or msg:find("премиум", 1, true) then
                    return true
                end
            end
            return false
        end)
        ShowFriends()
        DEFAULT_CHAT_FRAME:AddMessage("|cFFFFD700[Premium]|r загружен. |cFF00FF00/premium|r или |cFF00FF00/vip|r")
    elseif event == "CHAT_MSG_SYSTEM" then
        P:OnSystemMessage(a1 or "")
    elseif event == "CHAT_MSG_ADDON" then
        OnAddonMsg(a1, a2, a3, a4)
    end
end)

----------------------------------------------------------------------
-- SLASH + MINIMAP
----------------------------------------------------------------------
SLASH_ORSPREMIUM1 = "/premium"
SLASH_ORSPREMIUM2 = "/vip"
SlashCmdList["ORSPREMIUM"] = function() P:Toggle() end

do
    local init = CreateFrame("Frame")
    init:RegisterEvent("PLAYER_ENTERING_WORLD")
    init:SetScript("OnEvent", function(self)
        self:UnregisterAllEvents()
        local mb = CreateFrame("Button", "OrsPremium_Mini", Minimap)
        mb:SetSize(32, 32)
        mb:SetFrameStrata("MEDIUM")
        mb:SetFrameLevel(8)
        mb:SetHighlightTexture("Interface\\Minimap\\UI-Minimap-ZoomButton-Highlight")

        local brd = mb:CreateTexture(nil, "OVERLAY")
        brd:SetSize(52, 52)
        brd:SetPoint("CENTER", mb, "CENTER", 10, -10)
        brd:SetTexture("Interface\\Minimap\\MiniMap-TrackingBorder")

        local ico = mb:CreateTexture(nil, "BACKGROUND")
        ico:SetSize(20, 20)
        ico:SetPoint("CENTER", 0, 0)
        ico:SetTexture("Interface\\Icons\\Spell_Holy_DivineIllumination")
        ico:SetTexCoord(.08, .92, .08, .92)

        local ang = 220
        local function Pos()
            local r = math.rad(ang)
            mb:ClearAllPoints()
            mb:SetPoint("CENTER", Minimap, "CENTER", math.cos(r) * 77, math.sin(r) * 77)
        end
        Pos()

        local drag = false
        mb:RegisterForDrag("LeftButton")
        mb:SetScript("OnDragStart", function() drag = true end)
        mb:SetScript("OnDragStop", function() drag = false end)
        mb:SetScript("OnUpdate", function()
            if not drag then return end
            local cx, cy = GetCursorPosition()
            local s = Minimap:GetEffectiveScale()
            local mx, my = Minimap:GetCenter()
            ang = math.deg(math.atan2(cy / s - my, cx / s - mx))
            Pos()
        end)
        mb:SetScript("OnClick", function() P:Toggle() end)
        mb:SetScript("OnEnter", function(s)
            GameTooltip:SetOwner(s, "ANCHOR_LEFT")
            GameTooltip:AddLine("|cFFFFD700Ors|cFFE94560Premium|r")
            GameTooltip:AddLine("|cFF888888Открыть VIP-панель|r")
            GameTooltip:Show()
        end)
        mb:SetScript("OnLeave", function() GameTooltip:Hide() end)
    end)
end
