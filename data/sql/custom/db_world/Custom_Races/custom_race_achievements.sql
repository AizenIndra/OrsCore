-- Custom Race Achievements - Race-specific criteria data
-- This file links achievement criteria to specific races so each race gets only its own achievement

-- Delete existing entries for custom race achievements to avoid duplicates
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (13471, 13472, 13473, 13474, 13475, 13476, 13477, 13478, 13479, 13480, 13481, 13482, 13483, 13484, 13485, 13486);

-- Insert race-specific criteria data
-- Type 21 = ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE
-- value1 = class_id (0 = any class)
-- value2 = race_id

-- Achievement 4825: Вульпера 80-го уровня -> Criteria 13471 -> Race 9 (Vulpera)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13471, 21, 0, 9, '');

-- Achievement 4826: Ворген 80-го уровня -> Criteria 13472 -> Race 12 (Worgen)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13472, 21, 0, 12, '');

-- Achievement 4827: Ночнорожденный 80-го уровня -> Criteria 13473 -> Race 13 (Nightborne)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13473, 21, 0, 13, '');

-- Achievement 4828: Эльф Бездны 80-го уровня -> Criteria 13474 -> Race 14 (Void Elf)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13474, 21, 0, 14, '');

-- Achievement 4829: Эредар 80-го уровня -> Criteria 13475 -> Race 15 (Eredar)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13475, 21, 0, 15, '');

-- Achievement 4830: Эредар 80-го уровня -> Criteria 13475 -> Race 16 (LightForged)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13476, 21, 0, 16, '');

-- Achievement 4831: Эредар 80-го уровня -> Criteria 13475 -> Race 17 (zandalari)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13477, 21, 0, 17, '');

-- Achievement 4832: Эредар 80-го уровня -> Criteria 13475 -> Race 18 (vulpera)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13478, 21, 0, 18, '');

-- Achievement 4833: Эредар 80-го уровня -> Criteria 13475 -> Race 19 (pandaren_alliance)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13479, 21, 0, 19, '');

-- Achievement 4834: Эредар 80-го уровня -> Criteria 13475 -> Race 20 (pandaren_horde)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13480, 21, 0, 20, '');

-- Achievement 4835: Эредар 80-го уровня -> Criteria 13475 -> Race 21 (darkirondwarf)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13481, 21, 0, 21, '');

-- Achievement 4836: Эредар 80-го уровня -> Criteria 13475 -> Race 22 (demonhunter_horde)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13482, 21, 0, 22, '');

-- Achievement 4837: Эредар 80-го уровня -> Criteria 13475 -> Race 23 (kultiran)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13483, 21, 0, 23, '');

-- Achievement 4838: Эредар 80-го уровня -> Criteria 13475 -> Race 24 (demonhunter_alliance)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13484, 21, 0, 24, '');

-- Achievement 4839: Эредар 80-го уровня -> Criteria 13475 -> Race 25 (dracthyr_horde)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13485, 21, 0, 25, '');

-- Achievement 4840: Эредар 80-го уровня -> Criteria 13475 -> Race 26 (dracthyr_alliance)
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(13486, 21, 0, 26, '');