-- Race 15: Eredar
-- Race Mask: 16384 (1<<14)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(15, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(15, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(15, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(15, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(15, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(15, 1, 0, 6603, 0),
(15, 1, 72, 6603, 0),
(15, 1, 73, 78, 0),
(15, 1, 74, 28880, 0),
(15, 1, 84, 6603, 0),
(15, 1, 96, 6603, 0),
(15, 1, 108, 6603, 0),
(15, 2, 0, 6603, 0),
(15, 2, 1, 21084, 0),
(15, 2, 2, 635, 0),
(15, 2, 3, 59542, 0),
(15, 3, 0, 6603, 0),
(15, 3, 1, 2973, 0),
(15, 3, 2, 75, 0),
(15, 3, 3, 59543, 0),
(15, 4, 0, 6603, 0),
(15, 4, 1, 1752, 0),
(15, 4, 2, 2098, 0),
(15, 4, 3, 2764, 0),
(15, 4, 11, 28880, 0),
(15, 5, 0, 585, 0),
(15, 5, 1, 2050, 0),
(15, 5, 2, 59544, 0),
(15, 6, 0, 6603, 0),
(15, 6, 1, 49576, 0),
(15, 6, 2, 45477, 0),
(15, 6, 3, 45462, 0),
(15, 6, 4, 45902, 0),
(15, 6, 5, 47541, 0),
(15, 6, 10, 59545, 0),
(15, 7, 0, 6603, 0),
(15, 7, 1, 403, 0),
(15, 7, 2, 331, 0),
(15, 7, 3, 59547, 0),
(15, 8, 0, 133, 0),
(15, 8, 1, 168, 0),
(15, 8, 2, 59548, 0),
(15, 9, 0, 686, 0),
(15, 9, 1, 687, 0),
(15, 9, 11, 28880, 0),
(15, 11, 0, 5176, 0),
(15, 11, 1, 5185, 0),
(15, 11, 11, 28880, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 16384 for Eredar
-- Copying from Draenei (1024) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '16384', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1024;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 16384
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Eredar can be Shaman - class 7)
SET @EredarFireTotem := 30758;
SET @EredarEarthTotem := 30757;
SET @EredarWaterTotem := 30759;
SET @EredarAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 15;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 15, @EredarFireTotem),
(2, 15, @EredarEarthTotem),
(3, 15, @EredarWaterTotem),
(4, 15, @EredarAirTotem);

-- playercreateinfo_item
-- Copying starting items from Draenei (11) for Eredar (15)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 15, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 11;

