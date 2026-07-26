/*
This query allows you to copy information from a specific breed to add new information.
In this example, the code copies the information from RACE 1 (Human) to add it to race 12 (Fel'Orc).

Note: player_levelstats table no longer exists (replaced by player_class_stats which is class-based, not race-based).
Note: playercreateinfo_spell table does not exist, only playercreateinfo_spell_custom is used.

*/

-- PlayerCreateInfo
REPLACE INTO `playercreateinfo`
	SELECT race+11, class, map, zone, position_x, position_y, position_z, orientation FROM `playercreateinfo` WHERE race = 1;

-- PlayerLevelStats (REMOVED: table no longer exists - replaced by player_class_stats which is class-based, not race-based)

-- PlayerCreateInfo_Action
REPLACE INTO `playercreateinfo_action`
	SELECT race+11, class, button, `action`, `type` FROM `playercreateinfo_action` WHERE race = 1;

-- PlayerCreateInfo_Item
REPLACE INTO `playercreateinfo_item`
	SELECT race+11, class, itemid, amount, Note FROM `playercreateinfo_item` WHERE race = 1;

-- PlayerCreateInfo_Spell (REMOVED: table does not exist, only playercreateinfo_spell_custom is used)

-- PlayerCreateInfo_Spell_Custom
REPLACE INTO `playercreateinfo_spell_custom`
	SELECT racemask+2047, classmask, Spell, Note FROM `playercreateinfo_spell_custom` WHERE racemask = 1;
