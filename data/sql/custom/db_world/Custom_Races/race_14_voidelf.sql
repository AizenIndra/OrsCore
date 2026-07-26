-- Race 14: Void Elf
-- Race Mask: 8192 (1<<13)

-- playercreateinfo (Alliance)
INSERT IGNORE INTO `playercreateinfo` VALUES
(14, 1, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 2, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 3, 0, 0, -8949.95, -132.493, 83.5312, 0),
(14, 4, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 5, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 6, 609, 4298, 2355.84, -5664.77, 426.028, 3.65997),
(14, 7, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 8, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 9, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 11, 0, 12, -8949.95, -132.493, 83.5312, 0),
(14, 12, 0, 12, -8949.95, -132.493, 83.5312, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(14, 1, 0, 6603, 0),
(14, 1, 72, 6603, 0),
(14, 1, 73, 78, 0),
(14, 1, 82, 28730, 0),
(14, 1, 84, 6603, 0),
(14, 1, 96, 6603, 0),
(14, 1, 108, 6603, 0),
(14, 2, 0, 6603, 0),
(14, 2, 1, 21084, 0),
(14, 2, 2, 635, 0),
(14, 2, 3, 28730, 0),
(14, 3, 0, 6603, 0),
(14, 3, 1, 2973, 0),
(14, 3, 2, 75, 0),
(14, 3, 3, 28730, 0),
(14, 4, 0, 6603, 0),
(14, 4, 1, 1752, 0),
(14, 4, 2, 2098, 0),
(14, 4, 3, 2764, 0),
(14, 4, 4, 25046, 0),
(14, 5, 0, 585, 0),
(14, 5, 1, 2050, 0),
(14, 5, 2, 28730, 0),
(14, 6, 0, 6603, 0),
(14, 6, 1, 49576, 0),
(14, 6, 2, 45477, 0),
(14, 6, 3, 45462, 0),
(14, 6, 4, 45902, 0),
(14, 6, 5, 47541, 0),
(14, 6, 6, 50613, 0),
(14, 8, 0, 133, 0),
(14, 8, 1, 168, 0),
(14, 8, 2, 28730, 0),
(14, 9, 0, 686, 0),
(14, 9, 1, 687, 0),
(14, 9, 2, 28730, 0),
(14, 11, 0, 5176, 0),
(14, 11, 1, 5185, 0),
(14, 11, 11, 28730, 0),
(14, 7, 0, 6603, 0),
(14, 7, 1, 403, 0),
(14, 7, 2, 331, 0),
(14, 7, 3, 20554, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 8192 for Void Elf
-- Copying from Blood Elf (512) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '8192', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 512;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 8192
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Blood Elf (10) for Void Elf (14)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 14, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 10;

