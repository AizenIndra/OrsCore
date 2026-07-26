-- Race 19: Pandaren (Alliance)
-- Race Mask: 262144 (1<<18)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(19, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(19, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(19, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(19, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(19, 1, 0, 6603, 0),
(19, 1, 72, 6603, 0),
(19, 1, 73, 78, 0),
(19, 1, 82, 59752, 0),
(19, 1, 84, 6603, 0),
(19, 1, 96, 6603, 0),
(19, 1, 108, 6603, 0),
(19, 2, 0, 6603, 0),
(19, 2, 1, 21084, 0),
(19, 2, 2, 635, 0),
(19, 2, 11, 59752, 0),
(19, 3, 0, 6603, 0),
(19, 3, 1, 2973, 0),
(19, 3, 2, 75, 0),
(19, 3, 11, 59752, 0),
(19, 4, 0, 6603, 0),
(19, 4, 1, 1752, 0),
(19, 4, 2, 2098, 0),
(19, 4, 3, 2764, 0),
(19, 4, 11, 59752, 0),
(19, 5, 0, 585, 0),
(19, 5, 1, 2050, 0),
(19, 5, 11, 59752, 0),
(19, 6, 0, 6603, 0),
(19, 6, 1, 49576, 0),
(19, 6, 2, 45477, 0),
(19, 6, 3, 45462, 0),
(19, 6, 4, 45902, 0),
(19, 6, 5, 47541, 0),
(19, 6, 11, 59752, 0),
(19, 7, 0, 6603, 0),
(19, 7, 1, 403, 0),
(19, 7, 2, 331, 0),
(19, 7, 3, 59752, 0),
(19, 8, 0, 133, 0),
(19, 8, 1, 168, 0),
(19, 8, 11, 59752, 0),
(19, 9, 0, 686, 0),
(19, 9, 1, 687, 0),
(19, 9, 11, 59752, 0),
(19, 10, 0, 107079, 0),
(19, 10, 1, 107074, 0),
(19, 10, 11, 59752, 0),
(19, 11, 0, 5176, 0),
(19, 11, 1, 5185, 0),
(19, 11, 11, 59752, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 262144 for Pandaren Alliance
-- Copying from Human (1) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '262144', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 262144
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- player_totem_model (Pandaren Alliance can be Shaman - class 7)
SET @PandarenAllianceFireTotem := 30754;
SET @PandarenAllianceEarthTotem := 30753;
SET @PandarenAllianceWaterTotem := 30755;
SET @PandarenAllianceAirTotem := 30736;

DELETE FROM player_totem_model WHERE RaceID = 19;
INSERT INTO player_totem_model (TotemID, RaceID, ModelID) VALUES 
(1, 19, @PandarenAllianceFireTotem),
(2, 19, @PandarenAllianceEarthTotem),
(3, 19, @PandarenAllianceWaterTotem),
(4, 19, @PandarenAllianceAirTotem);

-- playercreateinfo_item
-- Copying starting items from Human (1) for Pandaren Alliance (19)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 19, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 1;

