# Mak'Gora — RPA feature plan

Structured Agentic Engineering: each feature is its own **Refine → Plan → Act** cycle. Fresh context per phase (`/clear`). Do not vibe-code the whole module in one session.

## Epic map

| ID | Feature | Depends on | Priority |
|----|---------|------------|----------|
| F0 | Foundation: mgr, DB, hooks, commands, addon stub | — | **Done (scaffold)** |
| F1 | Quest path (eligibility) | F0 | Next |
| F2 | Elite champion encounter (content + tuning) | F0 | Next |
| F3 | Ceremony + coronation NPCs | F1, F2 | High |
| F4 | Hardcore bind UX + afterlife | F0 | High |
| F5 | World reaction (gossip/phasing/worldstates) | F3 | Medium |
| F6 | Spectators + cinematic scripts | F3 | Medium |
| F7 | Cosmetic progression | F3 | Medium |
| F8 | Full client addon | F0, F4 | Medium |
| F9 | Balance / Classic Plus polish | all | Ongoing |

---

## F0 — Foundation (current)

**Requirements (done):** enable flag; leaders table; player flags; arm ritual → duel resolve; GM commands; addon STATE/SYNC/BIND; champion AI stub.

**Manual test:**
1. Enable config, restart, confirm SQL applied.
2. Two same-faction 60s: `.makgora eligible` on both, `.makgora bind on`, `.makgora ritual`, duel.
3. Winner becomes leader if seat vacant/contested; loser permadeath if bound.
4. Relog loser → still ghost messaging; resurrect blocked.

---

## F1 — Quest path

**Refine:** Prove worthiness before challenge — how many quests? Horde vs Alliance mirrored? Item/token proof?

**Plan:** world SQL only where possible (quest_template, objectives, SmartAI). Set `makgora_player.eligible` via quest reward script or `PlayerScript::OnPlayerCompleteQuest`.

**Act:** SQL pack + thin C++ reward hook. No ceremony yet.

---

## F2 — Champion content

**Refine:** Entry, map, difficulty at 60, loot/none, scripted phases.

**Plan:** creature_template + ScriptName `npc_makgora_elite_champion`; expand AI (EventMap/TaskScheduler); set `MakGora.ChampionEntry`.

**Act:** content SQL + AI pass; remove GM auto-`championDefeated` from `.makgora eligible` when live.

---

## F3 — Ceremony

**Refine:** Where? Which NPCs (Thrall / Wrynn stand-ins)? Spectator seating?

**Plan:** GameObject/Creature scripts + timed Path/Talk; optional instance or outdoor arena.

**Act:** cinematic timeline script; call `SetLeader` at climax.

---

## F4 — Afterlife

**Refine:** Ghost restrictions (chat/group already patterned by mod-hardcore). Unique zone vs graveyard redirect?

**Plan:** `OnPlayerBeforeChooseGraveyard` / map teleport; afterlife quests.

**Act:** permadeath map + NPCs; keep exploit locks from F0.

---

## F5–F8

Same RPA loop: one ticket folder under `.claude/plans/makgora-Fx-…/` with `TICKET.md` → `REQUIREMENTS.md` → `PLAN.md` → implement → in-game test → PR notes.

### Model spend hint

- Fetch/SQL drafts: cheaper model  
- Refine/Plan: strongest model  
- Act (mechanical SQL/C++ from a good plan): mid model  

---

## Definition of done (any feature)

- [ ] You can explain every line in review  
- [ ] Builds  
- [ ] In-game test steps written and run  
- [ ] AI involvement disclosed if contributing upstream  
