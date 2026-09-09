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

#include "QuestFeasibility.h"
#include "QuestDef.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include <unordered_set>

namespace
{
    std::unordered_set<uint32> const& GetSpawnedCreatureEntries()
    {
        static std::unordered_set<uint32> entries;
        static bool built = false;
        if (!built)
        {
            for (auto const& pair : sObjectMgr->GetAllCreatureData())
                entries.insert(pair.second.id);
            built = true;
        }
        return entries;
    }

    std::unordered_set<uint32> const& GetSpawnedGameObjectEntries()
    {
        static std::unordered_set<uint32> entries;
        static bool built = false;
        if (!built)
        {
            for (auto const& pair : sObjectMgr->GetAllGameObjectData())
                entries.insert(pair.second.id);
            built = true;
        }
        return entries;
    }
}

bool QuestFeasibility::IsQuestUnreachable(Quest const* quest, UnreachableInfo* outInfo)
{
    if (outInfo)
        *outInfo = UnreachableInfo();

    if (!quest)
        return false;

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 reqEntry = quest->RequiredNpcOrGo[i];
        if (reqEntry == 0)
            continue;

        if (reqEntry > 0)
        {
            uint32 creatureEntry = uint32(reqEntry);

            if (!sObjectMgr->GetCreatureTemplate(creatureEntry))
            {
                if (outInfo)
                {
                    outInfo->type = MissingObjectiveType::Creature;
                    outInfo->entry = creatureEntry;
                    outInfo->reason = MissingReason::TemplateMissing;
                }
                return true;
            }

            if (GetSpawnedCreatureEntries().count(creatureEntry) == 0)
            {
                if (outInfo)
                {
                    outInfo->type = MissingObjectiveType::Creature;
                    outInfo->entry = creatureEntry;
                    outInfo->reason = MissingReason::NotSpawned;
                }
                return true;
            }
        }
        else
        {
            uint32 goEntry = uint32(-reqEntry);

            if (!sObjectMgr->GetGameObjectTemplate(goEntry))
            {
                if (outInfo)
                {
                    outInfo->type = MissingObjectiveType::GameObject;
                    outInfo->entry = goEntry;
                    outInfo->reason = MissingReason::TemplateMissing;
                }
                return true;
            }

            if (GetSpawnedGameObjectEntries().count(goEntry) == 0)
            {
                if (outInfo)
                {
                    outInfo->type = MissingObjectiveType::GameObject;
                    outInfo->entry = goEntry;
                    outInfo->reason = MissingReason::NotSpawned;
                }
                return true;
            }
        }
    }

    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        uint32 itemId = quest->RequiredItemId[i];
        if (itemId == 0)
            continue;

        if (!sObjectMgr->GetItemTemplate(itemId))
        {
            if (outInfo)
            {
                outInfo->type = MissingObjectiveType::Item;
                outInfo->entry = itemId;
                outInfo->reason = MissingReason::TemplateMissing;
            }
            return true;
        }
    }

    return false;
}

void QuestFeasibility::LogBypass(Quest const* quest, UnreachableInfo const& info, uint32 playerGuidLow)
{
    if (!quest || info.type == MissingObjectiveType::None)
        return;

    char const* typeStr = "creature";
    if (info.type == MissingObjectiveType::GameObject)
        typeStr = "gameobject";
    else if (info.type == MissingObjectiveType::Item)
        typeStr = "item";

    char const* reasonStr = (info.reason == MissingReason::NotSpawned) ? "no_spawn" : "template_missing";

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_QUEST_BYPASS_LOG);
    stmt->setUInt32(0, quest->GetQuestId());
    stmt->setString(1, quest->GetTitle());
    stmt->setString(2, typeStr);
    stmt->setUInt32(3, info.entry);
    stmt->setString(4, reasonStr);
    stmt->setUInt32(5, playerGuidLow);
    WorldDatabase.Execute(stmt);
}
