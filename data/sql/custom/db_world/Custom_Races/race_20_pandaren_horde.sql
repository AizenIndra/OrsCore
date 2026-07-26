-- Race 20: Pandaren (Horde)
-- Race Mask: 524288 (1<<19)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(20, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(20, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(20, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(20, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(20, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(20, 1, 0, 6603, 0),
(20, 1, 72, 6603, 0),
(20, 1, 73, 78, 0),
(20, 1, 74, 20572, 0),
(20, 1, 84, 6603, 0),
(20, 1, 96, 6603, 0),
(20, 1, 108, 6603, 0),
(20, 3, 0, 6603, 0),
(20, 3, 1, 2973, 0),
(20, 3, 2, 75, 0),
(20, 3, 11, 20572, 0),
(20, 4, 0, 6603, 0),
(20, 4, 1, 1752, 0),
(20, 4, 2, 2098, 0),
(20, 4, 3, 2764, 0),
(20, 4, 4, 20572, 0),
(20, 5, 0, 585, 0),
(20, 5, 1, 2050, 0),
(20, 5, 11, 20572, 0),
(20, 6, 0, 6603, 0),
(20, 6, 1, 49576, 0),
(20, 6, 2, 45477, 0),
(20, 6, 3, 45462, 0),
(20, 6, 4, 45902, 0),
(20, 6, 5, 47541, 0),
(20, 6, 10, 20572, 0),
(20, 7, 0, 6603, 0),
(20, 7, 1, 403, 0),
(20, 7, 2, 331, 0),
(20, 7, 3, 33697, 0),
(20, 9, 0, 686, 0),
(20, 9, 1, 687, 0),
(20, 9, 2, 33702, 0),
(20, 10, 0, 107079, 0),
(20, 10, 1, 107074, 0),
(20, 10, 11, 20572, 0),
(20, 11, 0, 5176, 0),
(20, 11, 1, 5185, 0),
(20, 11, 85, 6603, 0),
(20, 11, 97, 6603, 0),
(20, 11, 109, 6603, 0),
(20, 11, 2, 20572, 0),
(20, 11, 73, 6603, 0),
(20, 11, 76, 20572, 0),
(20, 2, 0, 6603, 0),
(20, 2, 1, 21084, 0),
(20, 2, 2, 635, 0),
(20, 2, 11, 20572, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 524288 for Pandaren Horde
-- Copying from Orc (2) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '524288', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 2;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 524288
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Pandaren Horde can be Shaman - class 7)
SET @PandarenHordeFireTotem := 30758;
SET @PandarenHordeEarthTotem := 30757;
SET @PandarenHordeWaterTotem := 30759;
SET @PandarenHordeAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 20;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 20, @PandarenHordeFireTotem),
(2, 20, @PandarenHordeEarthTotem),
(3, 20, @PandarenHordeWaterTotem),
(4, 20, @PandarenHordeAirTotem);

-- playercreateinfo_item
-- Copying starting items from Orc (2) for Pandaren Horde (20)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 20, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 2;

