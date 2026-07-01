/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Log.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include "World.h"

class npc_controlled_hozen : public CreatureScript
{
public:
    npc_controlled_hozen() : CreatureScript("npc_controlled_hozen") { }

    struct npc_controlled_hozenAI : public ScriptedAI
    {
        npc_controlled_hozenAI(Creature* creature) : ScriptedAI(creature) { }

        bool UpdateVictim()
        {
            me->CombatStop(true);
            me->AttackStop();
            me->RemoveAllAurasExceptType(SPELL_AURA_PROC_TRIGGER_SPELL);
            me->GetMotionMaster()->MoveTargetedHome();
        }

        bool UpdateVictim(Unit* /*who*/)
        {
            std::list<Creature*> temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 60.f);

            for (auto&& creature : temp)
                if (creature->IsAlive() && !creature->IsInCombat())
                    creature->AI()->DoZoneInCombat();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_controlled_hozenAI(creature);
    }
};

void AddSC_stormstout_brewery()
{
    new npc_controlled_hozen();
}