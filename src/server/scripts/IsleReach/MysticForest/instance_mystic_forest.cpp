/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

class instance_mystic_forest : public InstanceMapScript
{
public:
    instance_mystic_forest() : InstanceMapScript("instance_mystic_forest", 825) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_mystic_forest_InstanceMapScript(map);
    }

    struct instance_mystic_forest_InstanceMapScript : public InstanceScript
    {
        instance_mystic_forest_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }
    };
};

void AddSC_instance_mystic_forest()
{
    new instance_mystic_forest();
}
