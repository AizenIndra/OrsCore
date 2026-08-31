DROP TABLE IF EXISTS `custom_store_shop_version`;
CREATE TABLE `custom_store_shop_version` (
  `version` mediumint(8) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `custom_store_shop_version`;
INSERT INTO `custom_store_shop_version` (`version`) VALUES (1);