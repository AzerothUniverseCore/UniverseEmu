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

#ifndef __LEGION_SCENARIO_H
#define __LEGION_SCENARIO_H

#include "Define.h"

class Creature;
class Player;

namespace LegionScenario
{
    // Map 833 "The Battle for Broken Shore"
    constexpr uint32 MAP_ID = 833;

    constexpr float ZONE_MIN_X = -2652.65f;
    constexpr float ZONE_MAX_X = -7.71094f;
    constexpr float ZONE_MIN_Y = 1084.22f;
    constexpr float ZONE_MAX_Y = 4247.64f;

    bool IsInScenarioZone(uint32 mapId, float x, float y);

    bool IsSameScenarioInstance(Creature const* a, Creature const* b);
    bool BelongsToPlayer(Creature const* creature, Player const* player);

    enum Side : uint8
    {
        SIDE_NONE   = 0,
        SIDE_ENEMY  = 1,
        SIDE_ALLIED = 2
    };

    Side GetSide(uint32 entry);

    bool IsBossEntry(uint32 entry);

    // Enemy
    // Naga
    constexpr uint32 ENEMY_NAGA[] =
    {
        956230, 956231, 1502909, 953693, 956228
    };

    // Shivan
    constexpr uint32 ENEMY_SHIVAN[] =
    {
        956252, 956253, 954435, 953707
    };

    // Broken
    constexpr uint32 ENEMY_BROKEN[] =
    {
        950247, 955449, 955447, 954450, 1502905, 1502906
    };

    // General enemy
    constexpr uint32 ENEMY_GENERAL[] =
    {
        957603, 953716, 1501288, 959650, 1501748, 959656, 954654, 953115,
        956400, 955046, 956278, 957604, 1502714, 953112, 954744, 955226,
        958907,
        953105,
        956159,
        1505316, 957594, 958486, 958482, 958484, 958497, 954655,
        151002
    };

    // Allied
    // Blood Elf Illidari (Horde)
    constexpr uint32 ALLIED_HORDE_ILLIDARI[] =
    {
        1505945, 953127, 956655, 956930, 957014, 1502724,
        956654, 1500982, 954410, 953011, 958227, 959918
    };

    // Night Elf Illidari (Alliance)
    constexpr uint32 ALLIED_ALLIANCE_ILLIDARI[] =
    {
        957599, 956420, 959045, 956650, 953759, 956653,
        954705, 958228, 958290, 958292
    };

    constexpr uint32 BOSS_ENTRIES[] = { 953105, 956159, 151002 };
}

#endif
