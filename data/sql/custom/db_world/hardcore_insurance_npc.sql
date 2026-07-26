-- NPC for Hardcore Insurance (Scroll 56812)
-- This NPC allows players to insure items using scroll 56812
-- Entry: 999999
-- ScriptName: npc_hardcore_insurance

-- Delete existing entries (if any) to allow safe re-execution
DELETE FROM `creature_template_addon` WHERE `entry` = 999999;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 999999;
DELETE FROM `creature_template` WHERE `entry` = 999999;

-- Insert into creature_template
INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, 
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, 
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, 
    `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, 
    `scale`, `rank`, `dmgschool`, `DamageModifier`, 
    `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, 
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, 
    `family`, `type`, `type_flags`, 
    `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, 
    `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, 
    `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, 
    `RacialLeader`, `movementId`, `RegenHealth`, 
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, 
    `ScriptName`, `VerifiedBuild`
) VALUES (
    999999,  -- entry
    0,       -- difficulty_entry_1
    0,       -- difficulty_entry_2
    0,       -- difficulty_entry_3
    0,       -- KillCredit1
    0,       -- KillCredit2
    'Страховщик',  -- name
    'Мастер страхования',  -- subname
    NULL,    -- IconName
    0,       -- gossip_menu_id
    80,      -- minlevel
    80,      -- maxlevel
    2,       -- exp (WotLK)
    35,      -- faction (Friendly)
    1,       -- npcflag (Gossip enabled)
    1,       -- speed_walk
    1.14286, -- speed_run
    1,       -- speed_swim
    1,       -- speed_flight
    20,      -- detection_range
    1,       -- scale
    0,       -- rank (Normal)
    0,       -- dmgschool
    1,       -- DamageModifier
    2000,    -- BaseAttackTime
    2000,    -- RangeAttackTime
    1,       -- BaseVariance
    1,       -- RangeVariance
    1,       -- unit_class (Warrior)
    0,       -- unit_flags
    2048,    -- unit_flags2
    0,       -- dynamicflags
    0,       -- family
    7,       -- type (Humanoid)
    0,       -- type_flags
    0,       -- lootid
    0,       -- pickpocketloot
    0,       -- skinloot
    0,       -- PetSpellDataId
    0,       -- VehicleId
    0,       -- mingold
    0,       -- maxgold
    '',      -- AIName
    0,       -- MovementType
    1,       -- HoverHeight
    1,       -- HealthModifier
    1,       -- ManaModifier
    1,       -- ArmorModifier
    1,       -- ExperienceModifier
    0,       -- RacialLeader
    0,       -- movementId
    1,       -- RegenHealth
    0,       -- mechanic_immune_mask
    0,       -- spell_school_immune_mask
    0,       -- flags_extra
    'npc_hardcore_insurance',  -- ScriptName
    12340    -- VerifiedBuild
);

-- Insert into creature_template_model (using standard human NPC model - DisplayID 1826)
INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`
) VALUES (
    999999,  -- CreatureID
    0,       -- Idx (first model)
    1826,    -- CreatureDisplayID (standard human NPC)
    1,       -- DisplayScale
    1,       -- Probability (100%)
    12340    -- VerifiedBuild
);

-- Insert into creature_template_addon (for basic NPC behavior)
INSERT INTO `creature_template_addon` (
    `entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`
) VALUES (
    999999,  -- entry
    0,       -- path_id
    0,       -- mount
    0,       -- bytes1
    1,       -- bytes2 (stand state)
    0,       -- emote
    0,       -- visibilityDistanceType
    NULL     -- auras
);