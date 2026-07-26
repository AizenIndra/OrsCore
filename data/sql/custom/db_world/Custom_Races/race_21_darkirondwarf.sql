-- Race 21: Dark Iron Dwarf
-- Race Mask: 1048576 (1<<20)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(21, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(21, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(21, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(21, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(21, 1, 0, 6603, 0),
(21, 1, 1, 78, 0),
(21, 1, 72, 6603, 0),
(21, 1, 73, 78, 0),
(21, 1, 74, 20594, 0),
(21, 1, 75, 2481, 0),
(21, 1, 84, 6603, 0),
(21, 1, 96, 6603, 0),
(21, 1, 108, 6603, 0),
(21, 2, 0, 6603, 0),
(21, 2, 1, 21084, 0),
(21, 2, 2, 635, 0),
(21, 2, 3, 20594, 0),
(21, 2, 4, 2481, 0),
(21, 3, 0, 6603, 0),
(21, 3, 1, 2973, 0),
(21, 3, 2, 75, 0),
(21, 3, 3, 20594, 0),
(21, 3, 4, 2481, 0),
(21, 3, 75, 20594, 0),
(21, 3, 76, 2481, 0),
(21, 4, 0, 6603, 0),
(21, 4, 1, 1752, 0),
(21, 4, 2, 2098, 0),
(21, 4, 3, 2764, 0),
(21, 4, 4, 20594, 0),
(21, 4, 5, 2481, 0),
(21, 5, 0, 585, 0),
(21, 5, 1, 2050, 0),
(21, 5, 2, 20594, 0),
(21, 5, 3, 2481, 0),
(21, 6, 0, 6603, 0),
(21, 6, 1, 49576, 0),
(21, 6, 2, 45477, 0),
(21, 6, 3, 45462, 0),
(21, 6, 4, 45902, 0),
(21, 6, 5, 47541, 0),
(21, 6, 10, 2481, 0),
(21, 7, 0, 6603, 0),
(21, 7, 1, 403, 0),
(21, 7, 2, 331, 0),
(21, 7, 3, 20594, 0),
(21, 7, 4, 2481, 0),
(21, 8, 0, 133, 0),
(21, 8, 1, 168, 0),
(21, 8, 11, 20594, 0),
(21, 8, 10, 2481, 0),
(21, 9, 0, 686, 0),
(21, 9, 1, 687, 0),
(21, 9, 11, 20594, 0),
(21, 9, 10, 2481, 0),
(21, 11, 0, 5176, 0),
(21, 11, 1, 5185, 0),
(21, 11, 11, 20594, 0),
(21, 11, 10, 2481, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 1048576 for Dark Iron Dwarf
-- Copying from Dwarf (4) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '1048576', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 4;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 1048576
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Dwarf (3) for Dark Iron Dwarf (21)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 21, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 3;

