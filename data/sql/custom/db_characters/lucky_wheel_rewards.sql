-- Stores rewards that can be won from the online rewards lucky wheel
CREATE TABLE IF NOT EXISTS `lucky_wheel_rewards` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Reward ID',
  `reward_type` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Gold, 1=Item, 2=Currency',
  `reward_value` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Item ID for items, Currency ID for currency, 0 for gold',
  `reward_count` INT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Amount of reward (gold in copper, item count, currency amount)',
  `chance` FLOAT NOT NULL DEFAULT 0.0 COMMENT 'Drop chance percentage (0-100)',
  `name` VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Display name',
  `icon` VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'INV_Misc_QuestionMark' COMMENT 'Icon name',
  `color` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Rarity color (1=gray, 2=green, 3=blue, 4=purple, 5=orange)',
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Is reward enabled (0=disabled, 1=enabled)',
  `order` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Display order',
  PRIMARY KEY (`id`),
  INDEX `idx_enabled` (`enabled`),
  INDEX `idx_order` (`order`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Lucky Wheel rewards pool';

-- Insert default rewards
INSERT INTO `lucky_wheel_rewards` (`id`, `reward_type`, `reward_value`, `reward_count`, `chance`, `name`, `icon`, `color`, `enabled`, `order`) VALUES
(1, 0, 0, 1000000, 30.0, '100 золота', 'INV_Misc_Coin_01', 1, 1, 1),
(2, 0, 0, 5000000, 20.0, '500 золота', 'INV_Misc_Coin_02', 2, 1, 2),
(3, 0, 0, 10000000, 10.0, '1000 золота', 'INV_Misc_Coin_03', 3, 1, 3),
(4, 1, 49426, 1, 15.0, 'Эмблема льда', 'INV_Jewelry_Necklace_35', 3, 1, 4),
(5, 2, 1, 10, 15.0, '10 Бонусов', 'WoW_Token01', 2, 1, 5),
(6, 2, 1, 50, 5.0, '50 Бонусов', 'WoW_Token01', 4, 1, 6),
(7, 1, 49426, 5, 5.0, '5x Эмблема льда', 'INV_Jewelry_Necklace_35', 4, 1, 7)
ON DUPLICATE KEY UPDATE `reward_type`=VALUES(`reward_type`), `reward_value`=VALUES(`reward_value`), `reward_count`=VALUES(`reward_count`), `chance`=VALUES(`chance`), `name`=VALUES(`name`), `icon`=VALUES(`icon`), `color`=VALUES(`color`), `enabled`=VALUES(`enabled`), `order`=VALUES(`order`);