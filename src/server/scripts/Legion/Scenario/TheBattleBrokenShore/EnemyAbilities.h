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

#ifndef __ENEMY_ABILITIES_H
#define __ENEMY_ABILITIES_H

#include "Define.h"

class Creature;
class Unit;

namespace LegionScenario
{
    enum class EnemyArchetype : uint8
    {
        NONE = 0,
        NAGA,
        SHIVAN,
        BROKEN,
        GENERAL,
        BOSS_KROSUS,
        BOSS_BALEFUL,
        BOSS_INFERNAL
    };

    EnemyArchetype GetEnemyArchetype(uint32 entry);

    class EnemyCombatKit
    {
    public:
        EnemyCombatKit();

        void Init(uint32 entry);
        void Reset();
        bool IsActive() const { return _archetype != EnemyArchetype::NONE; }

        void Update(Creature* me, Unit* victim, uint32 diff);

    private:
        static constexpr uint32 SLOT_COUNT = 4;

        EnemyArchetype _archetype;
        uint32 _cooldowns[SLOT_COUNT];
        uint32 _gcdTimer;
    };
}

#endif
