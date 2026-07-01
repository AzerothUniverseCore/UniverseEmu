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

#ifndef SKYREACH_H_
#define SKYREACH_H_

#include "CreatureAIImpl.h"

#define SHScriptName "instance_skyreach"
#define DataHeader "SH"

uint32 const EncounterCount             = 3;

enum SHDataTypes
{
    // Encounter States/Boss GUIDs
    DATA_DARKWEAVER_SYTH                = 0,
    DATA_ARAKNATH                       = 1,
    DATA_TALON_KING_IKISS               = 2,
};

enum SRGameObjectIds
{
    GO_SYTH_ENTER_DOOR          = 234311,
    GO_SYTH_EXIT_DOOR           = 234310,
    GO_ARAKNATH_EXIT_DOOR_1     = 234312,
    GO_ARAKNATH_EXIT_DOOR_2     = 234313,
    GO_ARAKNATH_ENTER_DOOR_1    = 234314,
    GO_ARAKNATH_ENTER_DOOR_2    = 234315,
    GO_IKISS_ENTER_DOOR         = 235994,
};

enum SkyreachCreaturesIds
{
    NPC_DARKWEAVER_SYTH                 = 184720,
    NPC_ARAKNATH                        = 194285,
    NPC_TALON_KING_IKISS                = 184730,

    // NPC for Araknath encounter
    NPC_SKYREACH_ARCANOLOGIST           = 76376,
    NPC_SUN_CONSTRUCT_ENERGIZER         = 76367,
    NPC_SKYREACH_SUN_PROTOTYPE          = 76142,
    NPC_INTERIOR_FOCUS                  = 77543,

    // Other
    NPC_RESHAD                          = 84782, // For leave dungeon, spawn after last encounter kill
};

// Define appear/move position XYZ for NPC_RESHAD
Position const reshadPos[2] =
{
    {1219.41f, 1849.78f, 291.05f},
    {1124.76f, 1802.27f, 262.17f},
};

template <class AI, class T>
inline AI* GetSkyreachAI(T* obj)
{
    return GetInstanceAI<AI>(obj, SHScriptName);
}

#define RegisterSkyreachCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetSkyreachAI)

#endif