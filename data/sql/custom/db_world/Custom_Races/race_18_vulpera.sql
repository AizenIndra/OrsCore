-- Race 18: Vulpera
-- Race Mask: 131072 (1<<17)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(18, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(18, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(18, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(18, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(18, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(18, 1, 0, 6603, 0),
(18, 1, 72, 6603, 0),
(18, 1, 73, 78, 0),
(18, 1, 74, 20572, 0),
(18, 1, 84, 6603, 0),
(18, 1, 96, 6603, 0),
(18, 1, 108, 6603, 0),
(18, 3, 0, 6603, 0),
(18, 3, 1, 2973, 0),
(18, 3, 2, 75, 0),
(18, 3, 11, 20572, 0),
(18, 4, 0, 6603, 0),
(18, 4, 1, 1752, 0),
(18, 4, 2, 2098, 0),
(18, 4, 3, 2764, 0),
(18, 4, 4, 20572, 0),
(18, 5, 0, 585, 0),
(18, 5, 1, 2050, 0),
(18, 5, 11, 20572, 0),
(18, 6, 0, 6603, 0),
(18, 6, 1, 49576, 0),
(18, 6, 2, 45477, 0),
(18, 6, 3, 45462, 0),
(18, 6, 4, 45902, 0),
(18, 6, 5, 47541, 0),
(18, 6, 10, 20572, 0),
(18, 7, 0, 6603, 0),
(18, 7, 1, 403, 0),
(18, 7, 2, 331, 0),
(18, 7, 3, 33697, 0),
(18, 9, 0, 686, 0),
(18, 9, 1, 687, 0),
(18, 9, 2, 33702, 0),
(18, 11, 0, 5176, 0),
(18, 11, 1, 5185, 0),
(18, 11, 85, 6603, 0),
(18, 11, 97, 6603, 0),
(18, 11, 109, 6603, 0),
(18, 11, 2, 20572, 0),
(18, 11, 73, 6603, 0),
(18, 11, 76, 20572, 0),
(18, 2, 0, 6603, 0),
(18, 2, 1, 21084, 0),
(18, 2, 2, 635, 0),
(18, 2, 11, 20572, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 131072 for Vulpera
-- Copying from Orc (2) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '131072', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 2;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 131072
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Vulpera can be Shaman - class 7)
SET @VulperaFireTotem := 30758;
SET @VulperaEarthTotem := 30757;
SET @VulperaWaterTotem := 30759;
SET @VulperaAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 18;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 18, @VulperaFireTotem),
(2, 18, @VulperaEarthTotem),
(3, 18, @VulperaWaterTotem),
(4, 18, @VulperaAirTotem);

-- playercreateinfo_item
-- Copying starting items from Orc (2) for Vulpera (18)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 18, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 2;

