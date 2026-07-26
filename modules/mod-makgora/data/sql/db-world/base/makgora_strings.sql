-- Module string range reserved for Mak'Gora (900100-900199)
SET @BASE := 900100;

DELETE FROM `acore_string` WHERE `entry` BETWEEN @BASE AND (@BASE + 20);
INSERT INTO `acore_string` (`entry`, `content_default`) VALUES
(@BASE + 0, '|cffc41e3a[Mak''Gora]|r System disabled.'),
(@BASE + 1, '|cffc41e3a[Mak''Gora]|r {} of the {} claims the mantle of leadership!'),
(@BASE + 2, '|cffc41e3a[Mak''Gora]|r {} has fallen in ritual combat. The ancestors claim their name.'),
(@BASE + 3, '|cffc41e3a[Mak''Gora]|r You are not eligible for Mak''Gora.'),
(@BASE + 4, '|cffc41e3a[Mak''Gora]|r Only same-faction champions may enter this ritual.'),
(@BASE + 5, '|cffc41e3a[Mak''Gora]|r A Mak''Gora ritual has begun between {} and {}.'),
(@BASE + 6, '|cffc41e3a[Mak''Gora]|r You walk as a permanent ghost. The afterlife awaits.'),
(@BASE + 7, '|cffc41e3a[Mak''Gora]|r Current {} leader: {}.'),
(@BASE + 8, '|cffc41e3a[Mak''Gora]|r No living leader holds this title.'),
(@BASE + 9, '|cffc41e3a[Mak''Gora]|r Eligible flag set for {}.'),
(@BASE + 10, '|cffc41e3a[Mak''Gora]|r Hardcore bind toggled: {}.'),
(@BASE + 11, '|cffc41e3a[Mak''Gora]|r Defeat the elite champion before you may challenge for leadership.'),
(@BASE + 12, '|cffc41e3a[Mak''Gora]|r Ritual interrupted.');
