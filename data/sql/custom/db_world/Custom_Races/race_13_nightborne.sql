-- Race 13: Nightborne
-- Race Mask: 4096 (1<<12)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(13, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(13, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(13, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(13, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(13, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(13, 1, 0, 6603, 0),
(13, 1, 72, 6603, 0),
(13, 1, 73, 78, 0),
(13, 1, 82, 28730, 0),
(13, 1, 84, 6603, 0),
(13, 1, 96, 6603, 0),
(13, 1, 108, 6603, 0),
(13, 2, 0, 6603, 0),
(13, 2, 1, 21084, 0),
(13, 2, 2, 635, 0),
(13, 2, 3, 28730, 0),
(13, 3, 0, 6603, 0),
(13, 3, 1, 2973, 0),
(13, 3, 2, 75, 0),
(13, 3, 3, 28730, 0),
(13, 4, 0, 6603, 0),
(13, 4, 1, 1752, 0),
(13, 4, 2, 2098, 0),
(13, 4, 3, 2764, 0),
(13, 4, 4, 25046, 0),
(13, 5, 0, 585, 0),
(13, 5, 1, 2050, 0),
(13, 5, 2, 28730, 0),
(13, 6, 0, 6603, 0),
(13, 6, 1, 49576, 0),
(13, 6, 2, 45477, 0),
(13, 6, 3, 45462, 0),
(13, 6, 4, 45902, 0),
(13, 6, 5, 47541, 0),
(13, 6, 6, 50613, 0),
(13, 8, 0, 133, 0),
(13, 8, 1, 168, 0),
(13, 8, 2, 28730, 0),
(13, 9, 0, 686, 0),
(13, 9, 1, 687, 0),
(13, 9, 2, 28730, 0),
(13, 11, 0, 5176, 0),
(13, 11, 1, 5185, 0),
(13, 11, 11, 28730, 0),
(13, 7, 0, 6603, 0),
(13, 7, 1, 403, 0),
(13, 7, 2, 331, 0),
(13, 7, 3, 20554, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 4096 for Nightborne
-- Copying from Blood Elf (512) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '4096', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 512;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 4096
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Nightborne can be Shaman - class 7)
SET @NightborneFireTotem := 30758;
SET @NightborneEarthTotem := 30757;
SET @NightborneWaterTotem := 30759;
SET @NightborneAirTotem := 30756;

DELETE FROM player_totem_model WHERE RaceID = 13;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 13, @NightborneFireTotem),
(2, 13, @NightborneEarthTotem),
(3, 13, @NightborneWaterTotem),
(4, 13, @NightborneAirTotem);

-- playercreateinfo_item
-- Copying starting items from Blood Elf (10) for Nightborne (13)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 13, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 10;

