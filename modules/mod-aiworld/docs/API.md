# AiWorld API contracts

## Ingress (AzerothCore → AI)

| Hook / source | Payload | Consumer |
|---------------|---------|----------|
| `OnPlayerLogin` / `OnPlayerLogout` / `OnPlayerUpdateZone` | guid, zone, side | `AiZoneAnalyzer` |
| `OnPlayerPVPKill` | zone, killer side | `AiZoneAnalyzer`, `AiFactionScore`, `AiWorldMemory` |
| Periodic sample (`Metrics.SampleSec`) | online counts in watched zones | `AiZoneAnalyzer` |
| `.aiworld *` GM commands | forced analyze / simulate / territory | `AiDirector` |

## Egress (AI → AzerothCore)

| Facade | Effect |
|--------|--------|
| `AiDirector::ApplyTerritoryChange` | DB territory + world announce |
| `AiEventScheduler::StartEvent` | announce, optional summons, history |
| `AiBotBridge::RequestDeploy/Defend/Assist` | queue bot orders (PlayerBots when available) |
| `ChatHandler::SendWorldText` | realm announcements (`acore_string` 900200+) |

## Bot bridge

```
AiBotBridge::RequestDeploy(side, zone, count, reason)
AiBotBridge::RequestDefend(zone, side, reason)
AiBotBridge::RequestAssist(playerGuid, zone, side)
```

When compiled with `MOD_PLAYERBOTS` (mod-playerbots present):

1. Select online random bots of the requested faction via `sRandomPlayerbotMgr.GetAllBots()`.
2. Prefer bots outside the target zone.
3. `TeleportTo` Hillsbrad spawn (or assist target position).
4. Set strategies `-rpg,+grind,+pvp` (assist also `+follow` + `SetMaster`).
5. `Refresh` random bots.

Without `MOD_PLAYERBOTS`, orders are still queued/logged into `aiworld_event_history`.

## LLM (Ollama)

**Endpoint:** `POST {AiWorld.Llm.Endpoint}/api/generate`

**Request body:**
```json
{
  "model": "llama3.2",
  "prompt": "...",
  "stream": false,
  "format": "json"
}
```

**Expected model JSON (inside `response`):**
```json
{
  "action": "ChangeTerritory|DeployBots|StartEvent|Announce",
  "beneficiary": "Alliance|Horde|Neutral",
  "botCount": 5,
  "eventTemplateId": 1,
  "reason": "short reason",
  "dialogue": null
}
```

Invalid JSON / unknown action → discarded; Director keeps using the rule-engine. Results apply on the **next** world-thread drain of `AiTaskQueue::PopCompletedAction`.

## Director decision inputs

`ZoneSnapshot`: players/bots per side, rolling PvP kills, territory state, control score, tension.

Power: `players*2 + bots + recentKills`.

## GM surface

```
.aiworld status
.aiworld analyze [zone]
.aiworld event [zone]
.aiworld tension [0-100]
.aiworld memory [zone]
.aiworld territory <zone> <alliance|horde|contested>
.aiworld simulate imbalance <alliance|horde> [magnitude]
.aiworld simulate clear
.aiworld reload
```
