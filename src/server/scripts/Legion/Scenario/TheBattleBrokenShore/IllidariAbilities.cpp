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

#include "IllidariAbilities.h"
#include "TheBattleBrokenShore.h"
#include "ScriptedCreature.h"

namespace LegionScenario
{
    bool IsIllidariDemonHunter(uint32 entry)
    {
        for (uint32 e : ALLIED_HORDE_ILLIDARI)
            if (e == entry) return true;
        for (uint32 e : ALLIED_ALLIANCE_ILLIDARI)
            if (e == entry) return true;
        return false;
    }

    namespace
    {
        enum AbilitySlot : uint8
        {
            SLOT_MORSURE_DU_DEMON = 0,
            SLOT_EBRANLEMENT,
            SLOT_LANCER_DE_GLAIVE,
            SLOT_FRAPPE_INFERNALE,
            SLOT_ENTAILLE,
            SLOT_GLAIVES_RAPIDES,
            SLOT_BARRAGE_GANGRENE,
            SLOT_MARQUE_ENFLAMMEE,
            SLOT_DANSE_DES_DOUBLES_LAMES,
            SLOT_LAME_DU_CHAOS,
            SLOT_MARCHE_DU_NEANT,
            SLOT_VOILE_CORROMPU,
            SLOT_COUNT
        };

        struct AbilityDef
        {
            AbilitySlot slot;
            uint32 spellId;
            uint32 cooldownMs;
            float  minRange;
            float  maxRange;
            bool   needsMeleeRange;
            bool   selfCast;
        };

        constexpr AbilityDef ABILITIES[SLOT_COUNT] =
        {
            { SLOT_MORSURE_DU_DEMON,       SPELL_MORSURE_DU_DEMON,       4000,  0.0f, 30.0f, false, false },
            { SLOT_EBRANLEMENT,            SPELL_EBRANLEMENT,            8000,  0.0f, 5.0f,  true,  false },
            { SLOT_LANCER_DE_GLAIVE,       SPELL_LANCER_DE_GLAIVE,       10000, 8.0f, 30.0f, false, false },
            { SLOT_FRAPPE_INFERNALE,       SPELL_FRAPPE_INFERNALE,       15000, 0.0f, 30.0f, false, false },
            { SLOT_ENTAILLE,               SPELL_ENTAILLE,               6000,  0.0f, 5.0f,  true,  false },
            { SLOT_GLAIVES_RAPIDES,        SPELL_GLAIVES_RAPIDES,        5000,  0.0f, 5.0f,  true,  false },
            { SLOT_BARRAGE_GANGRENE,       SPELL_BARRAGE_GANGRENE,       20000, 0.0f, 15.0f, false, false },
            { SLOT_MARQUE_ENFLAMMEE,       SPELL_MARQUE_ENFLAMMEE,       20000, 0.0f, 30.0f, false, false },
            { SLOT_DANSE_DES_DOUBLES_LAMES,SPELL_DANSE_DES_DOUBLES_LAMES,18000, 0.0f, 8.0f,  true,  false },
            { SLOT_LAME_DU_CHAOS,          SPELL_LAME_DU_CHAOS,          24000, 0.0f, 5.0f,  true,  false },
            { SLOT_MARCHE_DU_NEANT,        SPELL_MARCHE_DU_NEANT,        20000, 8.0f, 30.0f, false, false },
            { SLOT_VOILE_CORROMPU,         SPELL_VOILE_CORROMPU,         45000, 0.0f, 0.0f,  false, true  }
        };

        constexpr AbilitySlot BIG_COOLDOWN_PRIORITY[] =
        {
            SLOT_VOILE_CORROMPU,
            SLOT_LAME_DU_CHAOS,
            SLOT_BARRAGE_GANGRENE,
            SLOT_DANSE_DES_DOUBLES_LAMES,
            SLOT_MARQUE_ENFLAMMEE,
            SLOT_FRAPPE_INFERNALE,
            SLOT_MARCHE_DU_NEANT
        };

        constexpr AbilitySlot FILLER_PRIORITY[] =
        {
            SLOT_ENTAILLE,
            SLOT_GLAIVES_RAPIDES,
            SLOT_EBRANLEMENT,
            SLOT_LANCER_DE_GLAIVE,
            SLOT_MORSURE_DU_DEMON
        };

        constexpr size_t BIG_COOLDOWN_COUNT = sizeof(BIG_COOLDOWN_PRIORITY) / sizeof(BIG_COOLDOWN_PRIORITY[0]);
        constexpr size_t FILLER_COUNT = sizeof(FILLER_PRIORITY) / sizeof(FILLER_PRIORITY[0]);

        constexpr uint32 GCD_MS = 1500;
        constexpr uint32 IMMOLATION_RECAST_GUARD_MS = 2000;
    }

    IllidariCombatKit::IllidariCombatKit()
    {
        Reset();
    }

    void IllidariCombatKit::Reset()
    {
        for (uint32& cd : _cooldowns)
            cd = 0;
        _gcdTimer = 0;
        _immolationRecastGuard = 0;
    }

    void IllidariCombatKit::MaintainImmolationAura(Creature* me, uint32 diff)
    {
        if (_immolationRecastGuard > diff)
        {
            _immolationRecastGuard -= diff;
            return;
        }
        _immolationRecastGuard = 0;

        if (!me->HasAura(SPELL_AURA_IMMOLATION))
        {
            me->CastSpell(me, SPELL_AURA_IMMOLATION, true);
            _immolationRecastGuard = IMMOLATION_RECAST_GUARD_MS;
        }
    }

    bool IllidariCombatKit::TryCastPriority(Creature* me, Unit* victim)
    {
        bool lowHealth = me->HealthBelowPct(50);
        float dist = me->GetDistance(victim);

        auto tryList = [&](AbilitySlot const* list, size_t count) -> bool
        {
            for (size_t i = 0; i < count; ++i)
            {
                AbilitySlot slot = list[i];
                AbilityDef const& def = ABILITIES[slot];

                if (slot == SLOT_VOILE_CORROMPU && !lowHealth)
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
                return true;
            }
            return false;
        };

        if (tryList(BIG_COOLDOWN_PRIORITY, BIG_COOLDOWN_COUNT))
            return true;

        return tryList(FILLER_PRIORITY, FILLER_COUNT);
    }

    void IllidariCombatKit::Update(Creature* me, Unit* victim, uint32 diff)
    {
        if (!me || !victim || !me->IsAlive() || !victim->IsAlive())
            return;

        for (uint32& cd : _cooldowns)
            cd = (cd > diff) ? cd - diff : 0;

        MaintainImmolationAura(me, diff);

        if (_gcdTimer > diff)
        {
            _gcdTimer -= diff;
            return;
        }
        _gcdTimer = 0;

        TryCastPriority(me, victim);
    }
}
