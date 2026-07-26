CREATE TABLE IF NOT EXISTS `makgora_leader` (
    `team` tinyint unsigned NOT NULL COMMENT '0 Alliance, 1 Horde',
    `guid` int unsigned NOT NULL DEFAULT 0,
    `name` varchar(12) NOT NULL DEFAULT '',
    `claimed_at` int unsigned NOT NULL DEFAULT 0,
    `reign_count` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`team`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Active Mak''Gora faction leaders';

INSERT IGNORE INTO `makgora_leader` (`team`, `guid`, `name`, `claimed_at`, `reign_count`) VALUES
(0, 0, '', 0, 0),
(1, 0, '', 0, 0);

CREATE TABLE IF NOT EXISTS `makgora_player` (
    `guid` int unsigned NOT NULL,
    `eligible` tinyint unsigned NOT NULL DEFAULT 0 COMMENT 'Quest path completed',
    `champion_defeated` tinyint unsigned NOT NULL DEFAULT 0,
    `hardcore_bound` tinyint unsigned NOT NULL DEFAULT 0 COMMENT 'Opted into permadeath for ritual',
    `permadeath` tinyint unsigned NOT NULL DEFAULT 0 COMMENT 'Character is permanently dead',
    `cosmetics` int unsigned NOT NULL DEFAULT 0 COMMENT 'Bitfield for cosmetic progression',
    `updated_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Per-character Mak''Gora state';

CREATE TABLE IF NOT EXISTS `makgora_history` (
    `id` int unsigned NOT NULL AUTO_INCREMENT,
    `team` tinyint unsigned NOT NULL,
    `winner_guid` int unsigned NOT NULL,
    `loser_guid` int unsigned NOT NULL,
    `winner_name` varchar(12) NOT NULL DEFAULT '',
    `loser_name` varchar(12) NOT NULL DEFAULT '',
    `outcome` tinyint unsigned NOT NULL COMMENT '1 title transfer, 2 permadeath, 3 fled, 4 interrupted',
    `happened_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_team_time` (`team`, `happened_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Mak''Gora ritual history';
