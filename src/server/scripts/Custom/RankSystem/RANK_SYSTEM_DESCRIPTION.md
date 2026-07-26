# Rank System Description

## Назначение

Ранк-система добавляет отдельную PvP-прогрессию персонажа:

- хранит `rankPoints` в таблице `characters`;
- рассчитывает ранг по порогам (1..50);
- выдает стакающуюся ауру ранга `71201` как визуальный/системный индикатор;
- награждает очками ранга и золотом за PvP-активности;
- поддерживает VIP-множитель на получение rank points;
- выдает инстанс-баффы, зависящие от текущего ранга;
- выводит информацию о ранге в `.pinfo`;
- добавляет кастомные скрипты (логин/синхронизация и rank vendors).

---

## Где находится логика

### Core (обязательная часть)

- `src/server/game/Entities/Player/Player.h`
  - `m_rankPoints`, `GetRankPoints()`, `SetRankPoints()`
  - методы ранк-системы (`RewardRankPoints`, `CanRankUp`, `GetRankByExp`, и т.д.)
- `src/server/game/Globals/ObjectMgr.*`
  - загрузка порогов из `rank_system_levels` (единственный источник шкалы)
- `src/server/game/Entities/Player/Player.cpp`
  - начисления/вывод/выдача рангов
  - синхронизация аур ранга
  - инстанс-баффы по рангу
- `src/server/game/Entities/Player/PlayerStorage.cpp`
  - загрузка `rankPoints` из `characters`
- `src/server/database/Database/Implementation/CharacterDatabase.cpp`
  - `rankPoints` добавлен в `SELECT/INSERT/UPDATE` prepared statements
- `src/server/game/Battlegrounds/Battleground.cpp`
  - награда за победу/поражение в BG
- `src/server/game/Battlegrounds/Arena.cpp`
  - награда за победу/поражение на арене
- `modules/mod-premium`
  - `Premium.Rate.Rank.Reward` — множитель очков ранга для VIP
  - без VIP в инстансах стеки баффов ранга делятся пополам
- `src/server/game/Spells/SpellInfoCorrections.cpp`
  - фиксы для `71201`, `62519`, `66721`
- `src/server/scripts/Commands/cs_misc.cpp`
  - вывод rank points в `.pinfo`
- `src/server/game/Miscellaneous/Language.h`
  - `LANG_PINFO_CHAR_RANK_POINTS = 35411`

### Custom scripts (скриптовая часть)

- `src/server/scripts/Custom/RankSystem/OnLogin.cpp`
  - `OnPlayerLogin`: синхронизация ранга, инфо-сообщение, проверка инстанс-баффа
  - `OnPlayerMapChanged`: пересчет инстанс-баффа при смене карты
- `src/server/scripts/Custom/RankSystem/RankVendor.cpp`
  - rank-gated gossip/vendor меню 1..50
- `src/server/scripts/Custom/custom_script_loader.cpp`
  - регистрация `AddSC_Login_script()` и `AddSC_NPC_RANK_VENDOR()`
- `src/server/scripts/World/item_scripts.cpp`
  - `ItemUse_Glory_Exp`: предметы, дающие rank points (`PVP_ITEM`)

---

## Данные и SQL

### Characters DB

- `data/sql/updates/db_characters/2026_04_05_00.sql`
  - добавляет колонку:
    - `characters.rankPoints INT UNSIGNED NOT NULL DEFAULT 0`

### World DB

- `data/sql/updates/db_world/2026_04_05_00.sql`
  - добавляет строку локализации для `.pinfo`:
    - `acore_string.entry = 35411`
    - EN: `Rank points: {}`
    - RU: `Очки ранга: {}`

---

## Пороги рангов

Пороги берутся **только** из world-таблицы `rank_system_levels` (`rank`, `required_points`).
После правки в БД: `.reload rank_system_levels`.

Ранг и «до следующего» считаются по `characters.rankPoints` + порогам из БД (`GetRankByExp` / `PointsUntilNextRank`).

Хардкода шкалы в C++ больше нет.

---

## Начисление rank points и наград

### 1) Убийство игрока (open world / PvP kill path)

Файл: `Player.cpp` (`RewardHonor`):

- если `RankSystem.RewardWinArenaEnable = 1` и цель — игрок:
  - + `RankSystem.RewardKillBG` очков ранга (`PVP_KILL`)
  - денежная награда через `RewardRankMoney(6, rate)`

### 2) Battleground

Файл: `Battleground.cpp` (`EndBattleground`):

- победитель:
  - + `RankSystem.RewardWinBG` очков (`PVP_BG`)
  - деньги `RewardRankMoney(4, rate, true)`
- проигравший:
  - деньги `RewardRankMoney(4, rate, false)`
  - очки ранга не выдаются

### 3) Arena

Файл: `Arena.cpp` (`EndBattleground`):

- победитель:
  - очки ранга: коэффициент по размеру арены * `RankSystem.RewardWinArena`
  - деньги `RewardRankMoney(GetArenaType(), rate, true)`
- проигравший:
  - деньги `RewardRankMoney(GetArenaType(), rate, false)`
  - очки ранга не выдаются

### 4) Предметы ранга

Файл: `item_scripts.cpp`, `ItemUse_Glory_Exp`:

- предметы `1042/1043/1044/35778/842` дают фиксированные очки ранга
- начисление идет как источник `PVP_ITEM`
- предмет уничтожается при успешном использовании

---

## VIP-множитель ранга

Файл: `Player.cpp`, `RewardRankPoints`:

- если `player->IsPremium() == true`, то начисляемые rank points умножаются на:
  - `Rate.Rank.Reward.Premium`

Конфиг:

- `worldserver.conf.dist`
  - `Rate.Rank.Reward.Premium = 1` (по умолчанию)

Пример:

- поставить `Rate.Rank.Reward.Premium = 2`, чтобы VIP получал x2 rank points.

---

## Выдача ранга (ауры)

Основная аура ранга:

- `RANK_SYSTEM_AURA = 71201`
- стакается до 50

Алгоритм:

- `RewardRankPoints()` -> `CanRankUp()`
- `CanRankUp()` в цикле вызывает `RewardPvPRank()` пока хватает points
- `RewardPvPRank()`:
  - добавляет 1 стак `71201`
  - кастует визуал `47292`
  - отправляет системное сообщение
  - пересчитывает инстанс-баффы

При логине:

- `RankControlOnLogin()` приводит количество стаков `71201` в соответствие с `GetRankByExp()`.

---

## Инстанс-баффы от ранга

Файлы:

- `Player.cpp`
  - `GetRangBuffInInstance(int amount)`
  - `RemoveRankBuff()`
  - `VerifiedRankBuff(Map* map)`
- `OnLogin.cpp`
  - `OnPlayerLogin` и `OnPlayerMapChanged` вызывают `VerifiedRankBuff`

Логика:

- в рейде/данже добавляются стаки:
  - `62519` (healing)
  - `66721` (damage)
- количество стаков = текущий ранг
- вне рейда/данжа баффы снимаются

---

## Конфиги RankSystem

В `worldserver.conf.dist`:

- `RankSystem.RewardWinArenaEnable = 1`
- `RankSystem.RewardWinArena = 25`
- `RankSystem.RewardWinBG = 150`
- `RankSystem.RewardKillBG = 10`

Сопоставление в коде:

- `CONFIG_RANK_SYSTEM_WIN_ENABLE`
- `CONFIG_RANK_SYSTEM_WIN_RATE_ARENA`
- `CONFIG_RANK_SYSTEM_WIN_RATE_BG`
- `CONFIG_RANK_SYSTEM_KILL_RATE_BG`

---

## Команда .pinfo

Добавлен вывод rank points:

- `cs_misc.cpp` печатает `LANG_PINFO_CHAR_RANK_POINTS`
- строка в DB:
  - `acore_string.entry = 35411`

Примечание:

- сейчас rank points точно отображаются для online-target;
- для offline-target используется текущая ветка логики `.pinfo` (без отдельного SQL-поля `rankPoints` в `CHAR_SEL_CHAR_PINFO`).

---

## Rank Vendors

Файл: `RankVendor.cpp`.

Реализовано:

- NPC-скрипты с выдачей меню по диапазонам рангов: 1-10, 11-20, 21-30, 31-40, 41-50;
- доступ к конкретному vendor entry через `SendListInventory(..., action)` только при достаточном ранге;
- отдельный NPC info-меню с выводом:
  - текущий ранг
  - текущие rank points
  - points до следующего ранга

---

## Что нужно применить после кода

1. Применить SQL:
   - `db_characters/2026_04_05_00.sql`
   - `db_world/2026_04_05_00.sql`
2. Убедиться, что включены конфиги `RankSystem.*` и нужный `Rate.Rank.Reward.Premium`.
3. Перезапустить worldserver.

---

## Краткий поток работы системы

1. Игрок получает PvP-награду -> `RewardRankPoints`.
2. Применяется VIP-множитель (если Premium).
3. Обновляются `rankPoints`.
4. `CanRankUp` проверяет пороги и добавляет стаки ранга.
5. На логине/смене карты идет синхронизация ранга и инстанс-баффов.
6. `.pinfo` и RankVendor используют текущее состояние (`rankPoints` + стаки `71201`).
