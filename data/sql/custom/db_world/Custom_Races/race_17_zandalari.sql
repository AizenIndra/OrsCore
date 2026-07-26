-- Race 17: Zandalari Troll
-- Race Mask: 65536 (1<<16)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(17, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(17, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(17, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(17, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(17, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(17, 1, 0, 6603, 0),
(17, 1, 72, 6603, 0),
(17, 1, 73, 78, 0),
(17, 1, 74, 2764, 0),
(17, 1, 75, 26297, 0),
(17, 1, 84, 6603, 0),
(17, 1, 96, 6603, 0),
(17, 1, 108, 6603, 0),
(17, 3, 0, 6603, 0),
(17, 3, 1, 2973, 0),
(17, 3, 2, 75, 0),
(17, 3, 3, 26297, 0),
(17, 4, 0, 6603, 0),
(17, 4, 1, 1752, 0),
(17, 4, 2, 2098, 0),
(17, 4, 3, 2764, 0),
(17, 4, 4, 26297, 0),
(17, 5, 0, 585, 0),
(17, 5, 1, 2050, 0),
(17, 5, 2, 26297, 0),
(17, 6, 0, 6603, 0),
(17, 6, 1, 49576, 0),
(17, 6, 2, 45477, 0),
(17, 6, 3, 45462, 0),
(17, 6, 4, 45902, 0),
(17, 6, 5, 47541, 0),
(17, 6, 10, 26297, 0),
(17, 7, 0, 6603, 0),
(17, 7, 1, 403, 0),
(17, 7, 2, 331, 0),
(17, 7, 3, 26297, 0),
(17, 8, 0, 133, 0),
(17, 8, 1, 168, 0),
(17, 8, 2, 26297, 0),
(17, 9, 0, 686, 0),
(17, 9, 1, 687, 0),
(17, 9, 11, 20554, 0),
(17, 11, 0, 5176, 0),
(17, 11, 1, 5185, 0),
(17, 11, 11, 20554, 0),
(17, 2, 0, 6603, 0),
(17, 2, 1, 21084, 0),
(17, 2, 2, 635, 0),
(17, 2, 11, 20554, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 65536 for Zandalari Troll
-- Copying from Troll (128) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '65536', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 128;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 65536
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Zandalari can be Shaman - class 7)
SET @ZandalariFireTotem := 30758;
SET @ZandalariEarthTotem := 30757;
SET @ZandalariWaterTotem := 30759;
SET @ZandalariAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 17;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 17, @ZandalariFireTotem),
(2, 17, @ZandalariEarthTotem),
(3, 17, @ZandalariWaterTotem),
(4, 17, @ZandalariAirTotem);

-- playercreateinfo_item
-- Copying starting items from Troll (8) for Zandalari (17)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 17, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 8;

