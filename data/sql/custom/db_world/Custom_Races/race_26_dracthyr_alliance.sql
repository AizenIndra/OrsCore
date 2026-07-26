-- Race 26: Dracthyr (Alliance)
-- Race Mask: 33554432 (1<<25)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(26, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(26, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(26, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(26, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(26, 1, 0, 6603, 0),
(26, 1, 72, 6603, 0),
(26, 1, 73, 78, 0),
(26, 1, 82, 59752, 0),
(26, 1, 84, 6603, 0),
(26, 1, 96, 6603, 0),
(26, 1, 108, 6603, 0),
(26, 2, 0, 6603, 0),
(26, 2, 1, 21084, 0),
(26, 2, 2, 635, 0),
(26, 2, 11, 59752, 0),
(26, 3, 0, 6603, 0),
(26, 3, 1, 2973, 0),
(26, 3, 2, 75, 0),
(26, 3, 11, 59752, 0),
(26, 4, 0, 6603, 0),
(26, 4, 1, 1752, 0),
(26, 4, 2, 2098, 0),
(26, 4, 3, 2764, 0),
(26, 4, 11, 59752, 0),
(26, 5, 0, 585, 0),
(26, 5, 1, 2050, 0),
(26, 5, 11, 59752, 0),
(26, 6, 0, 6603, 0),
(26, 6, 1, 49576, 0),
(26, 6, 2, 45477, 0),
(26, 6, 3, 45462, 0),
(26, 6, 4, 45902, 0),
(26, 6, 5, 47541, 0),
(26, 6, 11, 59752, 0),
(26, 7, 0, 6603, 0),
(26, 7, 1, 403, 0),
(26, 7, 2, 331, 0),
(26, 7, 3, 59752, 0),
(26, 8, 0, 133, 0),
(26, 8, 1, 168, 0),
(26, 8, 11, 59752, 0),
(26, 9, 0, 686, 0),
(26, 9, 1, 687, 0),
(26, 9, 11, 59752, 0),
(26, 11, 0, 5176, 0),
(26, 11, 1, 5185, 0),
(26, 11, 11, 59752, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 33554432 for Dracthyr Alliance
-- Copying from Human (1) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '33554432', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 33554432
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Human (1) for Dracthyr Alliance (26)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 26, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 1;

