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

#ifndef __LEGION_SHIP_INTRO_H
#define __LEGION_SHIP_INTRO_H

#include "Define.h"

namespace LegionShipIntro
{
    constexpr uint32 MAP_ID = 781;

    struct Point { float x, y, z, o; };

    constexpr uint32 ILLIDAN = 7007719;
    constexpr uint32 ILLIDARI_STARR = 1173924;
    constexpr uint32 ILLIDARI_KILBRIDE = 1173927;

    constexpr uint32 QUEST_ID = 900020;

    constexpr uint32 PATH_SIZE = 30;

    constexpr uint32 TELEPORT_AFTER_POINT = 4;

    constexpr uint32 POST_TOUR_DESPAWN_DELAY_MS = 30000;

    constexpr Point STARR_SPAWN = { -11796.1f, 2544.6f, 2795.65f, 1.6386f };
    constexpr Point STARR_TELEPORT_DESTINATION = { -11762.7f, 2614.63f, 2795.6f, 1.20333f };

    constexpr Point STARR_PATH[PATH_SIZE] =
    {
        { -11794.6f, 2548.53f, 2795.66f, 1.12872f  }, // 1
        { -11790.4f, 2556.97f, 2795.66f, 1.12086f  }, // 2
        { -11783.1f, 2566.84f, 2795.66f, 1.18369f  }, // 3
        { -11776.3f, 2581.76f, 2795.66f, 1.20726f  }, // 4 -> jump to STARR_TELEPORT_DESTINATION
        { -11758.4f, 2623.5f,  2796.88f, 1.20333f  }, // 5
        { -11752.4f, 2639.81f, 2796.88f, 1.38994f  }, // 6
        { -11750.2f, 2648.49f, 2796.46f, 1.56665f  }, // 7
        { -11750.1f, 2660.39f, 2792.74f, 1.56665f  }, // 8
        { -11750.4f, 2682.93f, 2781.94f, 1.58629f  }, // 9
        { -11750.7f, 2704.62f, 2771.4f,  1.58629f  }, // 10
        { -11751.9f, 2725.57f, 2761.18f, 1.65305f  }, // 11
        { -11753.6f, 2736.07f, 2756.44f, 1.77086f  }, // 12
        { -11758.3f, 2749.07f, 2750.67f, 2.12036f  }, // 13
        { -11763.9f, 2756.75f, 2748.15f, 2.32456f  }, // 14
        { -11773.4f, 2765.88f, 2745.95f, 2.37954f  }, // 15
        { -11782.7f, 2775.15f, 2745.52f, 2.34027f  }, // 16
        { -11791.7f, 2785.68f, 2745.52f, 2.21853f  }, // 17
        { -11800.9f, 2796.02f, 2745.52f, 1.5588f   }, // 18
        { -11800.6f, 2816.32f, 2745.52f, 1.5588f   }, // 19
        { -11796.3f, 2826.86f, 2745.52f, 1.5588f   }, // 20
        { -11795.0f, 2841.62f, 2745.52f, 1.5588f   }, // 21
        { -11795.5f, 2853.1f,  2745.52f, 1.5588f   }, // 22
        { -11795.3f, 2870.46f, 2745.52f, 1.5588f   }, // 23
        { -11796.5f, 2882.96f, 2745.51f, 1.5588f   }, // 24
        { -11796.3f, 2897.8f,  2745.51f, 1.5588f   }, // 25
        { -11795.4f, 2910.93f, 2745.51f, 1.47633f  }, // 26
        { -11794.1f, 2924.3f,  2745.98f, 1.58236f  }, // 27
        { -11794.6f, 2934.49f, 2745.98f, 1.7041f   }, // 28
        { -11796.6f, 2944.91f, 2745.98f, 1.90045f  }, // 29
        { -11797.8f, 2948.18f, 2745.98f, 2.04182f  }  // 30
    };

    constexpr Point KILBRIDE_SPAWN = { -11805.1f, 2544.51f, 2795.65f, 1.12809f };
    constexpr Point KILBRIDE_TELEPORT_DESTINATION = { -11837.6f, 2613.48f, 2795.46f, 2.04701f };

    constexpr Point KILBRIDE_PATH[PATH_SIZE] =
    {
        { -11803.8f, 2548.59f, 2795.65f, 1.5797f   }, // 1
        { -11804.8f, 2556.3f,  2795.65f, 2.03523f  }, // 2
        { -11810.7f, 2567.13f, 2795.65f, 2.09413f  }, // 3
        { -11820.9f, 2580.75f, 2795.65f, 1.99989f  }, // 4 -> jump to KILBRIDE_TELEPORT_DESTINATION
        { -11840.6f, 2619.76f, 2796.45f, 2.01559f  }, // 5
        { -11845.4f, 2630.65f, 2796.88f, 1.93705f  }, // 6
        { -11850.0f, 2644.88f, 2796.88f, 1.81924f  }, // 7
        { -11851.2f, 2657.22f, 2793.96f, 1.56792f  }, // 8
        { -11851.0f, 2673.59f, 2786.45f, 1.55614f  }, // 9
        { -11850.8f, 2684.93f, 2780.95f, 1.55614f  }, // 10
        { -11850.6f, 2700.6f,  2773.31f, 1.55614f  }, // 11
        { -11850.3f, 2716.57f, 2765.54f, 1.51294f  }, // 12
        { -11848.9f, 2729.93f, 2759.3f,  1.40691f  }, // 13
        { -11846.1f, 2743.64f, 2752.86f, 1.16736f  }, // 14
        { -11840.2f, 2753.35f, 2749.15f, 0.883678f }, // 15
        { -11831.8f, 2761.65f, 2746.76f, 0.812992f }, // 16
        { -11824.0f, 2770.12f, 2745.52f, 0.844408f }, // 17
        { -11816.4f, 2778.04f, 2745.52f, 0.730525f }, // 18
        { -11806.7f, 2785.83f, 2745.52f, 1.49629f  }, // 19
        { -11801.3f, 2795.65f, 2745.52f, 1.55519f  }, // 20
        { -11803.6f, 2807.09f, 2745.52f, 1.88506f  }, // 21
        { -11806.5f, 2819.01f, 2745.52f, 1.60169f  }, // 22
        { -11807.1f, 2840.0f,  2745.52f, 1.60169f  }, // 23
        { -11807.8f, 2863.51f, 2745.52f, 1.60169f  }, // 24
        { -11806.7f, 2877.19f, 2745.52f, 1.57027f  }, // 25
        { -11804.8f, 2897.64f, 2745.52f, 1.42105f  }, // 26
        { -11802.1f, 2911.39f, 2745.95f, 1.5742f   }, // 27
        { -11807.0f, 2924.17f, 2745.98f, 1.54278f  }, // 28
        { -11810.5f, 2930.24f, 2745.98f, 1.08333f  }, // 29
        { -11802.4f, 2949.87f, 2745.98f, 1.1226f   }  // 30
    };

    enum TextGroup : uint8
    {
        SAY_START        = 0, // said once, before the first step
        SAY_PRE_JUMP     = 1, // said on reaching point 4, just before the jump
        SAY_POST_JUMP    = 2, // said right after the jump
        SAY_MIDWAY       = 3, // said on reaching point 15
        SAY_NEAR_END     = 4, // said on reaching point 24
        SAY_END          = 5  // said on reaching point 30, quest completes
    };

    constexpr uint8 NO_TEXT = 0xFF;

    inline uint8 GetMilestoneTextGroup(uint32 reachedIndex)
    {
        switch (reachedIndex)
        {
            case 4:  return SAY_PRE_JUMP;
            case 15: return SAY_MIDWAY;
            case 24: return SAY_NEAR_END;
            default: return NO_TEXT;
        }
    }

    inline Point const* GetPath(uint32 entry)
    {
        if (entry == ILLIDARI_STARR)
            return STARR_PATH;
        if (entry == ILLIDARI_KILBRIDE)
            return KILBRIDE_PATH;
        return nullptr;
    }

    inline Point const& GetSpawnPoint(uint32 entry)
    {
        return entry == ILLIDARI_KILBRIDE ? KILBRIDE_SPAWN : STARR_SPAWN;
    }

    inline Point const& GetTeleportDestination(uint32 entry)
    {
        return entry == ILLIDARI_KILBRIDE ? KILBRIDE_TELEPORT_DESTINATION : STARR_TELEPORT_DESTINATION;
    }
}

#endif
