--
-- Ensure all paid services exist in custom store and are visible in "Services".
-- CategoryID = 5 (Services), SubCategoryID = 0.
-- Premium one day (1000019) is configured for MMOTOP currency (MoneyID = 2).
--

INSERT INTO `custom_store_item_data`
    (`productID`, `itemEntry`, `count`, `price`, `discount`, `discountPrice`, `creatureEntry`, `storeFlags`, `CategoryID`, `SubCategoryID`, `MoneyID`)
SELECT
    s.`productID`, s.`itemEntry`, s.`count`, s.`price`, s.`discount`, s.`discountPrice`, s.`creatureEntry`, s.`storeFlags`, s.`CategoryID`, s.`SubCategoryID`, s.`MoneyID`
FROM (
    SELECT 1000000 AS `productID`, 1000000 AS `itemEntry`, 1 AS `count`,  99 AS `price`, 0 AS `discount`,  99 AS `discountPrice`, 0 AS `creatureEntry`, 0 AS `storeFlags`, 5 AS `CategoryID`, 0 AS `SubCategoryID`, 1 AS `MoneyID`
    UNION ALL SELECT 1000001, 1000001, 1, 299, 0, 299, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000002, 1000002, 1, 199, 0, 199, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000003, 1000003, 1, 149, 0, 149, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000004, 1000004, 1,  99, 0,  99, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000005, 1000005, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000006, 1000006, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000007, 1000007, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000008, 1000008, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000009, 1000009, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000010, 1000010, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000011, 1000011, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000012, 1000012, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000013, 1000013, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000014, 1000014, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000015, 1000015, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000016, 1000016, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000017, 1000017, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000018, 1000018, 1,  49, 0,  49, 0, 0, 5, 0, 1
    UNION ALL SELECT 1000019, 1000019, 1,  39, 0,  39, 0, 0, 5, 0, 2
    UNION ALL SELECT 1000020, 1000020, 1, 299, 0, 299, 0, 0, 5, 0, 1
) s
LEFT JOIN `custom_store_item_data` c
    ON c.`itemEntry` = s.`itemEntry`
WHERE c.`itemEntry` IS NULL;

UPDATE `custom_store_item_data`
SET
    `count` = 1,
    `creatureEntry` = 0,
    `storeFlags` = 0,
    `CategoryID` = 5,
    `SubCategoryID` = 0,
    `MoneyID` = 1,
    `discountPrice` = IF(`discount` = 0, `price`, `discountPrice`)
WHERE `itemEntry` IN (
    1000000, 1000001, 1000002, 1000003, 1000004,
    1000005, 1000006, 1000007, 1000008, 1000009,
    1000010, 1000011, 1000012, 1000013, 1000014,
    1000015, 1000016, 1000017, 1000018, 1000020
);

UPDATE `custom_store_item_data`
SET
    `count` = 1,
    `creatureEntry` = 0,
    `storeFlags` = 0,
    `CategoryID` = 5,
    `SubCategoryID` = 0,
    `MoneyID` = 2,
    `discountPrice` = IF(`discount` = 0, `price`, `discountPrice`)
WHERE `itemEntry` = 1000019;

INSERT INTO `custom_store_shop_version` (`version`)
SELECT 1
WHERE NOT EXISTS (SELECT 1 FROM `custom_store_shop_version`);

UPDATE `custom_store_shop_version`
SET `version` = `version` + 1;
