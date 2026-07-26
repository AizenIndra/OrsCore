-- Race 22: Demon Hunter (Horde)
-- Race Mask: 2097152 (1<<21)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(22, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(22, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(22, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(22, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(22, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(22, 1, 0, 6603, 0),
(22, 1, 72, 6603, 0),
(22, 1, 73, 78, 0),
(22, 1, 82, 28730, 0),
(22, 1, 84, 6603, 0),
(22, 1, 96, 6603, 0),
(22, 1, 108, 6603, 0),
(22, 2, 0, 6603, 0),
(22, 2, 1, 21084, 0),
(22, 2, 2, 635, 0),
(22, 2, 3, 28730, 0),
(22, 3, 0, 6603, 0),
(22, 3, 1, 2973, 0),
(22, 3, 2, 75, 0),
(22, 3, 3, 28730, 0),
(22, 4, 0, 6603, 0),
(22, 4, 1, 1752, 0),
(22, 4, 2, 2098, 0),
(22, 4, 3, 2764, 0),
(22, 4, 4, 25046, 0),
(22, 5, 0, 585, 0),
(22, 5, 1, 2050, 0),
(22, 5, 2, 28730, 0),
(22, 6, 0, 6603, 0),
(22, 6, 1, 49576, 0),
(22, 6, 2, 45477, 0),
(22, 6, 3, 45462, 0),
(22, 6, 4, 45902, 0),
(22, 6, 5, 47541, 0),
(22, 6, 6, 50613, 0),
(22, 8, 0, 133, 0),
(22, 8, 1, 168, 0),
(22, 8, 2, 28730, 0),
(22, 9, 0, 686, 0),
(22, 9, 1, 687, 0),
(22, 9, 2, 28730, 0),
(22, 11, 0, 5176, 0),
(22, 11, 1, 5185, 0),
(22, 11, 11, 28730, 0),
(22, 7, 0, 6603, 0),
(22, 7, 1, 403, 0),
(22, 7, 2, 331, 0),
(22, 7, 3, 20554, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 2097152 for Demon Hunter Horde
-- Copying from Blood Elf (512) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '2097152', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 512;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 2097152
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Blood Elf (10) for Demon Hunter Horde (22)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 22, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 10;

