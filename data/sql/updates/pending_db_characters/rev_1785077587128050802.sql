-- Hardcore: opt-in flag on characters

SET @db := DATABASE();

SET @has_hardcore := (
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = @db
      AND TABLE_NAME = 'characters'
      AND COLUMN_NAME = 'hardcore'
);
SET @sql := IF(@has_hardcore = 0,
    'ALTER TABLE `characters` ADD COLUMN `hardcore` TINYINT UNSIGNED NOT NULL DEFAULT ''0'' COMMENT ''1 = hardcore'' AFTER `extraBonusTalentCount`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
