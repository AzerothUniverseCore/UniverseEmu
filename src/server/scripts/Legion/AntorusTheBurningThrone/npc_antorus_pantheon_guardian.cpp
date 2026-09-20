/*
 * This file is part of the UniverseEmu Project. See AUTHORS file for Copyright information
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
#include "ScriptedCreature.h"
#include "AntorusArgus.h"
#include "Log.h"

using namespace AntorusArgus;

class npc_antorus_pantheon_guardian : public CreatureScript
{
public:
    npc_antorus_pantheon_guardian() : CreatureScript("npc_antorus_pantheon_guardian") { }

    struct npc_antorus_pantheon_guardianAI : public ScriptedAI
    {
        npc_antorus_pantheon_guardianAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
        uint32 abilityCycleIndex = 0;

        void Reset() override
        {
            abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
            abilityCycleIndex = 0;
            RegisterParticipant(me->GetInstanceId(), me);
        }

        void EnterEvadeMode(EvadeReason why) override
        {
            SC_LOG_ERROR("server", "[Antorus] EnterEvadeMode on '{}' (entry {}, why={}, inCombat={}, hasVictim={}, activeParticipant={}) - resets ALL 7 to full health. Instance {}.",
                me->GetName(), me->GetEntry(), uint32(why), me->IsInCombat(), me->GetVictim() != nullptr,
                IsActiveParticipant(me->GetInstanceId(), me->GetEntry()), me->GetInstanceId());
            ResetEncounter(me->GetInstanceId());
            ScriptedAI::EnterEvadeMode(why);
        }

        void JustDied(Unit* killer) override
        {
            NotifyDeath(me->GetInstanceId(), me->GetEntry(), killer);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!IsActiveParticipant(me->GetInstanceId(), me->GetEntry()))
                return;

            if (!UpdateVictim())
                return;

            if (CheckSwapOut(me->GetInstanceId(), me))
                return;

            int32 const idx = IndexOfEntry(me->GetEntry());

            if (idx >= 0 && abilityTimer <= diff)
            {
                if (Unit* victim = me->GetVictim())
                {
                    CastRotationAbility(me, victim, uint32(idx), abilityCycleIndex);
                    ++abilityCycleIndex;
                }
                abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
            }
            else if (abilityTimer > diff)
                abilityTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_antorus_pantheon_guardianAI(creature);
    }
};

void AddSC_npc_antorus_pantheon_guardian()
{
    new npc_antorus_pantheon_guardian();
}
