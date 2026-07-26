-- Premium/VIP acore_string entries (from OrstetCore)
DELETE FROM `acore_string` WHERE `entry` IN (
    12001, 12170, 12171, 12172, 12173, 12174, 12175, 12176,
    12177, 12178, 12179, 12180, 12181, 12182, 12183, 12184
);
INSERT INTO `acore_string` (`entry`, `content_default`, `locale_ruRU`) VALUES
(12001, 'VIP mode', 'Премиум режим'),
(12170, 'You are not a premium account.', 'У вас нет премиум-аккаунта.'),
(12171, 'This premium command is disabled.', 'Эта премиум-команда отключена.'),
(12172, 'Premium error.', 'Ошибка премиума.'),
(12173, 'You cannot do that in a battleground.', 'Вы не можете сделать это на поле боя.'),
(12174, 'You cannot do that while in stealth.', 'Вы не можете сделать это в режиме незаметности.'),
(12175, 'You cannot do that while dead.', 'Вы не можете сделать это будучи мёртвым.'),
(12176, 'You cannot change race. Relog to apply.', 'Перезайдите, чтобы сменить расу.'),
(12177, 'You cannot customize. Relog to apply.', 'Перезайдите, чтобы изменить внешность.'),
(12178, 'You cannot do that in a group.', 'Вы не можете сделать это в группе.'),
(12179, 'Target is not a premium account.', 'У цели нет премиум-аккаунта.'),
(12180, 'Premium time remaining: {}', 'Осталось премиум-времени: {}'),
(12181, 'Your premium time will expire in less than 5 minutes.', 'Ваше премиум-время истечёт менее чем через 5 минут.'),
(12182, 'Your premium time has expired.', 'Ваше премиум-время истекло.'),
(12183, 'Target premium time remaining: {}', 'Осталось премиум-времени у цели: {}'),
(12184, 'This feature is in development.', 'Эта функция в разработке.');
