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

using namespace AntorusArgus;

enum ArgusTextGroups
{
    TEXT_ARGUS_ENGAGE  = 0,
    TEXT_ARGUS_DEATH   = 1,
    TEXT_ARGUS_RETREAT = 2
};

class npc_antorus_argus_the_unmaker : public CreatureScript
{
public:
    npc_antorus_argus_the_unmaker() : CreatureScript("npc_antorus_argus_the_unmaker") { }

    struct npc_antorus_argus_the_unmakerAI : public ScriptedAI
    {
        npc_antorus_argus_the_unmakerAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
        uint32 abilityCycleIndex = 0;

        void Reset() override
        {
            abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
            abilityCycleIndex = 0;
            RegisterParticipant(me->GetInstanceId(), me);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            NotifyEngaged(me->GetInstanceId());
            Talk(TEXT_ARGUS_ENGAGE);
        }

        void EnterEvadeMode(EvadeReason why) override
        {
            ResetEncounter(me->GetInstanceId());
            ScriptedAI::EnterEvadeMode(why);
        }

        void JustDied(Unit* killer) override
        {
            NotifyDeath(me->GetInstanceId(), me->GetEntry(), killer);
            Talk(TEXT_ARGUS_DEATH);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
        {
            if (IsSafeForArgusToDie(me->GetInstanceId()))
                return;

            uint32 const curHealth = me->GetHealth();
            if (damage >= curHealth)
                damage = curHealth > 1 ? curHealth - 1 : 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!IsActiveParticipant(me->GetInstanceId(), me->GetEntry()))
                return;

            if (!UpdateVictim())
                return;

            if (CheckSwapOut(me->GetInstanceId(), me))
                return;

            if (abilityTimer <= diff)
            {
                if (Unit* victim = me->GetVictim())
                {
                    CastRotationAbility(me, victim, 0, abilityCycleIndex);
                    ++abilityCycleIndex;
                }
                abilityTimer = ROTATION_ABILITY_COOLDOWN_MS;
            }
            else
                abilityTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_antorus_argus_the_unmakerAI(creature);
    }
};

void AddSC_npc_antorus_argus_the_unmaker()
{
    new npc_antorus_argus_the_unmaker();
}
