/*
 * This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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
#ifndef __BATTLEGROUNDTV_H
#define __BATTLEGROUNDTV_H

#include "Arena.h"
#include "EventMap.h"

enum BattlegroundTVObjectTypes
{
    BG_TV_OBJECT_DOOR_1         = 0,
    BG_TV_OBJECT_DOOR_2         = 1,
    BG_TV_OBJECT_BUFF_1         = 2,
    BG_TV_OBJECT_BUFF_2         = 3,
    BG_TV_OBJECT_MAX            = 4
};

enum BattlegroundTVGameObjects
{
    BG_TV_OBJECT_TYPE_DOOR_1    = 213196,
    BG_TV_OBJECT_TYPE_DOOR_2    = 213197,
    BG_TV_OBJECT_TYPE_BUFF_1    = 184663,
    BG_TV_OBJECT_TYPE_BUFF_2    = 184664
};

inline constexpr Seconds BG_TV_REMOVE_DOORS_TIMER    = 5s;

enum BattlegroundTVEvents
{
    BG_TV_EVENT_REMOVE_DOORS    = 1
};

class BattlegroundTV : public Arena
{
    public:
        BattlegroundTV();

        /* inherited from BattlegroundClass */
        void StartingEventCloseDoors() override;
        void StartingEventOpenDoors() override;

        void HandleAreaTrigger(Player* Source, uint32 Trigger) override;
        bool SetupBattleground() override;
        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

    private:
        void PostUpdateImpl(uint32 diff) override;

        EventMap _events;
};
#endif
