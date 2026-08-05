-- NPC for Hardcore Insurance (Scroll 56812)
-- Entry: 999999
-- ScriptName: npc_hardcore_insurance

DELETE FROM `creature_template_addon` WHERE `entry` = 999999;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 999999;
DELETE FROM `creature_template` WHERE `entry` = 999999;

INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`,
    `rank`, `dmgschool`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`,
    `family`, `type`, `type_flags`,
    `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`,
    `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`,
    `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`,
    `RacialLeader`, `movementId`, `RegenHealth`,
    `CreatureImmunitiesId`, `flags_extra`,
    `ScriptName`, `VerifiedBuild`
) VALUES (
    999999,
    0,
    0,
    0,
    0,
    0,
    'Страховщик',
    'Мастер страхования',
    NULL,
    0,
    80,
    80,
    2,
    35,
    1,
    1,
    1.14286,
    1,
    1,
    20,
    0,
    0,
    1,
    2000,
    2000,
    1,
    1,
    1,
    0,
    2048,
    0,
    0,
    7,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '',
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    'npc_hardcore_insurance',
    12340
);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`
) VALUES (
    999999,
    0,
    1826,
    1,
    1,
    12340
);

INSERT INTO `creature_template_addon` (
    `entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`
) VALUES (
    999999,
    0,
    0,
    0,
    1,
    0,
    0,
    NULL
);
