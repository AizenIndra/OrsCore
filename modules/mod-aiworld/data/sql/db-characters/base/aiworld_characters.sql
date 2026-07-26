CREATE TABLE IF NOT EXISTS `aiworld_territory` (
    `zone_id` int unsigned NOT NULL,
    `state` tinyint unsigned NOT NULL DEFAULT 0 COMMENT '0 Contested, 1 AllianceHeld, 2 HordeHeld',
    `controller` tinyint unsigned NOT NULL DEFAULT 0 COMMENT '0 Neutral, 1 Alliance, 2 Horde',
    `control_score` int NOT NULL DEFAULT 0 COMMENT '-1000..1000',
    `tension` int unsigned NOT NULL DEFAULT 0 COMMENT '0..100',
    `updated_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World territory control';

INSERT IGNORE INTO `aiworld_territory` (`zone_id`, `state`, `controller`, `control_score`, `tension`, `updated_at`) VALUES
(267, 0, 0, 0, 0, 0);

CREATE TABLE IF NOT EXISTS `aiworld_event_history` (
    `id` bigint unsigned NOT NULL AUTO_INCREMENT,
    `zone_id` int unsigned NOT NULL,
    `event_type` varchar(64) NOT NULL DEFAULT '',
    `template_id` int unsigned NOT NULL DEFAULT 0,
    `aggressor` tinyint unsigned NOT NULL DEFAULT 0,
    `winner` tinyint unsigned NOT NULL DEFAULT 0,
    `payload_json` text,
    `created_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_zone_time` (`zone_id`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World event history';

CREATE TABLE IF NOT EXISTS `aiworld_faction_score` (
    `side` tinyint unsigned NOT NULL COMMENT '1 Alliance, 2 Horde',
    `window_hour` int NOT NULL DEFAULT 0,
    `window_day` int NOT NULL DEFAULT 0,
    `total` bigint NOT NULL DEFAULT 0,
    `updated_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`side`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World faction score windows';

INSERT IGNORE INTO `aiworld_faction_score` (`side`, `window_hour`, `window_day`, `total`, `updated_at`) VALUES
(1, 0, 0, 0, 0),
(2, 0, 0, 0, 0);

CREATE TABLE IF NOT EXISTS `aiworld_player_action` (
    `id` bigint unsigned NOT NULL AUTO_INCREMENT,
    `guid` int unsigned NOT NULL,
    `zone_id` int unsigned NOT NULL,
    `action_type` varchar(32) NOT NULL DEFAULT '',
    `side` tinyint unsigned NOT NULL DEFAULT 0,
    `created_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_zone_time` (`zone_id`, `created_at`),
    KEY `idx_guid_time` (`guid`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World player actions';

CREATE TABLE IF NOT EXISTS `aiworld_npc_change` (
    `id` bigint unsigned NOT NULL AUTO_INCREMENT,
    `zone_id` int NOT NULL,
    `entry` int NOT NULL DEFAULT 0,
    `change_type` varchar(32) NOT NULL DEFAULT '',
    `details_json` text,
    `created_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_zone_time` (`zone_id`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World NPC changes';

CREATE TABLE IF NOT EXISTS `aiworld_llm_cache` (
    `cache_key` char(64) NOT NULL,
    `response_json` mediumtext NOT NULL,
    `created_at` int unsigned NOT NULL DEFAULT 0,
    `expires_at` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`cache_key`),
    KEY `idx_expires` (`expires_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI World LLM response cache';
