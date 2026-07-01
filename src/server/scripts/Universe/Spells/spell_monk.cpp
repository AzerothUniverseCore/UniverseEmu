/*
 * This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
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

#include "GridNotifiers.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "SpellScript.h"

enum MonkSpells
{
		   SPELL_MONK_TRANSCENDENCE = 480190,
		   SPELL_MONK_TRANSCENDENCE_TRANSFER = 480030,
		   SPELL_MONK_TRANSCENDENCE_ALLOW_CAST = 62388,

           SPELL_MONK_TRANSCENDENCE_BRA = 4180190,
           SPELL_MONK_TRANSCENDENCE_TRANSFER_BRA = 4180030,

           SPELL_MONK_TRANSCENDENCE_BRU = 4280190,
           SPELL_MONK_TRANSCENDENCE_TRANSFER_BRU = 4280030,

		   SPELL_MONK_XUEN_AURA = 123999,

           SPELL_MONK_HEALING_SPHERE = 115460,

           SPELL_MONK_CELESTIAL_FLAMES = 325190,
           SPELL_MONK_BREATH_FIRE = 123725,

           NPC_BLACK_OX_STATUE = 61146,
           SPELL_MONK_SANCTUARY_OF_THE_OX = 126119,
};

enum MonkSummons
{
	
};

// 480190 - Transcendance
class spell_monk_transcendence : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
            GetTarget()->RemoveGameObject(GetId(), true);

        GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
    }

    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
        {
            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_MONK_TRANSCENDENCE_TRANSFER);

            if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
            {
                if (!GetTarget()->HasAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST))
                    GetTarget()->CastSpell(GetTarget(), SPELL_MONK_TRANSCENDENCE_ALLOW_CAST, true);
            }
            else
                GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_monk_transcendence::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_transcendence::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 480030 - Transcendance : Transfert
class spell_monk_transcendence_transfer : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence_transfer);

    void HandleTeleport(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            if (GameObject* circle = player->GetGameObject(SPELL_MONK_TRANSCENDENCE))
            {
                player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                player->RemoveMovementImpairingAuras(false);
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_transcendence_transfer::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 4180190 - Transcendance
class spell_monk_transcendence_bra : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence_bra);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
            GetTarget()->RemoveGameObject(GetId(), true);

        GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
    }

    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
        {
            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_MONK_TRANSCENDENCE_TRANSFER_BRA);

            if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
            {
                if (!GetTarget()->HasAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST))
                    GetTarget()->CastSpell(GetTarget(), SPELL_MONK_TRANSCENDENCE_ALLOW_CAST, true);
            }
            else
                GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_monk_transcendence_bra::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_transcendence_bra::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 4180030 - Transcendance : Transfert
class spell_monk_transcendence_transfer_bra : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence_transfer_bra);

    void HandleTeleport(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            if (GameObject* circle = player->GetGameObject(SPELL_MONK_TRANSCENDENCE_BRA))
            {
                player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                player->RemoveMovementImpairingAuras(false);
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_transcendence_transfer_bra::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 4280190 - Transcendance
class spell_monk_transcendence_bru : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence_bru);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
            GetTarget()->RemoveGameObject(GetId(), true);

        GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
    }

    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
        {
            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_MONK_TRANSCENDENCE_TRANSFER_BRU);

            if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
            {
                if (!GetTarget()->HasAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST))
                    GetTarget()->CastSpell(GetTarget(), SPELL_MONK_TRANSCENDENCE_ALLOW_CAST, true);
            }
            else
                GetTarget()->RemoveAura(SPELL_MONK_TRANSCENDENCE_ALLOW_CAST);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_monk_transcendence_bru::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_transcendence_bru::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 4280030 - Transcendance : Transfert
class spell_monk_transcendence_transfer_bru : public AuraScript
{
    PrepareAuraScript(spell_monk_transcendence_transfer_bru);

    void HandleTeleport(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            if (GameObject* circle = player->GetGameObject(SPELL_MONK_TRANSCENDENCE_BRU))
            {
                player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                player->RemoveMovementImpairingAuras(false);
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_transcendence_transfer_bru::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};
 
 // 68888 - Invocation de Xuen, le Tigre blanc
struct npc_monk_xuen : public ScriptedAI
{
    npc_monk_xuen(Creature* c) : ScriptedAI(c) { }

    void IsSummonedBy(WorldObject* /*summoner*/)
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (!me->GetCharmInfo()->GetCharmSpell(i)->GetAction())
            {
                if (auto spellInfo = sSpellMgr->GetSpellInfo(130793))
                {
                    me->GetCharmInfo()->GetCharmSpell(i)->SetActionAndType(spellInfo->Id, ACT_DISABLED);
                    me->GetCharmInfo()->AddSpellToActionBar(spellInfo, ACT_DISABLED);
                }
                break;
            }
        }
    }

    void Reset() override
    {
        me->CastSpell(me, 123995, true); // Invoke Xuen, the White Tiger mod threat
        me->CastSpell(me, 123999, true); // Crackling Tiger Lightning Driver
        me->CastSpell(me, 130324, true); // Eminence
    }
};

// 61146
struct npc_black_ox_statue : public ScriptedAI
{
    npc_black_ox_statue(Creature* c) :
        ScriptedAI(c) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->CastSpell(me->GetOwner(), SPELL_MONK_SANCTUARY_OF_THE_OX, true);
    }

    void Unsummoned()
    {
        if (Unit* owner = me->GetOwner())
            owner->RemoveAurasDueToSpell(SPELL_MONK_SANCTUARY_OF_THE_OX);
    }

    void UpdateAI(uint32) override { }
};
 
void AddSC_monk_spell_scripts()
{
		   RegisterSpellScript(spell_monk_transcendence);
		   RegisterSpellScript(spell_monk_transcendence_transfer);
           RegisterSpellScript(spell_monk_transcendence_bra);
           RegisterSpellScript(spell_monk_transcendence_transfer_bra);
           RegisterSpellScript(spell_monk_transcendence_bru);
           RegisterSpellScript(spell_monk_transcendence_transfer_bru);
		   RegisterCreatureAI(npc_monk_xuen);
           RegisterCreatureAI(npc_black_ox_statue);
}