-- Race 9: Goblin
-- Race Mask: 256 (1<<8)

-- playercreateinfo
INSERT IGNORE INTO `playercreateinfo` VALUES
(9, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(9, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(9, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(9, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(9, 12, 1, 14, -618.518, -4251.67, 38.718, 0);


-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(9, 1, 0, 6603, 0),
(9, 1, 72, 6603, 0),
(9, 1, 73, 78, 0),
(9, 1, 74, 2764, 0),
(9, 1, 75, 20589, 0),
(9, 1, 84, 6603, 0),
(9, 1, 96, 6603, 0),
(9, 1, 108, 6603, 0),
(9, 3, 0, 6603, 0),
(9, 3, 1, 2973, 0),
(9, 3, 2, 75, 0),
(9, 3, 3, 20589, 0),
(9, 4, 0, 6603, 0),
(9, 4, 1, 1752, 0),
(9, 4, 2, 2098, 0),
(9, 4, 3, 2764, 0),
(9, 4, 4, 20589, 0),
(9, 5, 0, 585, 0),
(9, 5, 1, 2050, 0),
(9, 5, 2, 20589, 0),
(9, 6, 0, 6603, 0),
(9, 6, 1, 49576, 0),
(9, 6, 2, 45477, 0),
(9, 6, 3, 45462, 0),
(9, 6, 4, 45902, 0),
(9, 6, 5, 47541, 0),
(9, 6, 10, 20589, 0),
(9, 7, 0, 6603, 0),
(9, 7, 1, 403, 0),
(9, 7, 2, 331, 0),
(9, 7, 3, 20589, 0),
(9, 8, 0, 133, 0),
(9, 8, 1, 168, 0),
(9, 8, 2, 20589, 0),
(9, 9, 0, 686, 0),
(9, 9, 1, 687, 0),
(9, 9, 2, 20589, 0),
(9, 11, 0, 5176, 0),
(9, 11, 1, 5185, 0),
(9, 11, 2, 20589, 0),
(9, 2, 0, 6603, 0),
(9, 2, 1, 21084, 0),
(9, 2, 2, 635, 0),
(9, 2, 3, 20589, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 256 for Goblin
-- Copying from Troll (128) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '256', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 128;

-- playercreateinfo_item
-- Copying starting items from Troll (8) for Goblin (9)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 9, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 8;

-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 256
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Goblin can be Shaman - class 7)
SET @GoblinFireTotem := 30758;
SET @GoblinEarthTotem := 30757;
SET @GoblinWaterTotem := 30759;
SET @GoblinAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 9;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 9, @GoblinFireTotem),
(2, 9, @GoblinEarthTotem),
(3, 9, @GoblinWaterTotem),
(4, 9, @GoblinAirTotem);

