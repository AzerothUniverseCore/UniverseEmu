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

#ifndef __ILLIDARI_ABILITIES_H
#define __ILLIDARI_ABILITIES_H

#include "Define.h"

class Creature;
class Unit;

namespace LegionScenario
{
    enum IllidariSpells : uint32
    {
        SPELL_MORSURE_DU_DEMON        = 162243,
        SPELL_EBRANLEMENT              = 98898,
        SPELL_LANCER_DE_GLAIVE         = 9994,
        SPELL_FRAPPE_INFERNALE         = 100006,
        SPELL_POINTS_DEMONIAQUES       = 203819,
        SPELL_ENTAILLE                 = 100010,
        SPELL_GLAIVES_RAPIDES          = 98835,
        SPELL_AURA_IMMOLATION          = 178740,
        SPELL_BARRAGE_GANGRENE         = 98868,
        SPELL_MARQUE_ENFLAMMEE         = 100007,
        SPELL_DANSE_DES_DOUBLES_LAMES  = 98829,
        SPELL_LAME_DU_CHAOS            = 98859,
        SPELL_MARCHE_DU_NEANT          = 98884,
        SPELL_VOILE_CORROMPU           = 98990
    };

    bool IsIllidariDemonHunter(uint32 entry);

    class IllidariCombatKit
    {
    public:
        IllidariCombatKit();

        void Reset();

        void Update(Creature* me, Unit* victim, uint32 diff);

    private:
        bool TryCastPriority(Creature* me, Unit* victim);
        void MaintainImmolationAura(Creature* me, uint32 diff);

        static constexpr uint32 SLOT_COUNT = 12;

        uint32 _cooldowns[SLOT_COUNT];
        uint32 _gcdTimer;
        uint32 _immolationRecastGuard;
    };
}

#endif
