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

#ifndef __CONTINENT_LOCKDOWN_H
#define __CONTINENT_LOCKDOWN_H

#include "Define.h"
#include "Position.h"
#include "Random.h"

namespace ContinentLockdown
{
    constexpr uint32 LOCKED_MAPS[] = { 0, 1, 530, 571, 754 };

    inline bool IsLockedMap(uint32 mapId)
    {
        for (uint32 locked : LOCKED_MAPS)
            if (locked == mapId)
                return true;
        return false;
    }

    inline WorldLocation const& PickRandomDestination()
    {
        static WorldLocation const destinations[] =
        {
            WorldLocation(781, -11908.8f, 2961.1f, 1857.4f, 5.04f),
            WorldLocation(792, 1658.18f, 1573.7f, 5.84094f, 2.46316f)
        };

        return destinations[urand(0, 1)];
    }

    inline WorldLocation const& GetSubLevel80Destination()
    {
        static WorldLocation const destination(781, -11800.7f, 2555.49f, 2795.65f, 4.69982f);
        return destination;
    }

    inline WorldLocation const& PickDestination(uint8 level)
    {
        if (level < 80)
            return GetSubLevel80Destination();

        return PickRandomDestination();
    }
}

#endif
