-- Rank System: acore_string, thresholds, glory-exp item scripts

DELETE FROM `acore_string` WHERE `entry` = 12752;
INSERT INTO `acore_string`
    (`entry`, `content_default`,
     `locale_koKR`, `locale_frFR`, `locale_deDE`,
     `locale_zhCN`, `locale_zhTW`,
     `locale_esES`, `locale_esMX`, `locale_ruRU`)
VALUES
    (12752,
     'Rank points: {}',
     NULL, NULL, NULL,
     NULL, NULL,
     NULL, NULL, 'Очки ранга: {}');

CREATE TABLE IF NOT EXISTS `rank_system_levels` (
  `rank` TINYINT UNSIGNED NOT NULL,
  `required_points` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `rank_system_levels`;
INSERT INTO `rank_system_levels` (`rank`, `required_points`) VALUES
(1, 250),
(2, 500),
(3, 1000),
(4, 2000),
(5, 4000),
(6, 8000),
(7, 16000),
(8, 32000),
(9, 60000),
(10, 80000),
(11, 100000),
(12, 125000),
(13, 150000),
(14, 175000),
(15, 200000),
(16, 225000),
(17, 250000),
(18, 275000),
(19, 300000),
(20, 350000),
(21, 400000),
(22, 450000),
(23, 500000),
(24, 550000),
(25, 600000),
(26, 650000),
(27, 700000),
(28, 750000),
(29, 800000),
(30, 850000),
(31, 900000),
(32, 950000),
(33, 1000000),
(34, 1100000),
(35, 1200000),
(36, 1300000),
(37, 1400000),
(38, 1500000),
(39, 1600000),
(40, 1700000),
(41, 1800000),
(42, 1900000),
(43, 2000000),
(44, 2100000),
(45, 2200000),
(46, 2300000),
(47, 2400000),
(48, 2500000),
(49, 2600000),
(50, 3000000);

UPDATE `item_template`
SET `ScriptName` = 'ItemUse_Glory_Exp'
WHERE `entry` IN (1042, 1043, 1044, 35778, 842);
