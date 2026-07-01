/*
* Copyright (C) 2020-2021 Trickerer <https://github.com/trickerer/>
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

#ifndef _CUSTOM_DATA_H
#define _CUSTOM_DATA_H

struct CreatureCustomData
{
    uint32 instancePlayerCount = 0;
    uint8 selectedLevel = 0;
    uint32 entry = 0;
    float damageMultiplier = 1.0f;
    float healthMultiplier = 1.0f;
    float manaMultiplier = 1.0f;
    float armorMultiplier = 1.0f;

    CreatureCustomData() = default;
    CreatureCustomData(CreatureCustomData const&) = delete;
    CreatureCustomData(CreatureCustomData&&) = delete;
    CreatureCustomData& operator=(CreatureCustomData const&) = delete;
    CreatureCustomData& operator=(CreatureCustomData&&) = delete;
};

struct MapCustomData
{
    uint32 playerCount = 0;
    uint8 mapLevel = 0;

    MapCustomData() = default;
    MapCustomData(MapCustomData const&) = delete;
    MapCustomData(MapCustomData&&) = delete;
    MapCustomData& operator=(MapCustomData const&) = delete;
    MapCustomData& operator=(MapCustomData&&) = delete;
};

#endif