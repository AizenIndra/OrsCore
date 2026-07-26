/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "MakGoraConfig.h"
#include "MakGoraMgr.h"
#include "CreatureScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "TaskScheduler.h"

struct npc_makgora_elite_champion : public ScriptedAI
{
    npc_makgora_elite_champion(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->Yell("Prove your worth, challenger!", LANG_UNIVERSAL);
        _scheduler.Schedule(3s, [this](TaskContext context)
        {
            if (Unit* victim = me->GetVictim())
                DoCast(victim, SPELL_SHADOW_BOLT);
            context.Repeat(4s, 6s);
        });
    }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player)
            return;

        sMakGoraMgr->EnsurePlayerLoaded(player->GetGUID().GetCounter());
        MakGora::PlayerRecord record = sMakGoraMgr->GetPlayerRecord(player->GetGUID().GetCounter());
        record.championDefeated = true;
        sMakGoraMgr->SetPlayerRecord(player->GetGUID().GetCounter(), record);
        sMakGoraMgr->SyncAddonState(player);
        me->Yell("The path of Mak'Gora is open to you.", LANG_UNIVERSAL);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
        DoMeleeAttackIfReady();
    }

private:
    static constexpr uint32 SPELL_SHADOW_BOLT = 116;
    TaskScheduler _scheduler;
};

void AddSC_MakGoraChampion()
{
    // Assign ScriptName = npc_makgora_elite_champion on the custom creature_template row.
    RegisterCreatureAI(npc_makgora_elite_champion);
}
