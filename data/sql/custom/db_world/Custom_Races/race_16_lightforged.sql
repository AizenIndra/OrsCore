-- Race 16: Lightforged Draenei
-- Race Mask: 32768 (1<<15)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(16, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(16, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(16, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(16, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(16, 1, 0, 6603, 0),
(16, 1, 72, 6603, 0),
(16, 1, 73, 78, 0),
(16, 1, 74, 28880, 0),
(16, 1, 84, 6603, 0),
(16, 1, 96, 6603, 0),
(16, 1, 108, 6603, 0),
(16, 2, 0, 6603, 0),
(16, 2, 1, 21084, 0),
(16, 2, 2, 635, 0),
(16, 2, 3, 59542, 0),
(16, 3, 0, 6603, 0),
(16, 3, 1, 2973, 0),
(16, 3, 2, 75, 0),
(16, 3, 3, 59543, 0),
(16, 4, 0, 6603, 0),
(16, 4, 1, 1752, 0),
(16, 4, 2, 2098, 0),
(16, 4, 3, 2764, 0),
(16, 4, 11, 28880, 0),
(16, 5, 0, 585, 0),
(16, 5, 1, 2050, 0),
(16, 5, 2, 59544, 0),
(16, 6, 0, 6603, 0),
(16, 6, 1, 49576, 0),
(16, 6, 2, 45477, 0),
(16, 6, 3, 45462, 0),
(16, 6, 4, 45902, 0),
(16, 6, 5, 47541, 0),
(16, 6, 10, 59545, 0),
(16, 7, 0, 6603, 0),
(16, 7, 1, 403, 0),
(16, 7, 2, 331, 0),
(16, 7, 3, 59547, 0),
(16, 8, 0, 133, 0),
(16, 8, 1, 168, 0),
(16, 8, 2, 59548, 0),
(16, 9, 0, 686, 0),
(16, 9, 1, 687, 0),
(16, 9, 11, 28880, 0),
(16, 11, 0, 5176, 0),
(16, 11, 1, 5185, 0),
(16, 11, 11, 28880, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 32768 for Lightforged Draenei
-- Copying from Draenei (1024) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '32768', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1024;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 32768
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Draenei (11) for Lightforged (16)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 16, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 11;

