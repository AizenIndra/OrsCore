DROP TABLE IF EXISTS `custom_store_mounts`;
CREATE TABLE `custom_store_mounts` (
  `id` mediumint(8) NOT NULL DEFAULT '0',
  `hash` varchar(255) NOT NULL DEFAULT '0',
  `currency` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `price` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `productID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `hash` (`hash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
