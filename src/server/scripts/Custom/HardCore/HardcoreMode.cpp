/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * Custom/HardCore — режим хардкор: смерть до макс. уровня необратима, 80+ потеря предметов в подземельях.
 * Конфиг: worldserver.conf, секция CUSTOM, ключи ModHardcore.*
 */

 #include "ScriptMgr.h"
 #include "Player.h"
 #include "Config.h"
 #include "AddonIO.h"
 #include "Chat.h"
 #include "World.h"
 #include "Map.h"
 #include "Item.h"
 #include "ItemTemplate.h"
 #include "SpellAuras.h"
 #include "SpellMgr.h"
 #include "SharedDefines.h"
 #include "ObjectMgr.h"
 #include "WorldSessionMgr.h"
 #include "Bag.h"
 #include "IWorld.h"
 #include "ItemScript.h"
 #include "Spell.h"
 #include "SpellInfo.h"
 #include "DBCStores.h"
 #include "Battleground.h"
 #include "BattlegroundMgr.h"
 #include "Group.h"
 #include "GroupScript.h"
 #include "UnitScript.h"
 #include "AllSpellScript.h"
 #include "ScriptedGossip.h"
 #include "GossipDef.h"
 #include "Common.h"
 #include "Log.h"
 #include "AllItemScript.h"
 #include "CreatureScript.h"
 #include "LFGMgr.h"
 #include <unordered_map>
 #include <unordered_set>
 #include <functional>
 
 namespace Hardcore
 {
     static std::unordered_set<ObjectGuid> playersDiedFromFall;
 
     static uint8 GetMaxLevel()
     {
         int maxLevel = sConfigMgr->GetOption<int>("ModHardcoreMaxLevel.Enable", 80);
         if (maxLevel < 1)
             maxLevel = 1;
         if (maxLevel > MAX_LEVEL)
             maxLevel = MAX_LEVEL;
         return static_cast<uint8>(maxLevel);
     }
 
    static bool IsHardcoreEnabledForPlayer(Player* player)
    {
        if (!player || player->IsGameMaster() || !player->IsHardcore())
            return false;
        uint8 level = player->GetLevel();
        int minLevel = sConfigMgr->GetOption<int>("ModHardcoreMinLevel.Enable", 1);
        uint8 maxLevel = GetMaxLevel();
        return (level >= minLevel && level < maxLevel);
    }
 
     static void BroadcastHardcoreDeath(Player* player, Unit* killerUnit)
     {
         if (!player)
             return;
 
         std::string playerName = player->GetName();
         std::string locationName = "неизвестной локации";
 
         if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(player->GetAreaId()))
         {
             if (areaEntry->area_name[LOCALE_ruRU] && strlen(areaEntry->area_name[LOCALE_ruRU]) > 0)
                 locationName = areaEntry->area_name[LOCALE_ruRU];
             else
                 locationName = areaEntry->area_name[0];
         }
         else if (AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(player->GetZoneId()))
         {
             if (zoneEntry->area_name[LOCALE_ruRU] && strlen(zoneEntry->area_name[LOCALE_ruRU]) > 0)
                 locationName = zoneEntry->area_name[LOCALE_ruRU];
             else
                 locationName = zoneEntry->area_name[0];
         }
 
         std::string killerName = "Неизвестный враг";
         uint8 killerLevel = 0;
 
         if (killerUnit)
         {
             if (Creature* creature = killerUnit->ToCreature())
             {
                 killerName = creature->GetNameForLocaleIdx(LOCALE_ruRU).empty() ? creature->GetName() : creature->GetNameForLocaleIdx(LOCALE_ruRU);
                 killerLevel = creature->GetLevel();
             }
             else if (Player* killerPlayer = killerUnit->ToPlayer())
             {
                 killerName = killerPlayer->GetName();
                 killerLevel = killerPlayer->GetLevel();
             }
         }
 
         // Уведомление только через Custom_hardCore (FrameXML). Разделитель "|" — в зонах/именах может быть ":"
         // Payload: name|race|gender|class|level|zone|reason|npc|npcLevel (race/class — id для RACE_NAMES/CLASS_NAMES на клиенте)
         uint8 race   = player->getRace();   // ChrRaces id
         uint8 gender = player->getGender();
         uint8 clazz  = player->getClass();  // ChrClasses id
         uint8 level  = player->GetLevel();
         uint8 reason = 0;
         std::string addonPayload = playerName + "|" + std::to_string(uint32(race)) + "|" + std::to_string(uint32(gender)) + "|" + std::to_string(uint32(clazz)) + "|" + std::to_string(uint32(level)) + "|" + locationName + "|" + std::to_string(uint32(reason)) + "|" + killerName + "|" + std::to_string(uint32(killerLevel));
         sAddonIO->BroadcastHardcoreDeath(addonPayload);
     }
 }
 
 using Hardcore::GetMaxLevel;
 static bool ApplyInsuranceToItemStatic(Player* player, Item* scrollItem, Item* targetItem)
 {
     if (!player || !scrollItem || !targetItem)
     {
         return false;
     }
 
     if (targetItem->GetOwnerGUID() != player->GetGUID())
     {
         ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Этот предмет вам не принадлежит.");
         return false;
     }
 
     uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
     for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
     {
         uint32 enchantId = targetItem->GetEnchantmentId(EnchantmentSlot(i));
         if (enchantId == insuranceEnchantId)
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Этот предмет уже застрахован.");
             return false;
         }
     }
 
     if (targetItem->IsEquipped())
     {
         uint8 slot = targetItem->GetSlot();
         if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Рубашку и накидку нельзя застраховать.");
             return false;
         }
     }
 
     // 3 часа страховки в миллисекундах
     uint32 insuranceDurationMs = 3 * 60 * 60 * IN_MILLISECONDS;
 
     // выбираем свободный слот под страховой энчант
     EnchantmentSlot freeSlot = PERM_ENCHANTMENT_SLOT;
     bool slotFound = false;
 
     for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
     {
         if (targetItem->GetEnchantmentId(EnchantmentSlot(i)) == 0)
         {
             freeSlot = EnchantmentSlot(i);
             slotFound = true;
             break;
         }
     }
 
     if (!slotFound)
     {
         freeSlot = TEMP_ENCHANTMENT_SLOT;
     }
 
     // применяем только страховой энчант на 3 часа
     // визуальное зачарование удалено т.к. оно может вешать ауру на игрока
     targetItem->SetEnchantment(freeSlot, insuranceEnchantId, insuranceDurationMs, 0);
     
     targetItem->SetState(ITEM_CHANGED, player);
 
     player->DestroyItemCount(scrollItem->GetEntry(), 1, true);
 
     std::string itemName = targetItem->GetTemplate()->Name1;
     if (ItemLocale const* il = sObjectMgr->GetItemLocale(targetItem->GetEntry()))
     {
         ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
     }
     
     ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00[Страхование]|r Предмет |cFFFF0000{}|r успешно застрахован!", itemName);
     ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00[Страхование]|r Этот предмет не будет потерян при смерти на уровне 80+ в течение 3 часов.");
     
     return true;
 }
 
 static void ApplyHardcoreAuraHelper(Player* player)
 {
     if (!player || player->IsGameMaster() || !player->IsInWorld())
         return;
 
     uint32 auraSpell = 0;
     uint32 oldAuraSpell = 0;
     
     uint8 maxLevel = GetMaxLevel();

     if (player->GetLevel() >= maxLevel)
     {
         auraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId80", 0);
         oldAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId", 0);
         if (oldAuraSpell > 0 && player->HasAura(oldAuraSpell))
         {
             player->RemoveAura(oldAuraSpell);
         }
     }
     else
     {
         auraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId", 0);
         oldAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId80", 0);
         if (oldAuraSpell > 0 && player->HasAura(oldAuraSpell))
         {
             player->RemoveAura(oldAuraSpell);
         }
     }
     
     if (auraSpell > 0)
     {
         if (player->HasAura(auraSpell))
         {
             if (Aura* aura = player->GetAura(auraSpell))
             {
                 uint32 remainingDuration = aura->GetDuration();
                 if (remainingDuration > 0 && remainingDuration < 60000)
                 {
                     player->RemoveAura(auraSpell);
                     player->CastSpell(player, auraSpell, true);
                 }
             }
         }
         else
         {
             SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(auraSpell);
             if (!spellInfo)
             {
                 return;
             }
             
             player->CastSpell(player, auraSpell, true);
             
             if (Aura* aura = player->GetAura(auraSpell))
             {
                 aura->SetDuration(-1);
                 aura->SetMaxDuration(-1);
             }
         }
     }
 }
 
 class HardcoreMode : public PlayerScript
 {
 public:
     explicit HardcoreMode() : PlayerScript("mod-hardcore")
     {
     }
 
     void OnPlayerLogin(Player* player) override
     {
         // сброс статуса смерти от падения на всякий случай
         Hardcore::playersDiedFromFall.erase(player->GetGUID());
 
         if (getHardcoreEnabledForPlayer(player))
         {
             if (!player->isDead())
             {
                 // Force-disable PvP for players below configured max level (if enabled)
                 if (sConfigMgr->GetOption<bool>("ModHardcore.ForceDisablePvPUntilMaxLevel", false))
                 {
                     if (player && player->GetLevel() < GetMaxLevel())
                         player->SetPvP(false);
                 }
                 applyHardcoreAura(player);
             }
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            applyHardcoreAura(player);
        }
    }

    void OnPlayerLevelChanged(Player* player, uint8 oldlevel) override
     {
         if (getHardcoreEnabledForPlayer(player))
         {
             applyHardcoreAura(player);
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore() && oldlevel < GetMaxLevel())
        {
            applyHardcoreAura(player);
        }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            applyHardcoreAura(player);
        }
    }

    void OnPlayerPVPFlagChange(Player* player, bool state) override
     {
         // Prevent players from enabling PvP below configured max level
         if (!player)
             return;
 
        if (sConfigMgr->GetOption<bool>("ModHardcore.ForceDisablePvPUntilMaxLevel", false))
        {
            if (state && player->IsHardcore() && player->GetLevel() < GetMaxLevel())
            {
                // revert the change
                player->SetPvP(false);
                 ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[PvP]|r PvP принудительно отключён до уровня {}.", GetMaxLevel());
             }
         }
     }

    bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/,
        BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (!player)
            return true;

        if (player->IsHardcore() && getHardcoreEnabledForPlayer(player))
        {
            err = ERR_BATTLEGROUND_JOIN_FAILED;
            return false;
        }
        return true;
    }

    bool OnPlayerCanJoinLfg(Player* player, uint8 /*roles*/, std::set<uint32>& /*dungeons*/,
        const std::string& /*comment*/) override
    {
        if (!player)
            return true;

        if (player->IsHardcore() && getHardcoreEnabledForPlayer(player))
            return false;
        return true;
    }
 
     void OnPlayerMapChanged(Player* player) override
     {
         if (getHardcoreEnabledForPlayer(player) && !player->isDead())
         {
             applyHardcoreAura(player);
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            applyHardcoreAura(player);
        }
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
     {
         static std::unordered_map<ObjectGuid, uint32> auraCheckTimers;
         
         ObjectGuid guid = player->GetGUID();
         uint32& checkTimer = auraCheckTimers[guid];
         checkTimer += diff;
         
         if (checkTimer >= 2000)
         {
             checkTimer = 0;
             
             bool shouldHaveAura = false;
             uint32 expectedAuraSpell = 0;
             
             if (getHardcoreEnabledForPlayer(player) && !player->isDead())
             {
                 shouldHaveAura = true;
                 expectedAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId", 0);
             }
            else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
            {
                shouldHaveAura = true;
                expectedAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId80", 0);
            }
            
            if (shouldHaveAura && expectedAuraSpell > 0 && !player->HasAura(expectedAuraSpell))
            {
                applyHardcoreAura(player);
            }
            
            if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
            {
                checkInsuranceExpiration(player);
            }
         }
     }
 
     void OnPlayerJustDied(Player* player) override
     {
         bool diedFromFall = Hardcore::playersDiedFromFall.find(player->GetGUID()) != Hardcore::playersDiedFromFall.end();
         
         if (getHardcoreEnabledForPlayer(player))
         {
             // Рассылку о смерти делаем только из OnPlayerKilledByCreature / OnPlayerPVPKill / OnUnitDeath,
             // чтобы всегда передавался правильный убийца (существо/игрок), а не "Неизвестный враг".
 
             Group* group = player->GetGroup();
             if (group)
             {
                 group->RemoveMember(player->GetGUID());
             }
             
             removeHardcoreAura(player);
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            if (!diedFromFall)
            {
                handleItemLossAfter80(player);
            }
            else
            {
                ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00[Падение]|r Вы погибли от падения - предметы не потеряны.");
            }

            applyHardcoreAura(player);
        }
    }

    void OnPlayerReleasedGhost(Player* player) override
     {
         if (getHardcoreEnabledForPlayer(player))
         {
             return;
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            applyHardcoreAura(player);
        }
    }

    void OnPlayerPVPKill(Player* killer, Player* killed) override
     {
         if (killed && getHardcoreEnabledForPlayer(killed))
         {
             Hardcore::BroadcastHardcoreDeath(killed, killer);
         }
     }
 
     void OnPlayerKilledByCreature(Creature* killer, Player* killed) override
     {
         if (killed && getHardcoreEnabledForPlayer(killed))
         {
             Hardcore::BroadcastHardcoreDeath(killed, killer);
         }
     }
 
     void OnPlayerResurrect(Player* player, float /*restore_percent*/, bool& /*applySickness*/) override
     {
         Hardcore::playersDiedFromFall.erase(player->GetGUID());
         
         if (getHardcoreEnabledForPlayer(player))
         {
             player->KillPlayer();
             player->GetSession()->KickPlayer("Игрок погиб в хардкор режиме.");
             return;
         }
        else if (player->GetLevel() >= GetMaxLevel() && player->IsHardcore())
        {
            applyHardcoreAura(player);
        }
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* receiver) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Group* group) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Guild* guild) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Channel* channel) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanGroupInvite(Player* player, std::string& membername) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
 
     bool OnPlayerCanGroupAccept(Player* player, Group* group) override
     {
         if (getHardcoreEnabledForPlayer(player) && player->isDead())
         {
             return false;
         }
         return true;
     }
     
 private:
 
    bool getHardcoreEnabledForPlayer(Player* player)
    {
        if (!player || player->IsGameMaster() || !player->IsHardcore())
            return false;
        uint8 level = player->GetLevel();
        int minLevel = sConfigMgr->GetOption<int>("ModHardcoreMinLevel.Enable", 1);
        uint8 maxLevel = GetMaxLevel();
        return (level >= minLevel && level < maxLevel);
    }
     
     void checkInsuranceExpiration(Player* player)
     {
         if (!player)
             return;
         
         uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
         
         for (uint8 slot = 0; slot < 19; ++slot)
         {
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                 continue;
                 
             if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
             {
                 for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
                 {
                     EnchantmentSlot enchantSlot = EnchantmentSlot(i);
                     if (item->GetEnchantmentId(enchantSlot) == insuranceEnchantId)
                     {
                         player->UpdateEnchantmentDurations();
                         
                         uint32 duration = item->GetEnchantmentDuration(enchantSlot);
                         
                         if (duration == 0 || duration < 1000)
                         {
                             if (item->IsEquipped())
                             {
                                 player->ApplyEnchantment(item, enchantSlot, false);
                             }
                             item->ClearEnchantment(enchantSlot);
                             
                             std::string itemName = item->GetTemplate()->Name1;
                             if (ItemLocale const* il = sObjectMgr->GetItemLocale(item->GetEntry()))
                             {
                                 ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
                             }
                             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF8080[Страхование]|r Страховка на предмете |cFFFF0000{}|r истекла!", itemName);
                             
                             item->SetState(ITEM_CHANGED, player);
                         }
                     }
                 }
             }
         }
     }
 
     void applyHardcoreAura(Player* player)
     {
         ApplyHardcoreAuraHelper(player);
     }
 
     void removeHardcoreAura(Player* player)
     {
         if (!player)
             return;
 
         uint32 hardcoreAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId", 0);
         if (hardcoreAuraSpell > 0 && player->HasAura(hardcoreAuraSpell))
         {
             player->RemoveAura(hardcoreAuraSpell);
         }
         
         uint32 aura80Spell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId80", 0);
         if (aura80Spell > 0 && player->HasAura(aura80Spell))
         {
             player->RemoveAura(aura80Spell);
         }
     }
 
     void broadcastHardcoreDeath(Player* player, Unit* killerUnit = nullptr)
     {
         Hardcore::BroadcastHardcoreDeath(player, killerUnit);
     }
 
     bool isItemInsured(Item* item)
     {
         if (!item)
             return false;
 
         uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
         for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
         {
             if (item->GetEnchantmentId(EnchantmentSlot(i)) == insuranceEnchantId)
             {
                 return true;
             }
         }
         
         return false;
     }
 
     bool canItemBeLost(Item* item)
     {
         if (!item)
             return false;
 
         ItemTemplate const* proto = item->GetTemplate();
         if (!proto)
             return false;
 
         if (isItemInsured(item))
             return false;
 
         if (proto->StartQuest > 0 || proto->Class == ITEM_CLASS_QUEST || (proto->BagFamily & BAG_FAMILY_MASK_QUEST_ITEMS))
             return false;
 
         if (proto->Class == ITEM_CLASS_KEY)
             return false;
 
         if (proto->Class == ITEM_CLASS_REAGENT)
             return false;
 
         if (proto->Class == ITEM_CLASS_GLYPH)
             return false;
 
         return true;
     }
 
     void handleItemLossAfter80(Player* player)
     {
         if (!player || player->GetLevel() < GetMaxLevel())
             return;
 
         if (player->InBattleground() || player->InArena())
             return;
 
         Map* map = player->GetMap();
         if (!map)
             return;
 
         if (!map->IsDungeon() || map->GetInstanceId() == 0)
         {
             return;
         }
 
         bool isHeroic = map->IsHeroic();
         uint32 itemsToLose = isHeroic ? 3 : 1;
         
         std::vector<std::pair<Item*, uint8>> eligibleItems;
 
         for (uint8 slot = 0; slot < 19; ++slot)
         {
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                 continue;
 
             if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
             {
                 if (canItemBeLost(item))
                 {
                     eligibleItems.push_back(std::make_pair(item, slot));
                 }
             }
         }
 
         if (eligibleItems.size() > 0)
         {
             uint32 itemsLost = std::min(itemsToLose, (uint32)eligibleItems.size());
             std::vector<std::string> lostItemNames;
             lostItemNames.reserve(itemsLost);
             
             for (uint32 i = 0; i < itemsLost; ++i)
             {
                 if (eligibleItems.empty())
                     break;
                     
                 uint32 randomIndex = urand(0, eligibleItems.size() - 1);
                 Item* itemToLose = eligibleItems[randomIndex].first;
                 
                 if (itemToLose)
                 {
                     std::string itemName = itemToLose->GetTemplate()->Name1;
                     if (ItemLocale const* il = sObjectMgr->GetItemLocale(itemToLose->GetEntry()))
                     {
                         ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
                     }
                     lostItemNames.push_back(itemName);
                     player->DestroyItem(itemToLose->GetBagSlot(), itemToLose->GetSlot(), true);
                     eligibleItems.erase(eligibleItems.begin() + randomIndex);
                 }
             }
 
             if (!lostItemNames.empty())
             {
                 std::string joinedNames;
                 for (size_t i = 0; i < lostItemNames.size(); ++i)
                 {
                     if (i > 0)
                         joinedNames += ", ";
                     joinedNames += lostItemNames[i];
                 }
 
                 ChatHandler(player->GetSession()).PSendSysMessage(
                     "|cFFFF0000[Хардкор]|r При смерти вы потеряли предметы: %s", joinedNames.c_str());
             }
         }
     }
 };
 
 class HardcoreFallTracker : public UnitScript
 {
 public:
     HardcoreFallTracker() : UnitScript("hardcore_fall_tracker")
     {
     }
 
     void OnUnitDeath(Unit* unit, Unit* killer) override
     {
         // Смерть без убийцы (падение, лава, урон окружения) — рассылаем один раз с "Неизвестный враг"
         if (unit && unit->IsPlayer() && !killer && Hardcore::IsHardcoreEnabledForPlayer(unit->ToPlayer()))
             Hardcore::BroadcastHardcoreDeath(unit->ToPlayer(), nullptr);
     }
 
     uint32 DealDamage(Unit* AttackerUnit, Unit* pVictim, uint32 damage, DamageEffectType damagetype) override
     {
         if (pVictim && pVictim->IsPlayer() && AttackerUnit == pVictim && damagetype == SELF_DAMAGE)
         {
             Player* player = pVictim->ToPlayer();
             if (player && player->GetHealth() <= damage)
             {
                 if (player->GetMap() && player->GetMap()->IsDungeon())
                 {
                     Hardcore::playersDiedFromFall.insert(player->GetGUID());
                 }
             }
         }
         return damage;
     }
 };
 
 class HardModeServerScript : public ServerScript
 {
 public:
     HardModeServerScript() : ServerScript("mod-hardcore", {
         SERVERHOOK_CAN_PACKET_RECEIVE
     })
     {
     }

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (!session)
            return true;

         Player* player = session->GetPlayer();
         if (!player || player->IsGameMaster())
             return true;

         uint16 opCode = packet.GetOpcode();

         if (opCode == CMSG_CANCEL_AURA)
         {
             WorldPacket copy(packet);
             uint32 spellId = 0;
             copy >> spellId;

             uint32 hardcoreAuraSpell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId", 0);
             uint32 aura80Spell = sConfigMgr->GetOption<uint32>("ModHardcore.AuraSpellId80", 0);

             if ((hardcoreAuraSpell > 0 && spellId == hardcoreAuraSpell) ||
                 (aura80Spell > 0 && spellId == aura80Spell))
             {
                 ApplyHardcoreAuraHelper(player);
                 return false;
             }
         }

         // Prevent client from toggling PvP if feature enabled and player below max level
         if (opCode == CMSG_TOGGLE_PVP)
         {
            if (sConfigMgr->GetOption<bool>("ModHardcore.ForceDisablePvPUntilMaxLevel", false) && player->IsHardcore() && player->GetLevel() < GetMaxLevel())
            {
                ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[PvP]|r PvP принудительно отключён до уровня {}.", GetMaxLevel());
                 return false;
             }
         }

         if (opCode == CMSG_CANCEL_TEMP_ENCHANTMENT)
         {
             WorldPacket copy(packet);
             uint32 equipmentSlot = 0;
             copy >> equipmentSlot;

             if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot))
             {
                 uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);

                 for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
                 {
                     EnchantmentSlot slot = EnchantmentSlot(i);
                     uint32 enchantId = item->GetEnchantmentId(slot);

                     if (enchantId == insuranceEnchantId)
                     {
                         ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Вы не можете снять страховку вручную. Она снимется автоматически через 3 часа.");
                         return false;
                     }
                 }
             }
         }

        if (player->isDead())
        {
            uint8 level = player->GetLevel();
            int minLevel = sConfigMgr->GetOption<int>("ModHardcoreMinLevel.Enable", 1);
            uint8 maxLevel = GetMaxLevel();
            if (player->IsHardcore() && level >= minLevel && level < maxLevel)
            {
                 switch (opCode)
                 {
                     case SMSG_PRE_RESURRECT:
                     case CMSG_HEARTH_AND_RESURRECT:
                     case SMSG_RESURRECT_REQUEST:
                     case CMSG_RESURRECT_RESPONSE:
                         return false;
                     case CMSG_RECLAIM_CORPSE:
                         ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Хардкор]|r Вы останетесь призраком навсегда. Мертвые не могут быть воскрешены.");
                         return false;
                     case CMSG_SPIRIT_HEALER_ACTIVATE:
                     case SMSG_SPIRIT_HEALER_CONFIRM:
                         ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Хардкор]|r Вы останетесь призраком навсегда. Мертвые не могут быть воскрешены.");
                         return false;
                     case CMSG_GM_RESURRECT:
                         if (!sConfigMgr->GetOption<bool>("ModHardcoreGMCanResurrect.Enable", false))
                             return false;
                         break;
                     default:
                         break;
                 }
             }
         }

         return true;
     }
 };
 
 class all_item_hardcore_insurance : public AllItemScript
 {
 public:
     all_item_hardcore_insurance() : AllItemScript("all_item_hardcore_insurance") { }
 
     bool CanItemUse(Player* player, Item* item, SpellCastTargets const& targets) override
     {
         if (!player || !item)
             return false;
 
         if (item->GetEntry() != 56812 && item->GetEntry() != 56813)
             return false;
 
         LOG_INFO("mod-hardcore", "[Insurance] AllItemScript CanItemUse called - Player: {}, Item Entry: {}", 
                  player->GetName(), item->GetEntry());
 
         return handleInsuranceScroll(player, item, targets);
     }
 
 private:
     bool handleInsuranceScroll(Player* player, Item* item, SpellCastTargets const& targets)
     {
         if (player->GetLevel() < GetMaxLevel())
         {
             LOG_DEBUG("mod-hardcore", "[Insurance] Player {} level {} < 80, denied", 
                       player->GetName(), player->GetLevel());
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Вы не можете использовать свиток страхования в хардкор режиме (до 80 уровня).");
             return true;
         }
 
         uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
         LOG_DEBUG("mod-hardcore", "[Insurance] InsuranceEnchantId: {}", insuranceEnchantId);
         
         std::vector<std::pair<Item*, uint8>> insurableItems;
         
         for (uint8 slot = 0; slot < 19; ++slot)
         {
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                 continue;
 
             Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
             if (equippedItem)
             {
                 LOG_DEBUG("mod-hardcore", "[Insurance] Checking slot {} - Item: {} (Entry: {})", 
                          slot, equippedItem->GetGUID().ToString(), equippedItem->GetEntry());
                 if (canInsureItem(equippedItem, insuranceEnchantId))
                 {
                     LOG_DEBUG("mod-hardcore", "[Insurance] Item {} in slot {} can be insured", 
                              equippedItem->GetEntry(), slot);
                     insurableItems.push_back(std::make_pair(equippedItem, slot));
                 }
                 else
                 {
                     LOG_DEBUG("mod-hardcore", "[Insurance] Item {} in slot {} cannot be insured", 
                              equippedItem->GetEntry(), slot);
                 }
             }
         }
 
         LOG_WARN("mod-hardcore", "[Insurance] Found {} insurable items", insurableItems.size());
 
         if (insurableItems.empty())
         {
             LOG_DEBUG("mod-hardcore", "[Insurance] No insurable items found for player {}", player->GetName());
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r У вас нет незастрахованных надетых предметов для страхования.");
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Все надетые предметы уже застрахованы или являются персональными.");
             return true;
         }
 
         LOG_DEBUG("mod-hardcore", "[Insurance] Opening gossip menu for player {}", player->GetName());
         ClearGossipMenuFor(player);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Выберите предмет для страхования:", GOSSIP_SENDER_MAIN, 0);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, " ", GOSSIP_SENDER_MAIN, 0);
         
         uint32 itemEntry = item->GetEntry();
         for (size_t i = 0; i < insurableItems.size(); ++i)
         {
             Item* targetItem = insurableItems[i].first;
             std::string itemName = targetItem->GetTemplate()->Name1;
             if (ItemLocale const* il = sObjectMgr->GetItemLocale(targetItem->GetEntry()))
             {
                 ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
             }
             
             uint32 itemQuality = targetItem->GetTemplate()->Quality;
             std::string qualityColor = "9d9d9d";
             switch (itemQuality)
             {
                 case ITEM_QUALITY_POOR: qualityColor = "9d9d9d"; break;
                 case ITEM_QUALITY_NORMAL: qualityColor = "ffffff"; break;
                 case ITEM_QUALITY_UNCOMMON: qualityColor = "1eff00"; break;
                 case ITEM_QUALITY_RARE: qualityColor = "0070dd"; break;
                 case ITEM_QUALITY_EPIC: qualityColor = "a335ee"; break;
                 case ITEM_QUALITY_LEGENDARY: qualityColor = "ff8000"; break;
                 case ITEM_QUALITY_ARTIFACT: qualityColor = "e6cc80"; break;
                 case ITEM_QUALITY_HEIRLOOM: qualityColor = "e6cc80"; break;
             }
             
             std::string itemText = "|cff" + qualityColor + itemName + "|r";
             
             uint32 action = (itemEntry * 10000) + insurableItems[i].second + 1000;
             LOG_DEBUG("mod-hardcore", "[Insurance] Adding gossip item: {} (slot {}) with action {}", 
                      itemName, insurableItems[i].second, action);
             AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, itemText, GOSSIP_SENDER_MAIN, action);
         }
         
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, " ", GOSSIP_SENDER_MAIN, 0);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Отмена", GOSSIP_SENDER_MAIN, (itemEntry * 10000) + 9999);
         
         LOG_WARN("mod-hardcore", "[Insurance] Sending gossip menu to player {} with {} items", 
                  player->GetName(), insurableItems.size());
         SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
         return true;
     }
 
     bool canInsureItem(Item* item, uint32 insuranceEnchantId)
     {
         if (!item)
         {
             LOG_DEBUG("mod-hardcore", "[Insurance] canInsureItem - NULL item");
             return false;
         }
 
         bool allowSoulbound = sConfigMgr->GetOption<bool>("ModHardcore.AllowSoulboundInsurance", true);
         if (!allowSoulbound && item->IsSoulBound())
         {
             LOG_INFO("mod-hardcore", "[Insurance] canInsureItem - Item {} is soulbound and insurance is disabled by config", item->GetEntry());
             return false;
         }
 
         bool alreadyInsured = false;
         for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
         {
             uint32 enchantId = item->GetEnchantmentId(EnchantmentSlot(i));
             if (enchantId == insuranceEnchantId)
             {
                 LOG_WARN("mod-hardcore", "[Insurance] canInsureItem - Item {} already insured (enchant {} in slot {})", 
                          item->GetEntry(), insuranceEnchantId, i);
                 alreadyInsured = true;
                 break;
             }
             if (enchantId > 0)
             {
                 LOG_DEBUG("mod-hardcore", "[Insurance] canInsureItem - Item {} has enchant {} in slot {}", 
                          item->GetEntry(), enchantId, i);
             }
         }
 
         if (alreadyInsured)
         {
             return false;
         }
 
         LOG_INFO("mod-hardcore", "[Insurance] canInsureItem - Item {} can be insured (Bonding: {})", 
                  item->GetEntry(), item->GetTemplate()->Bonding);
         return true;
     }
 };
 class item_hardcore_insurance : public ItemScript
 {
 public:
     item_hardcore_insurance() : ItemScript("item_hardcore_insurance") { }
 
     bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
     {
         LOG_WARN("mod-hardcore", "[Insurance] OnUse called - Player: {}, Item Entry: {}", 
                  player ? player->GetName() : "NULL", item ? item->GetEntry() : 0);
         
         if (!player || !item)
         {
             LOG_ERROR("mod-hardcore", "[Insurance] OnUse - NULL player or item!");
             return false;
         }
 
         LOG_DEBUG("mod-hardcore", "[Insurance] OnUse - Player: {} (Level: {}), Item: {} (Entry: {})", 
                   player->GetName(), player->GetLevel(), item->GetGUID().ToString(), item->GetEntry());
 
         if (player->GetLevel() < GetMaxLevel())
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Вы не можете использовать свиток страхования в хардкор режиме (до 80 уровня).");
             return false;
         }
 
         uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
         LOG_DEBUG("mod-hardcore", "[Insurance] InsuranceEnchantId: {}", insuranceEnchantId);
         
         std::vector<std::pair<Item*, uint8>> insurableItems;
         
         for (uint8 slot = 0; slot < 19; ++slot)
         {
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                 continue;
 
             Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
             if (equippedItem)
             {
                 LOG_DEBUG("mod-hardcore", "[Insurance] Checking slot {} - Item: {} (Entry: {})", 
                          slot, equippedItem->GetGUID().ToString(), equippedItem->GetEntry());
                 if (canInsureItem(equippedItem, insuranceEnchantId))
                 {
                     LOG_DEBUG("mod-hardcore", "[Insurance] Item {} in slot {} can be insured", 
                              equippedItem->GetEntry(), slot);
                     insurableItems.push_back(std::make_pair(equippedItem, slot));
                 }
                 else
                 {
                     LOG_DEBUG("mod-hardcore", "[Insurance] Item {} in slot {} cannot be insured", 
                              equippedItem->GetEntry(), slot);
                 }
             }
         }
 
         LOG_WARN("mod-hardcore", "[Insurance] Found {} insurable items", insurableItems.size());
 
         if (insurableItems.empty())
         {
             LOG_DEBUG("mod-hardcore", "[Insurance] No insurable items found for player {}", player->GetName());
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r У вас нет незастрахованных надетых предметов для страхования.");
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Все надетые предметы уже застрахованы или являются персональными.");
             return false;
         }
 
         ClearGossipMenuFor(player);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Выберите предмет для страхования:", GOSSIP_SENDER_MAIN, 0);
         
         uint32 itemEntry = item->GetEntry();
         for (size_t i = 0; i < insurableItems.size(); ++i)
         {
             Item* targetItem = insurableItems[i].first;
             std::string itemName = targetItem->GetTemplate()->Name1;
             if (ItemLocale const* il = sObjectMgr->GetItemLocale(targetItem->GetEntry()))
             {
                 ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
             }
             
             uint32 itemQuality = targetItem->GetTemplate()->Quality;
             std::string qualityColor = "9d9d9d";
             switch (itemQuality)
             {
                 case ITEM_QUALITY_POOR: qualityColor = "9d9d9d"; break;
                 case ITEM_QUALITY_NORMAL: qualityColor = "ffffff"; break;
                 case ITEM_QUALITY_UNCOMMON: qualityColor = "1eff00"; break;
                 case ITEM_QUALITY_RARE: qualityColor = "0070dd"; break;
                 case ITEM_QUALITY_EPIC: qualityColor = "a335ee"; break;
                 case ITEM_QUALITY_LEGENDARY: qualityColor = "ff8000"; break;
                 case ITEM_QUALITY_ARTIFACT: qualityColor = "e6cc80"; break;
                 case ITEM_QUALITY_HEIRLOOM: qualityColor = "e6cc80"; break;
             }
             
             std::string itemText = "|cff" + qualityColor + itemName + "|r";
             
             uint32 action = (itemEntry * 10000) + insurableItems[i].second + 1000;
             AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, itemText, GOSSIP_SENDER_MAIN, action);
         }
         
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Отмена", GOSSIP_SENDER_MAIN, (itemEntry * 10000) + 9999);
         
         SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
         return true;
     }
 
     void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
     {
         LOG_INFO("mod-hardcore", "[Insurance] OnGossipSelect called - Player: {}, Item Entry: {}, Action: {}", 
                  player ? player->GetName() : "NULL", item ? item->GetEntry() : 0, action);
         
         if (!player || !item)
         {
             LOG_ERROR("mod-hardcore", "[Insurance] OnGossipSelect - NULL player or item!");
             return;
         }
 
         if (item->GetEntry() != 56812 && item->GetEntry() != 56813)
         {
             LOG_WARN("mod-hardcore", "[Insurance] OnGossipSelect - Item {} is not an insurance scroll!", item->GetEntry());
             CloseGossipMenuFor(player);
             return;
         }
 
         uint32 remainder = action % 10000;
 
         if (remainder == 9999)
         {
             CloseGossipMenuFor(player);
             return;
         }
 
         if (remainder >= 1000 && remainder < 1019)
         {
             uint8 slot = remainder - 1000;
             
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
             {
                 CloseGossipMenuFor(player);
                 return;
             }
 
             Item* targetItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
             if (!targetItem)
             {
                 ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Предмет не найден.");
                 CloseGossipMenuFor(player);
                 return;
             }
 
             if (!ApplyInsuranceToItemStatic(player, item, targetItem))
             {
                 CloseGossipMenuFor(player);
                 return;
             }
 
             CloseGossipMenuFor(player);
         }
     }
 
 private:
     bool canInsureItem(Item* item, uint32 insuranceEnchantId)
     {
         if (!item)
         {
             LOG_DEBUG("mod-hardcore", "[Insurance] canInsureItem - NULL item");
             return false;
         }
 
         bool alreadyInsured = false;
         for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
         {
             uint32 enchantId = item->GetEnchantmentId(EnchantmentSlot(i));
             if (enchantId == insuranceEnchantId)
             {
                 LOG_WARN("mod-hardcore", "[Insurance] canInsureItem - Item {} already insured (enchant {} in slot {})", 
                          item->GetEntry(), insuranceEnchantId, i);
                 alreadyInsured = true;
                 break;
             }
             if (enchantId > 0)
             {
                 LOG_DEBUG("mod-hardcore", "[Insurance] canInsureItem - Item {} has enchant {} in slot {}", 
                          item->GetEntry(), enchantId, i);
             }
         }
 
         if (alreadyInsured)
         {
             return false;
         }
 
         LOG_INFO("mod-hardcore", "[Insurance] canInsureItem - Item {} can be insured (Bonding: {})", 
                  item->GetEntry(), item->GetTemplate()->Bonding);
         return true;
     }
 };
 
 class npc_hardcore_insurance : public CreatureScript
 {
 public:
     npc_hardcore_insurance() : CreatureScript("npc_hardcore_insurance") { }
 
     bool OnGossipHello(Player* player, Creature* creature) override
     {
         if (!player || !creature)
             return false;
 
         if (player->GetLevel() < GetMaxLevel())
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Вы не можете использовать страхование в хардкор режиме (до 80 уровня).");
             CloseGossipMenuFor(player);
             return true;
         }
 
         if (!player->HasItemCount(56812, 1))
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r У вас нет свитка страхования (ID: 56812).");
             CloseGossipMenuFor(player);
             return true;
         }
 
         uint32 insuranceEnchantId = sConfigMgr->GetOption<uint32>("ModHardcore.InsuranceEnchantId", 9999);
         
         std::vector<std::pair<Item*, uint8>> insurableItems;
         
         for (uint8 slot = 0; slot < 19; ++slot)
         {
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                 continue;
 
             Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
             if (equippedItem)
             {
                 if (canInsureItemNPC(equippedItem, insuranceEnchantId))
                 {
                     insurableItems.push_back(std::make_pair(equippedItem, slot));
                 }
             }
         }
 
         if (insurableItems.empty())
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r У вас нет незастрахованных надетых предметов для страхования.");
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Все надетые предметы уже застрахованы или являются персональными.");
             CloseGossipMenuFor(player);
             return true;
         }
 
         ClearGossipMenuFor(player);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Выберите предмет для страхования:", GOSSIP_SENDER_MAIN, 0);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, " ", GOSSIP_SENDER_MAIN, 0);
         
         for (size_t i = 0; i < insurableItems.size(); ++i)
         {
             Item* targetItem = insurableItems[i].first;
             std::string itemName = targetItem->GetTemplate()->Name1;
             if (ItemLocale const* il = sObjectMgr->GetItemLocale(targetItem->GetEntry()))
             {
                 ObjectMgr::GetLocaleString(il->Name, LOCALE_ruRU, itemName);
             }
             
             uint32 itemQuality = targetItem->GetTemplate()->Quality;
             std::string qualityColor = "9d9d9d";
             switch (itemQuality)
             {
                 case ITEM_QUALITY_POOR: qualityColor = "9d9d9d"; break;
                 case ITEM_QUALITY_NORMAL: qualityColor = "ffffff"; break;
                 case ITEM_QUALITY_UNCOMMON: qualityColor = "1eff00"; break;
                 case ITEM_QUALITY_RARE: qualityColor = "0070dd"; break;
                 case ITEM_QUALITY_EPIC: qualityColor = "a335ee"; break;
                 case ITEM_QUALITY_LEGENDARY: qualityColor = "ff8000"; break;
                 case ITEM_QUALITY_ARTIFACT: qualityColor = "e6cc80"; break;
                 case ITEM_QUALITY_HEIRLOOM: qualityColor = "e6cc80"; break;
             }
             
             std::string itemText = "|cff" + qualityColor + itemName + "|r";
             
             uint32 action = insurableItems[i].second + 1000;
             AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, itemText, GOSSIP_SENDER_MAIN, action);
         }
         
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, " ", GOSSIP_SENDER_MAIN, 0);
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Отмена", GOSSIP_SENDER_MAIN, 9999);
         
         SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
         return true;
     }
 
     bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
     {
         if (!player || !creature)
             return false;
 
         if (player->GetLevel() < GetMaxLevel())
         {
             CloseGossipMenuFor(player);
             return true;
         }
 
         Item* scrollItem = player->GetItemByEntry(56812);
         if (!scrollItem)
         {
             ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r У вас нет свитка страхования (ID: 56812).");
             CloseGossipMenuFor(player);
             return true;
         }
 
         if (action == 9999)
         {
             CloseGossipMenuFor(player);
             return true;
         }
 
         if (action >= 1000 && action < 1019)
         {
             uint8 slot = action - 1000;
             
             if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
             {
                 CloseGossipMenuFor(player);
                 return true;
             }
 
             Item* targetItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
             if (!targetItem)
             {
                 ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[Страхование]|r Предмет не найден.");
                 CloseGossipMenuFor(player);
                 return true;
             }
 
             if (ApplyInsuranceToItemStatic(player, scrollItem, targetItem))
             {
                 CloseGossipMenuFor(player);
             }
             else
             {
                 CloseGossipMenuFor(player);
             }
         }
 
         return true;
     }
 
 private:
     bool canInsureItemNPC(Item* item, uint32 insuranceEnchantId)
     {
         if (!item)
         {
             return false;
         }
 
         bool allowSoulbound = sConfigMgr->GetOption<bool>("ModHardcore.AllowSoulboundInsurance", true);
         if (!allowSoulbound && item->IsSoulBound())
         {
             return false;
         }
 
         for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
         {
             uint32 enchantId = item->GetEnchantmentId(EnchantmentSlot(i));
             if (enchantId == insuranceEnchantId)
             {
                 return false;
             }
         }
 
         return true;
     }
 };

// Hardcore: block group from joining BG queue if any member is hardcore
class HardcoreGroupScript : public GroupScript
{
public:
    HardcoreGroupScript() : GroupScript("hardcore_group_script", { GROUPHOOK_CAN_GROUP_JOIN_BATTLEGROUND_QUEUE }) { }

    bool CanGroupJoinBattlegroundQueue(Group const* /*group*/, Player* member,
        Battleground const* /*bgTemplate*/, uint32 /*MinPlayerCount*/, bool /*isRated*/, uint32 /*arenaSlot*/) override
    {
        if (member && member->IsHardcore())
        {
            // Block only during hardcore leveling phase, allow BG at max level
            if (member->GetLevel() < GetMaxLevel())
                return false;
        }
        return true;
    }
};

// Hardcore: +50% damage taken
class HardcoreUnitScript : public UnitScript
{
public:
    HardcoreUnitScript() : UnitScript("hardcore_unit_script", true, {}) { }

    uint32 DealDamage(Unit* /*AttackerUnit*/, Unit* pVictim, uint32 damage, DamageEffectType /*damagetype*/) override
    {
        if (pVictim && pVictim->IsPlayer() && damage > 0)
        {
            Player* player = pVictim->ToPlayer();
            if (player && player->IsHardcore())
            {
                uint8 maxLevel = GetMaxLevel();
                if (player->GetLevel() < maxLevel)
                    return damage + (damage / 2);
            }
        }
        return damage;
    }
};

// Hardcore: block beneficial spells from other players on hardcore target; forbid Reincarnation, Divine Intervention, Soulstone
class HardcoreAllSpellScript : public AllSpellScript
{
public:
    HardcoreAllSpellScript() : AllSpellScript("hardcore_all_spell_script", { ALLSPELLHOOK_ON_SPELL_CHECK_CAST }) { }

    void OnSpellCheckCast(Spell* spell, bool /*strict*/, SpellCastResult& res) override
    {
        if (!spell || res != SPELL_CAST_OK)
            return;

        Unit* caster = spell->GetCaster();
        Unit* target = spell->m_targets.GetUnitTarget();
        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!caster || !spellInfo)
            return;

        Player* casterPlayer = caster->ToPlayer();
        Player* targetPlayer = target ? target->ToPlayer() : nullptr;

        // Forbidden spells for hardcore: Reincarnation 20608, Divine Intervention 19752, Soulstone 20707 (and Create Soulstone 47884)
        static uint32 const forbiddenSpells[] = { 20608, 19752, 20707, 47884 };
        if (casterPlayer && casterPlayer->IsHardcore())
        {
            uint8 maxLevel = GetMaxLevel();
            if (casterPlayer->GetLevel() >= maxLevel)
                return;
            uint32 id = spellInfo->Id;
            for (uint32 sid : forbiddenSpells)
                if (id == sid)
                {
                    res = SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                    return;
                }
        }

        // Other players cannot cast beneficial spells on hardcore player
        if (targetPlayer && targetPlayer->IsHardcore() && casterPlayer && casterPlayer->GetGUID() != targetPlayer->GetGUID())
        {
            uint8 maxLevel = GetMaxLevel();
            if (targetPlayer->GetLevel() >= maxLevel)
                return;
            if (spellInfo->IsPositive())
            {
                res = SPELL_FAILED_BAD_TARGETS;
                return;
            }
        }
    }
};

 void AddSC_mod_hardcore()
 {
     new HardcoreMode();
     new HardModeServerScript();
     new all_item_hardcore_insurance();
     new item_hardcore_insurance();
     new HardcoreFallTracker();
     new npc_hardcore_insurance();
     new HardcoreGroupScript();
     new HardcoreUnitScript();
     new HardcoreAllSpellScript();
 }