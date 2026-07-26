# AI World System for AzerothCore

Strategic world AI for Classic 3.3.5a (OrsCore): zone analysis, faction territory control, event scheduling, PlayerBot orders, optional local LLM (Ollama).

## Status

Implemented as `modules/mod-aiworld`:

- AI Director (periodic rule-engine + tension)
- World Memory (MySQL characters tables + in-memory cache)
- Zone / faction analyzers
- Event scheduler (Hillsbrad skirmish)
- Bot bridge (orders queued; **wired to mod-playerbots** via `MOD_PLAYERBOTS`: teleport + grind/pvp strategies)
- Async LLM task queue + Ollama HTTP client
- Economy analyzer stub
- GM commands for demo / ops

## Install

1. Module path: `modules/mod-aiworld` (requires `modules/mod-playerbots` for live bot deploys).
2. Reconfigure CMake and rebuild `worldserver` (`MODULES=static` or dynamic).
3. Copy `conf/aiworld.conf.dist` → `aiworld.conf` (or merge into `worldserver.conf`) and set `AiWorld.Enable = 1`.
4. Apply module SQL (auto via module DB updater, or manually):
   - `data/sql/db-characters/base/aiworld_characters.sql`
   - `data/sql/db-world/base/aiworld_strings.sql`

## Quick demo (Hillsbrad)

Ensure random playerbots are online, then:

```
.aiworld simulate imbalance horde 10
.aiworld analyze 267
.aiworld status
.aiworld memory 267
```

Expect: Alliance bots teleported to Hillsbrad, territory/event actions, world announces, DB history rows.

## Optional LLM

1. Run Ollama locally with a small model (e.g. `llama3.2`).
2. Set `AiWorld.Llm.Enable = 1` and endpoint/model in conf.
3. Director enqueues proposals when zone tension ≥ 40; never blocks the world tick.

## Docs

- [Architecture](docs/ARCHITECTURE.md)
- [API](docs/API.md)
