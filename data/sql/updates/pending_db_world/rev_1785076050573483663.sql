-- Rank System: allow .reload rank_system_levels

DELETE FROM `command` WHERE `name` = 'reload rank_system_levels';
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('reload rank_system_levels', 3, 'Syntax: .reload rank_system_levels\nReload rank thresholds from rank_system_levels.');
