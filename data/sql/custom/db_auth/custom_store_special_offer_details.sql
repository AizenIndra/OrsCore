DROP TABLE IF EXISTS `custom_store_special_offer_details`;
CREATE TABLE `custom_store_special_offer_details` (
  `detailsID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `role` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `count` mediumint(8) unsigned NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
