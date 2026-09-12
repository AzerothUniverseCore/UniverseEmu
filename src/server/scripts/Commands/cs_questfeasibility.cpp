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
 
// .debug questfeasibility check
// .debug questfeasibility autofix [count]
// .debug questfeasibility autofix 20

#include "ScriptMgr.h"
#include "Chat.h"
#include "Creature.h"
#include "CreatureData.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "QuestDef.h"
#include "RBAC.h"
#include "World.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace Syphrena::ChatCommands;

namespace
{
    struct ReferenceSpot
    {
        bool     found = false;
        uint32   mapId = 0;
        float    x = 0.0f, y = 0.0f, z = 0.0f, o = 0.0f;
    };

    struct QuestLinks
    {
        std::unordered_map<uint32, std::vector<uint32>> creatureGivers;
        std::unordered_map<uint32, std::vector<uint32>> gameobjectGivers;
    };

    QuestLinks LoadQuestLinks()
    {
        QuestLinks links;

        if (QueryResult result = WorldDatabase.Query("SELECT id, quest FROM creature_queststarter"))
        {
            do
            {
                Field* f = result->Fetch();
                links.creatureGivers[f[1].GetUInt32()].push_back(f[0].GetUInt32());
            } while (result->NextRow());
        }
        if (QueryResult result = WorldDatabase.Query("SELECT id, quest FROM creature_questender"))
        {
            do
            {
                Field* f = result->Fetch();
                links.creatureGivers[f[1].GetUInt32()].push_back(f[0].GetUInt32());
            } while (result->NextRow());
        }
        if (QueryResult result = WorldDatabase.Query("SELECT id, quest FROM gameobject_queststarter"))
        {
            do
            {
                Field* f = result->Fetch();
                links.gameobjectGivers[f[1].GetUInt32()].push_back(f[0].GetUInt32());
            } while (result->NextRow());
        }
        if (QueryResult result = WorldDatabase.Query("SELECT id, quest FROM gameobject_questender"))
        {
            do
            {
                Field* f = result->Fetch();
                links.gameobjectGivers[f[1].GetUInt32()].push_back(f[0].GetUInt32());
            } while (result->NextRow());
        }

        return links;
    }

    std::unordered_set<uint32> BuildSpawnedCreatureEntries()
    {
        std::unordered_set<uint32> entries;
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
            entries.insert(pair.second.id);
        return entries;
    }

    std::unordered_set<uint32> BuildSpawnedGameObjectEntries()
    {
        std::unordered_set<uint32> entries;
        for (auto const& pair : sObjectMgr->GetAllGameObjectData())
            entries.insert(pair.second.id);
        return entries;
    }

    ReferenceSpot FindReferenceSpot(uint32 questId, QuestLinks const& links)
    {
        ReferenceSpot spot;

        auto tryCreatureList = [&](std::vector<uint32> const& list) -> bool
        {
            for (uint32 entry : list)
            {
                for (auto const& pair : sObjectMgr->GetAllCreatureData())
                {
                    if (pair.second.id == entry)
                    {
                        spot.found = true;
                        spot.mapId = pair.second.mapId;
                        spot.x = pair.second.spawnPoint.GetPositionX();
                        spot.y = pair.second.spawnPoint.GetPositionY();
                        spot.z = pair.second.spawnPoint.GetPositionZ();
                        spot.o = pair.second.spawnPoint.GetOrientation();
                        return true;
                    }
                }
            }
            return false;
        };

        auto tryGameObjectList = [&](std::vector<uint32> const& list) -> bool
        {
            for (uint32 entry : list)
            {
                for (auto const& pair : sObjectMgr->GetAllGameObjectData())
                {
                    if (pair.second.id == entry)
                    {
                        spot.found = true;
                        spot.mapId = pair.second.mapId;
                        spot.x = pair.second.spawnPoint.GetPositionX();
                        spot.y = pair.second.spawnPoint.GetPositionY();
                        spot.z = pair.second.spawnPoint.GetPositionZ();
                        spot.o = pair.second.spawnPoint.GetOrientation();
                        return true;
                    }
                }
            }
            return false;
        };

        auto itC = links.creatureGivers.find(questId);
        if (itC != links.creatureGivers.end() && tryCreatureList(itC->second))
            return spot;

        auto itG = links.gameobjectGivers.find(questId);
        if (itG != links.gameobjectGivers.end() && tryGameObjectList(itG->second))
            return spot;

        return spot;
    }

    ReferenceSpot FindReferenceSpotForEntry(std::vector<uint32> const& questIds, QuestLinks const& links)
    {
        for (uint32 questId : questIds)
        {
            ReferenceSpot spot = FindReferenceSpot(questId, links);
            if (spot.found)
                return spot;
        }
        return ReferenceSpot();
    }

    enum class IssueKind : uint8 { CreatureMissingTemplate, CreatureNeverSpawned, GameObjectMissingTemplate, GameObjectNeverSpawned, ItemMissingTemplate };

    struct Issue
    {
        uint32   questId;
        std::string questTitle;
        IssueKind kind;
        uint32   entry;
    };

    struct UniqueIssue
    {
        IssueKind kind;
        uint32    entry;
        std::vector<uint32> questIds;
    };

    std::vector<UniqueIssue> DeduplicateNeverSpawnedIssues(std::vector<Issue> const& issues)
    {
        std::vector<UniqueIssue> result;
        std::unordered_map<uint64, size_t> index;

        for (Issue const& issue : issues)
        {
            if (issue.kind != IssueKind::CreatureNeverSpawned && issue.kind != IssueKind::GameObjectNeverSpawned)
                continue;

            uint64 key = (uint64(issue.kind) << 32) | issue.entry;
            auto it = index.find(key);
            if (it == index.end())
            {
                index.emplace(key, result.size());
                result.push_back({ issue.kind, issue.entry, { issue.questId } });
            }
            else
                result[it->second].questIds.push_back(issue.questId);
        }

        return result;
    }

    std::vector<Issue> ScanAllQuests()
    {
        std::vector<Issue> issues;
        std::unordered_set<uint32> spawnedCreatures = BuildSpawnedCreatureEntries();
        std::unordered_set<uint32> spawnedGameObjects = BuildSpawnedGameObjectEntries();

        for (auto const& pair : sObjectMgr->GetQuestTemplates())
        {
            Quest const* quest = pair.second.get();
            if (!quest)
                continue;

            for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                int32 req = quest->RequiredNpcOrGo[i];
                if (req == 0)
                    continue;

                if (req > 0)
                {
                    uint32 entry = uint32(req);
                    if (!sObjectMgr->GetCreatureTemplate(entry))
                        issues.push_back({ quest->GetQuestId(), quest->GetTitle(), IssueKind::CreatureMissingTemplate, entry });
                    else if (spawnedCreatures.count(entry) == 0)
                        issues.push_back({ quest->GetQuestId(), quest->GetTitle(), IssueKind::CreatureNeverSpawned, entry });
                }
                else
                {
                    uint32 entry = uint32(-req);
                    if (!sObjectMgr->GetGameObjectTemplate(entry))
                        issues.push_back({ quest->GetQuestId(), quest->GetTitle(), IssueKind::GameObjectMissingTemplate, entry });
                    else if (spawnedGameObjects.count(entry) == 0)
                        issues.push_back({ quest->GetQuestId(), quest->GetTitle(), IssueKind::GameObjectNeverSpawned, entry });
                }
            }

            for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
            {
                uint32 itemId = quest->RequiredItemId[i];
                if (itemId && !sObjectMgr->GetItemTemplate(itemId))
                    issues.push_back({ quest->GetQuestId(), quest->GetTitle(), IssueKind::ItemMissingTemplate, itemId });
            }
        }

        return issues;
    }

    char const* IssueKindLabel(IssueKind kind)
    {
        switch (kind)
        {
            case IssueKind::CreatureMissingTemplate:   return "creature manquant (pas de creature_template)";
            case IssueKind::CreatureNeverSpawned:      return "creature jamais spawn";
            case IssueKind::GameObjectMissingTemplate: return "gameobject manquant (pas de gameobject_template)";
            case IssueKind::GameObjectNeverSpawned:    return "gameobject jamais spawn";
            case IssueKind::ItemMissingTemplate:       return "item manquant (pas de item_template)";
            default:                                   return "inconnu";
        }
    }
}

class questfeasibility_commandscript : public CommandScript
{
public:
    questfeasibility_commandscript() : CommandScript("questfeasibility_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable questFeasibilityCommandTable =
        {
            { "check",   HandleQuestFeasibilityCheckCommand,   rbac::RBAC_PERM_COMMAND_DEBUG, Console::Yes },
            { "autofix", HandleQuestFeasibilityAutofixCommand, rbac::RBAC_PERM_COMMAND_DEBUG, Console::Yes },
        };
        static ChatCommandTable debugCommandTable =
        {
            { "questfeasibility", questFeasibilityCommandTable },
        };
        static ChatCommandTable commandTable =
        {
            { "debug", debugCommandTable },
        };
        return commandTable;
    }

    static bool HandleQuestFeasibilityCheckCommand(ChatHandler* handler)
    {
        std::vector<Issue> issues = ScanAllQuests();

        uint32 missingTemplate = 0, neverSpawned = 0;
        for (Issue const& issue : issues)
        {
            SC_LOG_INFO("quest.feasibility", "Quete {} \"{}\" : {} (entry {})",
                issue.questId, issue.questTitle, IssueKindLabel(issue.kind), issue.entry);

            if (issue.kind == IssueKind::CreatureMissingTemplate || issue.kind == IssueKind::GameObjectMissingTemplate || issue.kind == IssueKind::ItemMissingTemplate)
                ++missingTemplate;
            else
                ++neverSpawned;
        }

        handler->PSendSysMessage("Scan termine : %u probleme(s) trouve(s) sur %u quete(s) chargee(s) (%u entry manquant, %u jamais spawn). Detail dans le logger 'quest.feasibility'.",
            uint32(issues.size()), uint32(sObjectMgr->GetQuestTemplates().size()), missingTemplate, neverSpawned);

        return true;
    }

    static bool HandleQuestFeasibilityAutofixCommand(ChatHandler* handler, Optional<uint32> count)
    {
        uint32 const limit = count.value_or(20);

        std::vector<Issue> issues = ScanAllQuests();
        std::vector<UniqueIssue> uniqueIssues = DeduplicateNeverSpawnedIssues(issues);
        QuestLinks links = LoadQuestLinks();

        uint32 created = 0, skippedInstanceBound = 0, skippedNoReference = 0, skippedInstanceMap = 0;

        for (UniqueIssue const& issue : uniqueIssues)
        {
            if (created >= limit)
                break;

            uint32 refQuestId = issue.questIds.front();

            if (issue.kind == IssueKind::CreatureNeverSpawned)
            {
                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(issue.entry);
                if (!cInfo)
                    continue;

                if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                {
                    SC_LOG_INFO("quest.feasibility", "Autofix: creature {} (\"{}\") ignoree, liee a une instance (CREATURE_FLAG_EXTRA_INSTANCE_BIND) - placement manuel requis ({} quete(s) concernee(s)).", issue.entry, cInfo->Name, uint32(issue.questIds.size()));
                    ++skippedInstanceBound;
                    continue;
                }

                ReferenceSpot spot = FindReferenceSpotForEntry(issue.questIds, links);
                if (!spot.found)
                {
                    SC_LOG_INFO("quest.feasibility", "Autofix: creature {} ignoree, aucun donneur/rendeur deja spawn trouve comme reference parmi {} quete(s).", issue.entry, uint32(issue.questIds.size()));
                    ++skippedNoReference;
                    continue;
                }

                Map* map = sMapMgr->CreateBaseMap(spot.mapId);
                if (!map || map->Instanceable())
                {
                    ++skippedInstanceMap;
                    continue;
                }

                float z = map->GetHeight(spot.x, spot.y, spot.z + 5.0f, true);
                if (z <= INVALID_HEIGHT)
                    z = spot.z;

                Position pos(spot.x, spot.y, z, spot.o);

                Creature* creature = new Creature();
                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, issue.entry, pos))
                {
                    delete creature;
                    continue;
                }

                creature->SaveToDB(map->GetId(), 1 << map->GetSpawnMode(), PHASEMASK_NORMAL);
                ObjectGuid::LowType dbGuid = creature->GetSpawnId();

                creature->CleanupsBeforeDelete();
                delete creature;
                creature = new Creature();
                if (creature->LoadFromDB(dbGuid, map, true, true))
                    sObjectMgr->AddCreatureToGrid(dbGuid, sObjectMgr->GetCreatureData(dbGuid));
                else
                    delete creature;

                SC_LOG_INFO("quest.feasibility", "Autofix: creature {} (\"{}\") spawn cree (spawnId {}) map {} pos ({}, {}, {}) - repare {} quete(s) (ex: {}).",
                    issue.entry, cInfo->Name, dbGuid, spot.mapId, spot.x, spot.y, z, uint32(issue.questIds.size()), refQuestId);
                ++created;
            }
            else
            {
                GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(issue.entry);
                if (!goInfo)
                    continue;

                ReferenceSpot spot = FindReferenceSpotForEntry(issue.questIds, links);
                if (!spot.found)
                {
                    SC_LOG_INFO("quest.feasibility", "Autofix: gameobject {} ignore, aucun donneur/rendeur deja spawn trouve comme reference parmi {} quete(s).", issue.entry, uint32(issue.questIds.size()));
                    ++skippedNoReference;
                    continue;
                }

                Map* map = sMapMgr->CreateBaseMap(spot.mapId);
                if (!map || map->Instanceable())
                {
                    ++skippedInstanceMap;
                    continue;
                }

                float z = map->GetHeight(spot.x, spot.y, spot.z + 5.0f, true);
                if (z <= INVALID_HEIGHT)
                    z = spot.z;

                Position pos(spot.x, spot.y, z, spot.o);
                QuaternionData rot = QuaternionData::fromEulerAnglesZYX(spot.o, 0.0f, 0.0f);

                GameObject* go = new GameObject();
                if (!go->Create(map->GenerateLowGuid<HighGuid::GameObject>(), issue.entry, map, PHASEMASK_NORMAL, pos, rot, 100, GO_STATE_READY))
                {
                    delete go;
                    continue;
                }

                go->SaveToDB(map->GetId(), 1 << map->GetSpawnMode(), PHASEMASK_NORMAL);
                ObjectGuid::LowType dbGuid = go->GetSpawnId();
                delete go;

                go = new GameObject();
                if (go->LoadFromDB(dbGuid, map, true))
                    sObjectMgr->AddGameobjectToGrid(dbGuid, sObjectMgr->GetGameObjectData(dbGuid));
                else
                    delete go;

                SC_LOG_INFO("quest.feasibility", "Autofix: gameobject {} (\"{}\") spawn cree (spawnId {}) map {} pos ({}, {}, {}) - repare {} quete(s) (ex: {}).",
                    issue.entry, goInfo->name, dbGuid, spot.mapId, spot.x, spot.y, z, uint32(issue.questIds.size()), refQuestId);
                ++created;
            }
        }

        handler->PSendSysMessage("Autofix termine : %u spawn(s) cree(s), %u entry ignore(s) (boss d'instance), %u entry ignore(s) (pas de point de reference spawn), %u entry ignore(s) (carte instanciee). Limite par appel : %u. Detail dans le logger 'quest.feasibility'.",
            created, skippedInstanceBound, skippedNoReference, skippedInstanceMap, limit);

        return true;
    }
};

void AddSC_questfeasibility_commandscript()
{
    new questfeasibility_commandscript();
}
