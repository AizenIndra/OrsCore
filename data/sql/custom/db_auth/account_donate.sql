-- Shop + Transmog auth schema/catalog from OrstetCore 67972dad
-- account_donate: CREATE IF NOT EXISTS (preserve mod-premium balances)

CREATE TABLE IF NOT EXISTS `account_donate` (
  `id` int UNSIGNED NOT NULL,
  `bonuses` int UNSIGNED NOT NULL DEFAULT 0,
  `votes` int UNSIGNED NOT NULL DEFAULT 0,
  `total_bonuses` int UNSIGNED NOT NULL DEFAULT 0,
  `total_votes` int UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
