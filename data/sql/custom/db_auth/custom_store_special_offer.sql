DROP TABLE IF EXISTS `custom_store_special_offer`;
CREATE TABLE `custom_store_special_offer` (
  `offerID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `background` varchar(255),
  `headline` varchar(255),
  `title` varchar(255),
  `description` varchar(255),
  `detailsTitle` varchar(255),
  `details` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `productID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `price` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`offerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
