-- Hardcore Insurance Items
-- Donator Insurance Scroll and Craftable Insurance Scroll

-- Donator Insurance Scroll (Item ID: 56813)
DELETE FROM `item_template` WHERE `entry` = 56813;

INSERT INTO `item_template` (
    `entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, 
    `Quality`, `Flags`, `FlagsExtra`, `BuyCount`, `BuyPrice`, `SellPrice`, 
    `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, 
    `maxcount`, `stackable`, 
    `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, 
    `bonding`, `description`, 
    `ScriptName`, `VerifiedBuild`
) VALUES (
    56813,                -- entry
    0,                     -- class (Consumable)
    0,                     -- subclass
    -1,                    -- SoundOverrideSubclass
    'Свиток страхования (Донат)',  -- name
    1301,                  -- displayid
    4,                     -- Quality (Epic)
    0,                     -- Flags
    0,                     -- FlagsExtra
    1,                     -- BuyCount
    0,                     -- BuyPrice
    0,                     -- SellPrice
    0,                     -- InventoryType (Non-equippable)
    -1,                    -- AllowableClass (All classes)
    -1,                    -- AllowableRace (All races)
    1,                     -- ItemLevel
    1,                     -- RequiredLevel
    0,                     -- maxcount
    1,                     -- stackable
    1,                     -- spellid_1 (Dummy spell to trigger OnUse)
    0,                     -- spelltrigger_1 (On Use)
    0,                     -- spellcharges_1
    -1,                    -- spellcooldown_1
    1,                     -- bonding (Bind on Pickup)
    'Используйте этот свиток на предмете, чтобы застраховать его от потери при смерти на уровне 80+. Застрахованные предметы не будут потеряны при смерти.',  -- description
    'item_hardcore_insurance',  -- ScriptName
    12340                  -- VerifiedBuild
);

-- Craftable Insurance Scroll (Item ID: 56812)
DELETE FROM `item_template` WHERE `entry` = 56812;

INSERT INTO `item_template` (
    `entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, 
    `Quality`, `Flags`, `FlagsExtra`, `BuyCount`, `BuyPrice`, `SellPrice`, 
    `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, 
    `maxcount`, `stackable`, 
    `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, 
    `bonding`, `description`, 
    `ScriptName`, `VerifiedBuild`
) VALUES (
    56812,                -- entry
    0,                     -- class (Consumable)
    0,                     -- subclass
    -1,                    -- SoundOverrideSubclass
    'Свиток страхования (Крафт)',  -- name
    1301,                  -- displayid
    3,                     -- Quality (Rare)
    0,                     -- Flags
    0,                     -- FlagsExtra
    1,                     -- BuyCount
    0,                     -- BuyPrice
    0,                     -- SellPrice
    0,                     -- InventoryType (Non-equippable)
    -1,                    -- AllowableClass (All classes)
    -1,                    -- AllowableRace (All races)
    1,                     -- ItemLevel
    1,                     -- RequiredLevel
    0,                     -- maxcount
    1,                     -- stackable
    1,                     -- spellid_1 (Dummy spell to trigger OnUse)
    0,                     -- spelltrigger_1 (On Use)
    0,                     -- spellcharges_1
    -1,                    -- spellcooldown_1
    1,                     -- bonding (Bind on Pickup)
    'Используйте этот свиток на предмете, чтобы застраховать его от потери при смерти на уровне 80+. Застрахованные предметы не будут потеряны при смерти.',  -- description
    'item_hardcore_insurance',  -- ScriptName
    12340                  -- VerifiedBuild
);