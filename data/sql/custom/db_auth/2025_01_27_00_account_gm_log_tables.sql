-- GM Logging tables for auth database

DROP TABLE IF EXISTS `account_gm_log`;
CREATE TABLE `account_gm_log` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `player` VARCHAR(12) NOT NULL DEFAULT '',
  `account` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `command` TEXT NOT NULL,
  `position` TEXT NOT NULL,
  `selected` TEXT NOT NULL,
  `realmId` INT(10) NOT NULL DEFAULT '0',
  `timestamp` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_account` (`account`),
  KEY `idx_realmId` (`realmId`),
  KEY `idx_timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `account_gm_log_item`;
CREATE TABLE `account_gm_log_item` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `guid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `player` VARCHAR(12) NOT NULL DEFAULT '',
  `account` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `item` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `item_guid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `count` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `position` TEXT NOT NULL,
  `target` TEXT NOT NULL,
  `realmId` INT(10) NOT NULL DEFAULT '0',
  `timestamp` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_account` (`account`),
  KEY `idx_item` (`item`),
  KEY `idx_realmId` (`realmId`),
  KEY `idx_timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
