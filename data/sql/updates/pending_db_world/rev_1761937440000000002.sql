--
-- Guild leveling system: world DB schema and baseline data.
--

CREATE TABLE IF NOT EXISTS `guild_xp_for_next_level` (
  `level` tinyint unsigned NOT NULL,
  `xp_for_next_level` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `guild_bonus_spell` (
  `level` tinyint unsigned NOT NULL,
  `spell` int unsigned NOT NULL,
  PRIMARY KEY (`level`, `spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `creature_onkill_reward_guildxp` (
  `creatureID` int unsigned NOT NULL,
  `rewardXP` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`creatureID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_xp_for_next_level`;
INSERT INTO `guild_xp_for_next_level` (`level`, `xp_for_next_level`) VALUES
(1, 10000),
(2, 15000),
(3, 20000),
(4, 25000),
(5, 30000),
(6, 35000),
(7, 40000),
(8, 45000),
(9, 50000),
(10, 55000),
(11, 60000),
(12, 65000),
(13, 70000),
(14, 75000),
(15, 80000),
(16, 85000),
(17, 90000),
(18, 95000),
(19, 100000),
(20, 105000),
(21, 110000),
(22, 115000),
(23, 120000),
(24, 125000);

-- Optional perk data can be inserted into `guild_bonus_spell`.
-- Optional NPC reward data can be inserted into `creature_onkill_reward_guildxp`.

DELETE FROM `acore_string` WHERE `entry` IN (12750, 12751);
INSERT INTO `acore_string` (`entry`, `content_default`, `locale_ruRU`) VALUES
(12750, 'Guild |cffffffff[{}]|r has reached level {}!', 'Гильдия |cffffffff[{}]|r достигла {} уровня!'),
(12751, 'Daily guild experience caps have been reset.', 'Дневные лимиты опыта гильдий сброшены.');
