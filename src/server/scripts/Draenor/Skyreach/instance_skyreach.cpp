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

#include "AreaBoundary.h"
#include "ScriptMgr.h"
#include "Creature.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "skyreach.h"

BossBoundaryData const boundaries =
{
    { DATA_DARKWEAVER_SYTH,     new CircleBoundary(Position(1165.0f,1728.0f), 33.0) },
    { DATA_ARAKNATH,            new CircleBoundary(Position(1043.0f, 1814.0f), 36.0f) },
};

DoorData const doorData[] =
{
    { GO_SYTH_ENTER_DOOR,           DATA_DARKWEAVER_SYTH,   DOOR_TYPE_ROOM      },
    { GO_SYTH_EXIT_DOOR,            DATA_DARKWEAVER_SYTH,   DOOR_TYPE_PASSAGE   },
    { GO_ARAKNATH_ENTER_DOOR_1,     DATA_ARAKNATH,          DOOR_TYPE_ROOM      },
    { GO_ARAKNATH_ENTER_DOOR_2,     DATA_ARAKNATH,          DOOR_TYPE_ROOM      },
    { GO_ARAKNATH_EXIT_DOOR_1,      DATA_ARAKNATH,          DOOR_TYPE_PASSAGE   },
    { GO_ARAKNATH_EXIT_DOOR_2,      DATA_ARAKNATH,          DOOR_TYPE_PASSAGE   },
    { GO_IKISS_ENTER_DOOR,          DATA_TALON_KING_IKISS,  DOOR_TYPE_ROOM      },
};

class instance_skyreach : public InstanceMapScript
{
public:
    instance_skyreach() : InstanceMapScript(SHScriptName, 795) {}

    struct instance_skyreach_InstanceMapScript : public InstanceScript
    {
        instance_skyreach_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadBossBoundaries(boundaries);
            LoadDoorData(doorData);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
            case NPC_DARKWEAVER_SYTH:
                SythGUID = creature->GetGUID();
                break;
            case NPC_ARAKNATH:
                AraknathGUID = creature->GetGUID();
                break;
            case NPC_TALON_KING_IKISS:
                IkissGUID = creature->GetGUID();
                break;
            default:
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 id) const override
        {
            switch (id)
            {
            case DATA_DARKWEAVER_SYTH:
                return SythGUID;
            case DATA_ARAKNATH:
                return AraknathGUID;
            case DATA_TALON_KING_IKISS:
                return IkissGUID;
            }

            return ObjectGuid::Empty;
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            // When last encounter is Done, the NPC Reshad appear and move into room
            switch (type)
            {
                case DATA_TALON_KING_IKISS:
                    if (state == DONE)
                    {
                        if (Creature* reshad = instance->SummonCreature(NPC_RESHAD, reshadPos[0]))
                            reshad->GetMotionMaster()->MovePoint(1, reshadPos[1]);
                    }
                    break;

                default:
                    break;
            }

            return true;
        }

        protected:
            ObjectGuid SythGUID;
            ObjectGuid AraknathGUID;
            ObjectGuid IkissGUID;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_skyreach_InstanceMapScript(map);
    }
};

void AddSC_instance_skyreach()
{
    new instance_skyreach();
}
