# Mak'Gora for AzerothCore

Endgame ritual system for Classic Plus (level 60): leadership through Mak'Gora, optional permadeath, world reaction, and a companion addon.

## Status

Foundation (MVP scaffold) is in place:

- Module config + character/world SQL
- `MakGoraMgr` (leaders, player flags, ritual sessions)
- Player/World hooks (duel → ritual resolve, permadeath ghost lock)
- GM commands for testing
- Elite champion AI stub (`npc_makgora_elite_champion`)
- Addon protocol stub (`MakGora\t…`)

Content (quest chain, ceremony NPCs, afterlife map, cosmetics, full UI) is planned in `docs/RPA.md`.

## Install

1. Ensure the module lives at `modules/mod-makgora`.
2. Reconfigure CMake and rebuild `worldserver` (`MODULES=static` or dynamic).
3. Copy `conf/makgora.conf.dist` → your `makgora.conf` (or merge into `worldserver.conf`) and set `MakGora.Enable = 1`.
4. Let the DB updater import module SQL, or apply manually:
   - `data/sql/db-characters/base/makgora_characters.sql`
   - `data/sql/db-world/base/makgora_strings.sql`

## Quick test (GM)

```
.makgora eligible          # mark selected/self eligible (+ champion flag)
.makgora bind on           # opt into permadeath for the next ritual
.makgora ritual            # arm ritual vs selected player
# then start a normal duel
.makgora status
.makgora leader            # force-set selected/self as faction leader
```

## Docs

- [Architecture](docs/ARCHITECTURE.md)
- [RPA feature plan](docs/RPA.md)
- [Announcement text](docs/ANNOUNCEMENT.md)
- [Addon stub](addon/MakGora/)
