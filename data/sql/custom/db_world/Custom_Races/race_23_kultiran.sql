-- Race 23: Kul Tiran
-- Race Mask: 4194304 (1<<22)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(23, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(23, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(23, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(23, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(23, 1, 0, 6603, 0),
(23, 1, 72, 6603, 0),
(23, 1, 73, 78, 0),
(23, 1, 82, 59752, 0),
(23, 1, 84, 6603, 0),
(23, 1, 96, 6603, 0),
(23, 1, 108, 6603, 0),
(23, 2, 0, 6603, 0),
(23, 2, 1, 21084, 0),
(23, 2, 2, 635, 0),
(23, 2, 11, 59752, 0),
(23, 3, 0, 6603, 0),
(23, 3, 1, 2973, 0),
(23, 3, 2, 75, 0),
(23, 3, 11, 59752, 0),
(23, 4, 0, 6603, 0),
(23, 4, 1, 1752, 0),
(23, 4, 2, 2098, 0),
(23, 4, 3, 2764, 0),
(23, 4, 11, 59752, 0),
(23, 5, 0, 585, 0),
(23, 5, 1, 2050, 0),
(23, 5, 11, 59752, 0),
(23, 6, 0, 6603, 0),
(23, 6, 1, 49576, 0),
(23, 6, 2, 45477, 0),
(23, 6, 3, 45462, 0),
(23, 6, 4, 45902, 0),
(23, 6, 5, 47541, 0),
(23, 6, 11, 59752, 0),
(23, 7, 0, 6603, 0),
(23, 7, 1, 403, 0),
(23, 7, 2, 331, 0),
(23, 7, 3, 59752, 0),
(23, 8, 0, 133, 0),
(23, 8, 1, 168, 0),
(23, 8, 11, 59752, 0),
(23, 9, 0, 686, 0),
(23, 9, 1, 687, 0),
(23, 9, 11, 59752, 0),
(23, 11, 0, 5176, 0),
(23, 11, 1, 5185, 0),
(23, 11, 11, 59752, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 4194304 for Kul Tiran
-- Copying from Human (1) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '4194304', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 4194304
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Kul Tiran can be Shaman - class 7)
SET @KulTiranFireTotem := 30754;
SET @KulTiranEarthTotem := 30753;
SET @KulTiranWaterTotem := 30755;
SET @KulTiranAirTotem := 30736;

DELETE FROM player_totem_model WHERE RaceID = 23;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 23, @KulTiranFireTotem),
(2, 23, @KulTiranEarthTotem),
(3, 23, @KulTiranWaterTotem),
(4, 23, @KulTiranAirTotem);

-- playercreateinfo_item
-- Copying starting items from Human (1) for Kul Tiran (23)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 23, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 1;

