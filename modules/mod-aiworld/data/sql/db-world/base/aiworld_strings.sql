-- Module string range reserved for AiWorld (900200-900299)
SET @BASE := 900200;

DELETE FROM `acore_string` WHERE `entry` BETWEEN @BASE AND (@BASE + 10);
INSERT INTO `acore_string` (`entry`, `content_default`) VALUES
(@BASE + 0, '|cff4ecdc4[AiWorld]|r System disabled.'),
(@BASE + 1, '|cff4ecdc4[AiWorld]|r {} has seized control of zone {}!'),
(@BASE + 2, '|cff4ecdc4[AiWorld]|r A skirmish erupts as {} presses the attack in zone {}!'),
(@BASE + 3, '|cff4ecdc4[AiWorld]|r The fighting in zone {} dies down.'),
(@BASE + 4, '|cff4ecdc4[AiWorld]|r {} reinforcements (x{}) deploy toward zone {}.'),
(@BASE + 5, '|cff4ecdc4[AiWorld]|r Status:');
