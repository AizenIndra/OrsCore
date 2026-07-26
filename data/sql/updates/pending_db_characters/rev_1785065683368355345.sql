-- item_transmogrification from OrstetCore 67972dad

CREATE TABLE IF NOT EXISTS `item_transmogrification` (
    `item` INT UNSIGNED NOT NULL,
    `transEntry` MEDIUMINT UNSIGNED NOT NULL,
    PRIMARY KEY (`item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
