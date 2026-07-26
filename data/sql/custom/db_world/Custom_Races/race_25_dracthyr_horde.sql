-- Race 25: Dracthyr (Horde)
-- Race Mask: 16777216 (1<<24)

-- playercreateinfo (horde)
INSERT IGNORE INTO `playercreateinfo` VALUES
(25, 1, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 2, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 3, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 4, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 5, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 6, 609, 4298, 2358.44, -5666.9, 426.023, 3.65997),
(25, 7, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 8, 1, 14, -618.518, -4215.67, 38.718, 0),
(25, 9, 1, 14, -618.518, -4251.67, 38.718, 0),
(25, 11, 1, 14, -618.518, -4215.67, 38.718, 0),
(25, 12, 1, 14, -618.518, -4251.67, 38.718, 0);

-- playercreateinfo_action
INSERT IGNORE INTO `playercreateinfo_action` VALUES
(25, 1, 0, 6603, 0),
(25, 1, 72, 6603, 0),
(25, 1, 73, 78, 0),
(25, 1, 82, 28730, 0),
(25, 1, 84, 6603, 0),
(25, 1, 96, 6603, 0),
(25, 1, 108, 6603, 0),
(25, 2, 0, 6603, 0),
(25, 2, 1, 21084, 0),
(25, 2, 2, 635, 0),
(25, 2, 3, 28730, 0),
(25, 3, 0, 6603, 0),
(25, 3, 1, 2973, 0),
(25, 3, 2, 75, 0),
(25, 3, 3, 28730, 0),
(25, 4, 0, 6603, 0),
(25, 4, 1, 1752, 0),
(25, 4, 2, 2098, 0),
(25, 4, 3, 2764, 0),
(25, 4, 4, 25046, 0),
(25, 5, 0, 585, 0),
(25, 5, 1, 2050, 0),
(25, 5, 2, 28730, 0),
(25, 6, 0, 6603, 0),
(25, 6, 1, 49576, 0),
(25, 6, 2, 45477, 0),
(25, 6, 3, 45462, 0),
(25, 6, 4, 45902, 0),
(25, 6, 5, 47541, 0),
(25, 6, 6, 50613, 0),
(25, 8, 0, 133, 0),
(25, 8, 1, 168, 0),
(25, 8, 2, 28730, 0),
(25, 9, 0, 686, 0),
(25, 9, 1, 687, 0),
(25, 9, 2, 28730, 0),
(25, 11, 0, 5176, 0),
(25, 11, 1, 5185, 0),
(25, 11, 11, 28730, 0),
(25, 7, 0, 6603, 0),
(25, 7, 1, 403, 0),
(25, 7, 2, 331, 0),
(25, 7, 3, 20554, 0);

-- playercreateinfo_spell_custom
-- Note: Race mask 16777216 for Dracthyr Horde
-- Copying from Blood Elf (512) and adjusting race-specific spells
INSERT IGNORE INTO `playercreateinfo_spell_custom` 
SELECT '16777216', classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 512;


-- playercreateinfo_skills
UPDATE `playercreateinfo_skills`
	SET `raceMask` = 0
		WHERE `skill` IN (45, 46, 160, 173, 226) AND `classMask` != 0;

-- quest_template
UPDATE `quest_template` INNER JOIN `quest_template_addon` 
	ON `quest_template_addon`.id = `quest_template`.id
		SET AllowableRaces = AllowableRaces | 16777216
			WHERE `quest_template_addon`.allowableclasses != 0 AND AllowableRaces != 0;

-- playercreateinfo_item
-- Copying starting items from Blood Elf (10) for Dracthyr Horde (25)
INSERT IGNORE INTO `playercreateinfo_item` 
SELECT 25, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 10;

