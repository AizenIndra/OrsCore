# Mak'Gora — architecture (AzerothCore)

## Placement

Ship as **`modules/mod-makgora`**, not core patches. Keeps OrsCore updatable and matches AC module conventions (`Addmod_makgoraScripts()`).

```
Client addon (Lua)  ←── AddonMessage "MakGora" ──→  PlayerScript
                                                      │
World DB (quests, NPC, strings, SmartAI)              │
Characters DB (leaders, flags, history)  ←────── MakGoraMgr
                                                      │
CreatureAI (elite champion) / future ceremony scripts ┘
```

## Layers

| Layer | Responsibility |
|--------|----------------|
| **MakGoraMgr** | Single source of truth: faction leaders, per-player flags, active ritual sessions, DB load/save, world announce |
| **Config** | `MakGora.*` via `ConfigValueCache` |
| **PlayerScript** | Duel lifecycle, login sync, permadeath enforcement, addon ingress |
| **WorldScript** | Config init + DB load on startup |
| **CommandScript** | GM/player test surface (`.makgora …`) |
| **CreatureAI** | Elite champion trial before vacant-seat claim |
| **World SQL** | Quests, spawns, gossip, ceremony, afterlife (content phases) |
| **Addon** | Atmosphere UI, bind toggle, state sync — never authority |

## Hooks that matter

| Hook | Use |
|------|-----|
| `OnPlayerDuelStart` / `OnPlayerDuelEnd` | Arena phase of an armed ritual; resolve title / permadeath |
| `OnPlayerLogin` | Load flags, sync addon, lock permanent ghosts |
| `OnPlayerResurrect` | Kill-loop block for permadeath exploits |
| `OnPlayerJustDied` / `OnPlayerReleasedGhost` | Messaging + afterlife hooks (later) |
| `OnPlayerBeforeSendChatMessage` (`LANG_ADDON`) | Addon protocol |
| `OnStartup` / `OnBeforeConfigLoad` | Load managers |

Vanilla duel does **not** permanently kill. Ritual must be **armed** (`.makgora ritual` / future gossip) before duel end applies Mak'Gora outcomes.

## DB (characters)

- `makgora_leader` — one row per faction (`team` 0/1)
- `makgora_player` — eligible, champion_defeated, hardcore_bound, permadeath, cosmetics
- `makgora_history` — audit / lore log

## Ritual state machine

```
Eligible → (ChampionTrial if seat vacant) → Pending → Arena(duel) → Resolved
                                              ↓
                                    TitleTransfer | Permadeath | Fled | Interrupted
```

## Champion (C++)

`npc_makgora_elite_champion` — ScriptedAI + TaskScheduler. Wire via `creature_template.ScriptName` when the custom entry exists (`MakGora.ChampionEntry` for gates).

## Addon protocol

Prefix: `MakGora` (config).

| Direction | Payload |
|-----------|---------|
| C→S | `PING` / `SYNC` / `BIND\t0\|1` |
| S→C | `PONG` / `STATE\tenabled\teligible\tbind\tpermadeath\tisLeader\tleaderName\tritualState` |

Authority stays on the server; UI only reflects state.

## What not to do

- Do not store raw `Player*` across ticks — use `ObjectGuid` (already in sessions).
- Do not put progression rules only in the addon.
- Do not edit `data/sql/base` / merged updates; module SQL lives under the module tree.
