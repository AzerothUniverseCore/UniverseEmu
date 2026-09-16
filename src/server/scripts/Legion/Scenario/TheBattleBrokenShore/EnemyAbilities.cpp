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

#include "EnemyAbilities.h"
#include "TheBattleBrokenShore.h"
#include "ScriptedCreature.h"

namespace LegionScenario
{
    EnemyArchetype GetEnemyArchetype(uint32 entry)
    {
        if (entry == BOSS_ENTRIES[2]) return EnemyArchetype::BOSS_KROSUS;
        if (entry == BOSS_ENTRIES[0]) return EnemyArchetype::BOSS_BALEFUL;
        if (entry == BOSS_ENTRIES[1]) return EnemyArchetype::BOSS_INFERNAL;

        for (uint32 e : ENEMY_NAGA)
            if (e == entry) return EnemyArchetype::NAGA;
        for (uint32 e : ENEMY_SHIVAN)
            if (e == entry) return EnemyArchetype::SHIVAN;
        for (uint32 e : ENEMY_BROKEN)
            if (e == entry) return EnemyArchetype::BROKEN;
        for (uint32 e : ENEMY_GENERAL)
            if (e == entry) return EnemyArchetype::GENERAL;

        return EnemyArchetype::NONE;
    }

    namespace
    {
        struct AbilityDef
        {
            uint32 spellId;
            uint32 cooldownMs;
            float  minRange;
            float  maxRange;
            bool   needsMeleeRange;
            bool   selfCast;
            bool   lowHealthOnly;
        };

        constexpr uint32 SLOTS = 4;

        constexpr AbilityDef NAGA_ABILITIES[SLOTS] =
        {
            { 65174, 14000, 0.0f, 25.0f, false, false, false },
            { 22800, 18000, 0.0f, 20.0f, false, false, false },
            { 50762,  8000, 0.0f, 20.0f, false, false, false },
            { 34353,  5000, 0.0f, 25.0f, false, false, false }
        };

        constexpr AbilityDef SHIVAN_ABILITIES[SLOTS] =
        {
            { 46264, 20000, 0.0f, 20.0f, false, false, false },
            { 15398, 28000, 0.0f, 10.0f, false, false, false },
            {   606,  9000, 0.0f, 25.0f, false, false, false },
            { 17238, 11000, 0.0f, 20.0f, false, false, false }
        };

        constexpr AbilityDef BROKEN_ABILITIES[SLOTS] =
        {
            { 17547, 16000, 0.0f, 5.0f, true, false, false },
            { 15577, 20000, 0.0f, 5.0f, true, true,  false },
            { 20569,  6000, 0.0f, 5.0f, true, false, false },
            { 12323, 10000, 0.0f, 5.0f, true, true,  false }
        };

        constexpr AbilityDef GENERAL_ABILITIES[SLOTS] =
        {
            { 24669, 18000, 0.0f, 15.0f, false, false, false },
            { 37648, 25000, 0.0f, 0.0f,  false, true,  true  },
            { 39054,  6000, 0.0f, 30.0f, false, false, false },
            { 12742,  8000, 0.0f, 25.0f, false, false, false }
        };

        constexpr AbilityDef BOSS_KROSUS_ABILITIES[SLOTS] =
        {
            { 24573, 16000, 0.0f, 5.0f,  true,  false, false },
            { 28335, 20000, 0.0f, 5.0f,  true,  true,  false },
            { 12809, 22000, 0.0f, 5.0f,  true,  false, false },
            { 39031, 40000, 0.0f, 0.0f,  false, true,  true  }
        };

        constexpr AbilityDef BOSS_BALEFUL_ABILITIES[SLOTS] =
        {
            { 19278, 18000, 0.0f, 25.0f, false, false, false },
            { 46264, 22000, 0.0f, 20.0f, false, false, false },
            { 15398, 30000, 0.0f, 10.0f, false, false, false },
            { 17238, 10000, 0.0f, 20.0f, false, false, false }
        };

        constexpr AbilityDef BOSS_INFERNAL_ABILITIES[SLOTS] =
        {
            { 28794, 20000, 0.0f, 15.0f, false, false, false },
            { 39031, 40000, 0.0f, 0.0f,  false, true,  true  },
            { 39054,  6000, 0.0f, 30.0f, false, false, false },
            { 12742,  8000, 0.0f, 25.0f, false, false, false }
        };

        AbilityDef const* GetAbilityTable(EnemyArchetype archetype)
        {
            switch (archetype)
            {
                case EnemyArchetype::NAGA:          return NAGA_ABILITIES;
                case EnemyArchetype::SHIVAN:        return SHIVAN_ABILITIES;
                case EnemyArchetype::BROKEN:        return BROKEN_ABILITIES;
                case EnemyArchetype::GENERAL:       return GENERAL_ABILITIES;
                case EnemyArchetype::BOSS_KROSUS:   return BOSS_KROSUS_ABILITIES;
                case EnemyArchetype::BOSS_BALEFUL:  return BOSS_BALEFUL_ABILITIES;
                case EnemyArchetype::BOSS_INFERNAL: return BOSS_INFERNAL_ABILITIES;
                default:                            return nullptr;
            }
        }

        constexpr uint32 GCD_MS = 1500;
    }

    EnemyCombatKit::EnemyCombatKit() : _archetype(EnemyArchetype::NONE)
    {
        Reset();
    }

    void EnemyCombatKit::Init(uint32 entry)
    {
        _archetype = GetEnemyArchetype(entry);
        Reset();
    }

    void EnemyCombatKit::Reset()
    {
        for (uint32& cd : _cooldowns)
            cd = 0;
        _gcdTimer = 0;
    }

    void EnemyCombatKit::Update(Creature* me, Unit* victim, uint32 diff)
    {
        if (_archetype == EnemyArchetype::NONE || !me || !victim || !me->IsAlive() || !victim->IsAlive())
            return;

        AbilityDef const* table = GetAbilityTable(_archetype);
        if (!table)
            return;

        for (uint32& cd : _cooldowns)
            cd = (cd > diff) ? cd - diff : 0;

        if (_gcdTimer > diff)
        {
            _gcdTimer -= diff;
            return;
        }
        _gcdTimer = 0;

        bool lowHealth = me->HealthBelowPct(50);
        float dist = me->GetDistance(victim);

        for (uint32 slot = 0; slot < SLOT_COUNT; ++slot)
        {
            AbilityDef const& def = table[slot];
            if (def.spellId == 0)
                continue;

            if (def.lowHealthOnly && !lowHealth)
                continue;

            if (_cooldowns[slot] > 0)
                continue;

            if (def.needsMeleeRange && !me->IsWithinMeleeRange(victim))
                continue;
            if (!def.needsMeleeRange && def.maxRange > 0.0f && dist > def.maxRange)
                continue;
            if (def.minRange > 0.0f && dist < def.minRange)
                continue;

            Unit* target = def.selfCast ? static_cast<Unit*>(me) : victim;
            me->CastSpell(target, def.spellId, true);

            _cooldowns[slot] = def.cooldownMs;
            _gcdTimer = GCD_MS;
            return;
        }
    }
}
