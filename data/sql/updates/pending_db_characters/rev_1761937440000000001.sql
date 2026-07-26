--
-- Guild leveling system: character DB schema.
-- Adds persistent fields used by GuildMgr/Guild and player ilvl snapshots.
--

SET @db := DATABASE();

SET @has_guild_xp := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'guild'
      AND COLUMN_NAME = 'xp'
);
SET @sql := IF(@has_guild_xp = 0,
    'ALTER TABLE `guild` ADD COLUMN `xp` int unsigned NOT NULL DEFAULT ''0'' AFTER `BankMoney`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_guild_level := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'guild'
      AND COLUMN_NAME = 'level'
);
SET @sql := IF(@has_guild_level = 0,
    'ALTER TABLE `guild` ADD COLUMN `level` tinyint unsigned NOT NULL DEFAULT ''1'' AFTER `xp`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_guild_todayxp := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'guild'
      AND COLUMN_NAME = 'todayXP'
);
SET @sql := IF(@has_guild_todayxp = 0,
    'ALTER TABLE `guild` ADD COLUMN `todayXP` int unsigned NOT NULL DEFAULT ''0'' AFTER `level`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_member_ilvl := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'guild_member'
      AND COLUMN_NAME = 'ItemLvl'
);
SET @sql := IF(@has_member_ilvl = 0,
    'ALTER TABLE `guild_member` ADD COLUMN `ItemLvl` int unsigned NOT NULL DEFAULT ''0'' AFTER `offnote`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `guild`
SET `level` = 1
WHERE `level` = 0;
