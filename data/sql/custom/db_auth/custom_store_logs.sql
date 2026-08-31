DROP TABLE IF EXISTS `custom_store_logs`;
CREATE TABLE `custom_store_logs` (
  `character_ID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `character_name` varchar(255),
  `account_ID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `serviceName` varchar(255),
  `itemID` int(10) unsigned NOT NULL DEFAULT '0',
  `itemCount` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `totalPrice` int(10) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
