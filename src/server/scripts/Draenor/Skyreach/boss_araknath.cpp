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

/*
    Dungeon : Skyreach 97 - 99
    Encounter: Araknath
*/

#include "skyreach.h"

enum Says
{
    SAY_DEATH
};

enum Spells
{
    //Araknath
    SPELL_SUBMERGED_VISUAL    = 154163, // OK
    SPELL_MELEE               = 154121, // OK
    SPELL_SMASH_1             = 154113, // OK
    SPELL_BURST               = 154135, // OK a revoir

    //Arcanologist
    SPELL_SOLAR_DETONATION    = 160288, // OK
    SPELL_SOLAR_STORM         = 159215,

    //Phase Visual
    SPELL_ENERGIZE_SINGLE     = 156382, // OK
    SPELL_ENERGIZE_MASS       = 154177, // OK
    SPELL_ENERGIZE_HEAL       = 154179, // OK

    //Phase Battle
    SPELL_ENERGIZE_FORCE_CAST = 156384, //
    SPELL_ENERGIZE_AURA       = 154159, // OK
    SPELL_ENERGIZE_AURA_TICK  = 154139, // OK
    SPELL_ENERGIZE_HEAL_2     = 154149, // OK
    SPELL_ENERGIZE_DMG_PLR    = 154150  // OK
};

enum eEvents
{
    EVENT_MELE_ATTACK    = 1,
    EVENT_SMASH          = 2,
    EVENT_BURST          = 3,
    EVENT_ENERGIZE_BEAM  = 4
};

// Comes from EventMap.h into Legion Core 7.3.5
enum c_events
{
    EVENT_1 = 1,
    EVENT_2,
    EVENT_3
};

// Comes from EventMap.h into Legion Core 7.3.5
enum c_actions
{
    ACTION_NONE,

    ACTION_1,
    ACTION_2
};

struct boss_araknath : public BossAI
{
    explicit boss_araknath(Creature* creature) : BossAI(creature, DATA_ARAKNATH)
    {
        firstPull = true;
        initSummon = false;
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        DoCast(me, SPELL_SUBMERGED_VISUAL, true);
    }

    bool firstPull;
    bool initSummon;

    void Reset() override
    {
        _Reset();

        me->SummonCreatureGroup(0);

        if (!initSummon)
        {
            me->SummonCreature(NPC_SKYREACH_ARCANOLOGIST, 1061.96f, 1800.92f, 200.20f, 2.65f);
            
            initSummon = true;
        }
    }

    void JustEnteredCombat(Unit* who) override
    {
        BossAI::JustEnteredCombat(who);

        events.RescheduleEvent(EVENT_MELE_ATTACK, 2s);
        events.RescheduleEvent(EVENT_SMASH, 6s);
        events.RescheduleEvent(EVENT_BURST, 20s);
        events.RescheduleEvent(EVENT_ENERGIZE_BEAM, 18s);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() != NPC_SKYREACH_ARCANOLOGIST)
            summon->SetReactState(REACT_PASSIVE);

        if (!firstPull)
            return;

        switch (summon->GetEntry())
        {
        case NPC_INTERIOR_FOCUS:
            if (Creature* target = summon->FindNearestCreature(NPC_SUN_CONSTRUCT_ENERGIZER, 200.0f))
            {
                summon->CastSpell(target, SPELL_ENERGIZE_SINGLE);
            }
            break;
        case NPC_SUN_CONSTRUCT_ENERGIZER:
            if (Creature* target = summon->FindNearestCreature(NPC_SKYREACH_SUN_PROTOTYPE, 100.0f))
            {
                summon->CastSpell(target, SPELL_ENERGIZE_MASS);
            }
            break;
        case NPC_SKYREACH_SUN_PROTOTYPE:
            summon->CastSpell(me, SPELL_ENERGIZE_HEAL, true);
            break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            firstPull = false;
            EntryCheckPredicate pred1(NPC_INTERIOR_FOCUS);
            EntryCheckPredicate pred2(NPC_SUN_CONSTRUCT_ENERGIZER);
            summons.DoAction(ACTION_1, pred1);
            summons.DoAction(ACTION_1, pred2);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (me->GetDistance(me->GetHomePosition()) > 30.0f)
        {
            EnterEvadeMode(EVADE_REASON_BOUNDARY);
            return;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_MELE_ATTACK:
                if (auto target = me->GetVictim())
                    DoCast(target, SPELL_MELEE, false);

                events.RescheduleEvent(EVENT_MELE_ATTACK, 2s);
                break;
            case EVENT_SMASH:
                if (auto target = me->GetVictim())
                    DoCast(target, SPELL_SMASH_1, false);

                events.RescheduleEvent(EVENT_SMASH, 8s);
                break;
            case EVENT_BURST:
                DoCast(SPELL_BURST);
                events.RescheduleEvent(EVENT_BURST, 22s);
                break;
            case EVENT_ENERGIZE_BEAM:
                EntryCheckPredicate pred1(NPC_INTERIOR_FOCUS);
                summons.DoAction(ACTION_2, pred1);
                events.RescheduleEvent(EVENT_ENERGIZE_BEAM, 20s);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//76376
struct npc_skyreach_arcanologist : public ScriptedAI
{
    explicit npc_skyreach_arcanologist(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;
    ObjectGuid AraknathGUID;

    void Reset() override
    {
        events.Reset();
    }

    void JustEnteredCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2s);
        events.RescheduleEvent(EVENT_2, 8s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);

        if (auto araknath = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARAKNATH)))
        {
            araknath->AI()->DoAction(ACTION_1);
            araknath->RemoveAllAuras();
            araknath->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            araknath->SetReactState(REACT_AGGRESSIVE);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (Unit* randomTarget = SelectTarget(SelectTargetMethod::Random, 0))
                {
                    if (randomTarget->IsPlayer())
                    {
                        DoCast(randomTarget, SPELL_SOLAR_DETONATION);
                    }
                }
                events.RescheduleEvent(EVENT_1, 10s);
                break;
            case EVENT_2:
                DoCast(SPELL_SOLAR_STORM);
                events.RescheduleEvent(EVENT_2, 12s);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//77543, 76367
struct npc_araknath_energizer : public ScriptedAI
{
    explicit npc_araknath_energizer(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                me->InterruptSpell(CURRENT_CHANNELED_SPELL);
            break;
        case ACTION_2:
            DoCast(me, SPELL_ENERGIZE_FORCE_CAST, true);
            break;
        }
    }
};

//76142
struct npc_skyreach_sun_prototype : public ScriptedAI
{
    explicit npc_skyreach_sun_prototype(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        DoCast(SPELL_SUBMERGED_VISUAL);
    }

    EventMap events;

    void Reset() override {}

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_ENERGIZE_AURA)
        {
            me->RemoveAurasDueToSpell(SPELL_SUBMERGED_VISUAL);
            events.RescheduleEvent(EVENT_1, 3s);
            events.RescheduleEvent(EVENT_2, 15s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(SPELL_ENERGIZE_AURA_TICK);
                break;
            case EVENT_2:
                if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    me->InterruptSpell(CURRENT_CHANNELED_SPELL);

                DoCast(SPELL_SUBMERGED_VISUAL);
                break;
            }
        }
    }
};

// 154177
class spell_energize_mass : public SpellScript
{
    PrepareSpellScript(spell_energize_mass);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj) {
            return obj->GetEntry() != NPC_SKYREACH_SUN_PROTOTYPE;
            });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_energize_mass::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

//154140
class spell_araknath_energize : public SpellScript
{
    PrepareSpellScript(spell_araknath_energize);

    bool check = false;

    void Dummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                check = true;

                if (!caster->FindCurrentSpellBySpellId(SPELL_ENERGIZE_DMG_PLR))
                    caster->CastSpell(target, SPELL_ENERGIZE_DMG_PLR, true);
            }
        }
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!check && !caster->FindCurrentSpellBySpellId(SPELL_ENERGIZE_HEAL_2))
            caster->CastSpell(caster, SPELL_ENERGIZE_HEAL_2, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_araknath_energize::HandleAfterCast);
        OnEffectHitTarget += SpellEffectFn(spell_araknath_energize::Dummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_boss_araknath()
{
    RegisterCreatureAI(boss_araknath);
    RegisterCreatureAI(npc_skyreach_arcanologist);
    RegisterCreatureAI(npc_araknath_energizer);
    RegisterCreatureAI(npc_skyreach_sun_prototype);
    RegisterSpellScript(spell_araknath_energize);
    RegisterSpellScript(spell_energize_mass);
}
