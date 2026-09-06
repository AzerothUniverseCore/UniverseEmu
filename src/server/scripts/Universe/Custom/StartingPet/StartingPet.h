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

#ifndef _STARTINGPET_H_
#define _STARTINGPET_H_

#include "Common.h"
#include "Player.h"
#include "Pet.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "SharedDefines.h"

class StartingPet
{
public:
    static StartingPet* instance();

    void CreateRandomPet(Player* player, bool petName);
    std::string RandName();
    void LearnPetSpells(Player* player);
    void SummonWarlockMinion(Player* player, uint32 minionMask);
    void LearnWarlockSpells(Player* player, uint32 minionMask);
    void LearnSpellAndRanksForLevel(uint32 spellId, Player* player);

private:
    std::string const _vowels = "aeiou";
    std::string const _consonants = "bcdfghjklmnpqrstvw";
    uint32 const _pets[205] = {1513, 1554, 4861, 10357, 1186, 17345, 17348, 2354, 1130, 1815, 14280, 17661, 26643, 17053, 17034, 27244, 20109, 7099, 23051, 14343, 19429, 26369, 28004, 29044, 23945, 24747, 32481, 119, 345, 3098, 547, 16117, 2966, 4512, 4511, 4514, 7376, 1109, 5429, 2829, 2931,
                               21470, 18707, 5983, 21515, 16973, 977, 18982, 21723, 24064, 17203, 2731, 682, 3619, 7430, 29327, 21724, 10200, 2175, 7434, 28001, 28297, 7431, 16348, 15651, 28097, 5807, 14222, 3475, 28404, 26521, 830, 23929, 24478, 4344, 17952, 20773, 1693, 25867, 15650, 15649,
                               18155, 20502, 28096, 1559, 28098, 9622, 6585, 4690, 5985, 4534, 5427, 12418, 18468, 18469, 20611, 25464, 25498, 18131, 20196, 18880, 21901, 23873, 730, 1022, 29334, 1019, 3632, 3132, 2561, 20729, 20634, 21124, 20751, 17199, 19189, 16900, 16932, 17199, 19350, 16934,
                               19349, 16933, 7022, 9691, 11736, 3252, 11737, 4140, 5423, 3125, 5048, 5225, 28011, 5755, 7268, 1505, 5856, 4379, 217, 15976, 22044, 15974, 20682, 22132, 28342, 17683, 17522, 32475, 1986, 14266, 5857, 442, 11738, 18128, 18280, 18129, 2955, 2956, 3068, 22807, 2172,
                               3246, 3653, 4824, 4397, 2505, 25482, 18884, 18878, 22255, 23219, 28086, 28085, 22182, 20198, 20197, 18283, 5056, 5756, 5307, 5708, 26806, 8336, 26446, 21123, 20673, 20749, 28477, 20330, 1817, 1508, 2958, 10221, 19458, 8211, 118, 29889, 2753, 521, 24475, 24516,
                               24517, 24677, 17669};
    std::vector<uint32> const _hunterPetSpells = {136, 883, 982, 1002, 1462, 1515, 2641, 6991};
    std::vector<uint32> const _warlockSpells = {688, 697, 712, 691, 30146, 1120};
};

#define sStartingPet StartingPet::instance()

#endif
