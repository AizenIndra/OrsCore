/*
 Navicat Premium Dump SQL

 Source Server         : WoW Server
 Source Server Type    : MySQL
 Source Server Version : 80411 (8.4.11)
 Source Host           : localhost:3306
 Source Schema         : acore_auth

 Target Server Type    : MySQL
 Target Server Version : 80411 (8.4.11)
 File Encoding         : 65001

 Date: 30/08/2026 14:18:19
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for custom_store_item_data
-- ----------------------------
DROP TABLE IF EXISTS `custom_store_item_data`;
CREATE TABLE `custom_store_item_data`  (
  `productID` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `itemEntry` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `count` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `price` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `discount` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `discountPrice` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `creatureEntry` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `storeFlags` int UNSIGNED NOT NULL DEFAULT 0,
  `CategoryID` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `SubCategoryID` mediumint UNSIGNED NOT NULL DEFAULT 0,
  `MoneyID` mediumint NULL DEFAULT 0,
  PRIMARY KEY (`productID`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;
