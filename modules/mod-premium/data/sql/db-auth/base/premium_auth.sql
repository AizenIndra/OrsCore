CREATE TABLE IF NOT EXISTS `account_premium` (
    `id` INT UNSIGNED NOT NULL COMMENT 'Account id',
    `StartTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP(),
    `EndTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP(),
    `active` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '1 = active, 0 = inactive',
    `chat_text_color` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '0 off, 1-10 VIP chat colors',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Premium account status';

-- Existing installs may already have account_premium without chat_text_color
-- (CREATE TABLE IF NOT EXISTS does not alter old schemas).
SET @premium_has_color := (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'account_premium'
      AND COLUMN_NAME = 'chat_text_color'
);
SET @premium_add_color := IF(
    @premium_has_color = 0,
    'ALTER TABLE `account_premium` ADD COLUMN `chat_text_color` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT ''0 off, 1-10 VIP chat colors'' AFTER `active`',
    'SELECT ''account_premium.chat_text_color already exists'' AS info'
);
PREPARE premium_stmt FROM @premium_add_color;
EXECUTE premium_stmt;
DEALLOCATE PREPARE premium_stmt;

CREATE TABLE IF NOT EXISTS `account_premium_free_day` (
    `id` INT UNSIGNED NOT NULL COMMENT 'Account id',
    `claimed_at` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unix time when free VIP was granted',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='One-time free VIP per account';

-- Account bonus currency for .vip buy (OrstetCore-compatible account_donate)
CREATE TABLE IF NOT EXISTS `account_donate` (
    `id` INT UNSIGNED NOT NULL COMMENT 'Account id',
    `bonuses` INT UNSIGNED NOT NULL DEFAULT 0,
    `votes` INT UNSIGNED NOT NULL DEFAULT 0,
    `total_bonuses` INT UNSIGNED NOT NULL DEFAULT 0,
    `total_votes` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Account donate/bonus balance';
