DROP TABLE IF EXISTS promotion_codes;
CREATE TABLE `promotion_codes` (
  `id` int unsigned NOT NULL,
  `collection` int unsigned,
  `code` varchar(19) NOT NULL DEFAULT '',
  `honor` int unsigned,
  `arena` int unsigned,
  `money` int unsigned,
  `item_1` int unsigned,
  `item_2` int unsigned,
  `item_3` int unsigned,
  `item_count_1` int unsigned,
  `item_count_2` int unsigned,
  `item_count_3` int unsigned,
  `aura` int unsigned,
  `spell_1` int unsigned,
  `spell_2` int unsigned,
  `spell_3` int unsigned,
  `coin` int unsigned,
  `count_of_exists` int unsigned NOT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `code` (`code`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS promotion_codes_history;
CREATE TABLE `promotion_codes_history` (
  `id` int unsigned NOT NULL,
  `codeID` int unsigned NOT NULL,
  `code` varchar(19) NOT NULL DEFAULT '',
  `accountID` int unsigned NOT NULL,
  `playerID` int unsigned,
  `use_time` varchar(19) NOT NULL DEFAULT '',
  `use_unix_time` int unsigned,
PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Тестовые промокоды
-- Формат: id, collection, code, honor, arena, money (в медных), item_1, item_2, item_3, item_count_1, item_count_2, item_count_3, aura, spell_1, spell_2, spell_3, coin (бонусы), count_of_exists

INSERT INTO `promotion_codes` (`id`, `collection`, `code`, `honor`, `arena`, `money`, `item_1`, `item_2`, `item_3`, `item_count_1`, `item_count_2`, `item_count_3`, `aura`, `spell_1`, `spell_2`, `spell_3`, `coin`, `count_of_exists`) VALUES
-- Промокод 1: Только золото (100 золота)
(1, 0, 'TEST100GOLD', 0, 0, 1000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100),

-- Промокод 2: Золото + бонусы (50 золота + 10 бонусов)
(2, 0, 'TEST50G10B', 0, 0, 500000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 50),

-- Промокод 3: Предметы (Эмблема льда x1)
(3, 0, 'TESTITEM1', 0, 0, 0, 49426, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 75),

-- Промокод 4: Предметы + золото (Эмблема льда x5 + 200 золота)
(4, 0, 'TESTITEM5G', 0, 0, 2000000, 49426, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 30),

-- Промокод 5: Множественные предметы (3 разных предмета)
(5, 0, 'TEST3ITEMS', 0, 0, 0, 49426, 49427, 49428, 1, 2, 3, 0, 0, 0, 0, 0, 25),

-- Промокод 6: Очки чести и арены (1000 чести + 500 арены)
(6, 0, 'TESTHONOR', 1000, 500, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20),

-- Промокод 7: Полный набор (золото + бонусы + предметы + честь)
(7, 0, 'TESTFULL', 500, 250, 500000, 49426, 0, 0, 3, 0, 0, 0, 0, 0, 0, 25, 10),

-- Промокод 8: Только бонусы (50 бонусов)
(8, 0, 'TEST50BONUS', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 200),

-- Промокод 9: Большое золото (1000 золота)
(9, 0, 'TEST1000G', 0, 0, 10000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5),

-- Промокод 10: Золото + предметы + бонусы (комбо)
(10, 0, 'TESTCOMBO', 0, 0, 1000000, 49426, 49427, 0, 2, 1, 0, 0, 0, 0, 0, 15, 15);
