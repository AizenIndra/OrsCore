-- Custom_Feedback: одноразовый отзыв об игре на аккаунт (окно «Обратная связь»).
-- Одна строка на аккаунт: повторная отправка отклоняется (опкод 67).

CREATE TABLE IF NOT EXISTS `account_feedback` (
  `account_id` INT UNSIGNED NOT NULL COMMENT 'Account ID',
  `type` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=general, 1=bug, 2=balance',
  `rating` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Rating 1-5 (general only)',
  `class_name` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Class name (balance only)',
  `theme` VARCHAR(96) NOT NULL DEFAULT '' COMMENT 'Bug report subject',
  `category` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Bug category',
  `priority` VARCHAR(16) NOT NULL DEFAULT '' COMMENT 'Bug priority: low/medium/high',
  `message` TEXT NOT NULL COMMENT 'Feedback text',
  `character_guid` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Character GUID (low part)',
  `character_name` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Character name',
  `submit_time` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unix timestamp',
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='One-time account feedback';

-- Журнал всех обработанных попыток отправки (включая отклонённые).
CREATE TABLE IF NOT EXISTS `account_feedback_log` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Account ID',
  `type` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=general, 1=bug, 2=balance',
  `rating` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Rating 1-5 (general only)',
  `class_name` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Class name (balance only)',
  `theme` VARCHAR(96) NOT NULL DEFAULT '' COMMENT 'Bug report subject',
  `category` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Bug category',
  `priority` VARCHAR(16) NOT NULL DEFAULT '' COMMENT 'Bug priority: low/medium/high',
  `message` TEXT NOT NULL COMMENT 'Feedback text',
  `character_guid` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Character GUID (low part)',
  `character_name` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Character name',
  `submit_time` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unix timestamp',
  `result` VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'accepted / already / validation reason',
  PRIMARY KEY (`id`),
  KEY `idx_account` (`account_id`),
  KEY `idx_time` (`submit_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Feedback attempts log';
