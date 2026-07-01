/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
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

/*
This placeholder for the instance is needed for dungeon finding to be able
to give credit after the boss defined in lastEncounterDungeon is killed.
Without it, the party doing random dungeon won't get satchel of spoils and
gets instead the deserter debuff.
*/

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "GameObject.h"
#include "siegeofniuzaotemple.h"

DoorData const doorData[] =
{
    {GO_TEMPLE_INVIS_DOOR,          DATA_GENERAL_PAVALAK,       DOOR_TYPE_PASSAGE },
    {GO_FIRE_WALL,                  DATA_SIKTHIK_WARDEN,        DOOR_TYPE_PASSAGE },
    {GO_DOOR,                       DATA_SIKTHIK_WARDEN,        DOOR_TYPE_PASSAGE },
};

class instance_siegeofniuzaotemple : public InstanceMapScript
{
public:
    instance_siegeofniuzaotemple() : InstanceMapScript(SNTScriptName, 778) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_siegeofniuzaotemple_InstanceMapScript(map);
    }

    struct instance_siegeofniuzaotemple_InstanceMapScript : public InstanceScript
    {
        instance_siegeofniuzaotemple_InstanceMapScript(InstanceMap* map) : InstanceScript(map) 
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
            case GO_TEMPLE_INVIS_DOOR:
                invisDoorGUID[invisDoorGUID[0] != 0] = go->GetGUID();
                break;
            case GO_FIRE_WALL:
                wardenDoor = go->GetGUID();
                break;
            case GO_DOOR:
                wardenDoorGUID = go->GetGUID();
                break;
            case GO_HARDENED_RESIN:
                jinbakDoorGUID = go->GetGUID();

                // SetBossState couldn`t work because GO possible load after load data.
                if (GetBossState(DATA_VIZIER_JINBAK) == DONE)
                    go->RemoveFlag(GO_FLAG_NOT_SELECTABLE);
                break;
            default:
                break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
            case DATA_VIZIER_JINBAK:
                if (state == DONE)
                    if (GameObject* go = instance->GetGameObject(jinbakDoorGUID))
                        go->RemoveFlag(GO_FLAG_NOT_SELECTABLE);
                break;
            default:
                break;
            }

            return true;
        }

    private:
        ObjectGuid wardenDoor;
        ObjectGuid wardenDoorGUID;
        ObjectGuid jinbakDoorGUID;
        ObjectGuid invisDoorGUID[2];
    };
};

void AddSC_instance_siegeofniuzaotemple()
{
    new instance_siegeofniuzaotemple();
}
