-- Rank System: persistent rank points on characters

SET @db := DATABASE();

SET @has_rank_points := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'characters'
      AND COLUMN_NAME = 'rankPoints'
);
SET @sql := IF(@has_rank_points = 0,
    'ALTER TABLE `characters` ADD COLUMN `rankPoints` int unsigned NOT NULL DEFAULT ''0'' AFTER `innTriggerId`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
