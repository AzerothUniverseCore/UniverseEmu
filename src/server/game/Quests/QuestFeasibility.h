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

#ifndef SYPHRENA_QUEST_FEASIBILITY_H
#define SYPHRENA_QUEST_FEASIBILITY_H

#include "Define.h"

class Quest;

namespace QuestFeasibility
{
    enum class MissingObjectiveType : uint8
    {
        None       = 0,
        Creature   = 1,
        GameObject = 2,
        Item       = 3
    };

    enum class MissingReason : uint8
    {
        None            = 0,
        TemplateMissing = 1,
        NotSpawned      = 2
    };

    struct UnreachableInfo
    {
        MissingObjectiveType type = MissingObjectiveType::None;
        uint32 entry = 0;
        MissingReason reason = MissingReason::None;
    };

    bool IsQuestUnreachable(Quest const* quest, UnreachableInfo* outInfo = nullptr);

    void LogBypass(Quest const* quest, UnreachableInfo const& info, uint32 playerGuidLow);
}

#endif
