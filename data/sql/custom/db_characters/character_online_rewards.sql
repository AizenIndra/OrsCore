-- Character online rewards system table
-- Stores online time tracking and reward information for each character
CREATE TABLE IF NOT EXISTS `character_online_rewards` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'Character GUID',
  `total_online_time` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Total online time in seconds since last reward',
  `last_reward_time` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unix timestamp of last reward claim',
  `last_login_time` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unix timestamp of last login',
  `total_rewards_claimed` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Total number of rewards claimed',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Online rewards tracking system';