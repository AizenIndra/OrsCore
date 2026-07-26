/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "TransmogrificationMgr.h"

class npc_transmogrifier : public CreatureScript
{
public:
    npc_transmogrifier() : CreatureScript("npc_transmogrifier") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!creature->IsTransmogrifier())
            return false;

        AddGossipItemFor(player, player->GetGossipTextId(creature), 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (!creature->IsTransmogrifier())
            return false;

        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            CloseGossipMenuFor(player);
            sTransmogrificationMgr->SendTransmogrificationMenuOpenTo(player, creature);
            return true;
        }

        CloseGossipMenuFor(player);
        return false;
    }
};

void AddSC_npc_transmogrifier()
{
    new npc_transmogrifier();
}
