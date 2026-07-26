-- Race 24: Demon Hunter (Alliance)
-- Race Mask: 8388608 (1<<23)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(24, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(24, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(24, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(24, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(24, 1, 0, 6603, 0),
(24, 1, 72, 6603, 0),
(24, 1, 73, 78, 0),
(24, 1, 82, 58984, 0),
(24, 1, 85, 6603, 0),
(24, 1, 97, 6603, 0),
(24, 1, 109, 6603, 0),
(24, 3, 0, 6603, 0),
(24, 3, 1, 2973, 0),
(24, 3, 2, 75, 0),
(24, 3, 3, 58984, 0),
(24, 4, 0, 6603, 0),
(24, 4, 1, 1752, 0),
(24, 4, 2, 2098, 0),
(24, 4, 3, 2764, 0),
(24, 4, 11, 58984, 0),
(24, 5, 0, 585, 0),
(24, 5, 1, 2050, 0),
(24, 5, 2, 58984, 0),
(24, 6, 0, 6603, 0),
(24, 6, 1, 49576, 0),
(24, 6, 2, 45477, 0),
(24, 6, 3, 45462, 0),
(24, 6, 4, 45902, 0),
(24, 6, 5, 47541, 0),
(24, 6, 10, 58984, 0),
(24, 6, 83, 58984, 0),
(24, 11, 0, 5176, 0),
(24, 11, 1, 5185, 0),
(24, 11, 11, 58984, 0),
(24, 2, 0, 6603, 0),
(24, 2, 1, 21084, 0),
(24, 2, 2, 635, 0),
(24, 2, 11, 58984, 0),
(24, 7, 0, 6603, 0),
(24, 7, 1, 403, 0),
(24, 7, 2, 331, 0),
(24, 7, 3, 58984, 0),
(24, 8, 0, 133, 0),
(24, 8, 1, 168, 0),
(24, 8, 11, 58984, 0),
(24, 9, 0, 686, 0),
(24, 9, 1, 687, 0),
(24, 9, 11, 58984, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 8388608 for Demon Hunter Alliance
-- Copying from Night Elf (8) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '8388608', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 8;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 8388608
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Night Elf (4) for Demon Hunter Alliance (24)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 24, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 4;

