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

#include "ContinentLevelScaling.h"
#include "Config.h"
#include "Creature.h"
#include "CreatureData.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "World.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace
{
    bool s_enabled = false;
    uint8 s_minLevel = 1;
    uint8 s_maxLevel = 90;
    uint32 s_updateInterval = 3000;
    bool s_skipInCombat = true;
    bool s_skipCivilian = true;
    bool s_skipWorldBoss = true;
    std::unordered_set<uint32> s_scaledMaps;

    void LoadMapIdsFromString(std::string const& csv)
    {
        s_scaledMaps.clear();

        std::stringstream stream;
        stream.str(csv);
        std::string token;
        while (std::getline(stream, token, ','))
        {
            if (!token.empty())
                s_scaledMaps.insert(uint32(atoi(token.c_str())));
        }
    }

    void LoadContinentLevelScalingConfig(bool /*reload*/)
    {
        s_enabled = sConfigMgr->GetBoolDefault("ContinentLevelScaling.Enable", false);
        LoadMapIdsFromString(sConfigMgr->GetStringDefault("ContinentLevelScaling.MapIds", "0,1,530,571,754,725,732,736,751,764,750,734,737,763,791,794,779,735,775,776,778,795,796,797,807,811,819,821,814,815,820"));

        s_minLevel = uint8(sConfigMgr->GetIntDefault("ContinentLevelScaling.MinLevel", 1));
        s_maxLevel = uint8(sConfigMgr->GetIntDefault("ContinentLevelScaling.MaxLevel", 90));
        s_updateInterval = uint32(sConfigMgr->GetIntDefault("ContinentLevelScaling.UpdateInterval", 3000));

        s_skipInCombat = sConfigMgr->GetBoolDefault("ContinentLevelScaling.SkipInCombat", true);
        s_skipCivilian = sConfigMgr->GetBoolDefault("ContinentLevelScaling.SkipCivilian", true);
        s_skipWorldBoss = sConfigMgr->GetBoolDefault("ContinentLevelScaling.SkipWorldBoss", true);

        if (s_minLevel < 1)
            s_minLevel = 1;
        if (s_maxLevel < s_minLevel)
            s_maxLevel = s_minLevel;
        if (s_maxLevel > MAX_LEVEL)
            s_maxLevel = uint8(MAX_LEVEL);
        if (s_updateInterval < 500)
            s_updateInterval = 500;

        SC_LOG_INFO("server.loading", ">> ContinentLevelScaling: {} ({} map(s), levels {}-{}, every {} ms).",
            s_enabled ? "enabled" : "disabled", s_scaledMaps.size(), s_minLevel, s_maxLevel, s_updateInterval);
    }

    bool IsScalableMap(uint32 mapId)
    {
        return s_scaledMaps.find(mapId) != s_scaledMaps.end();
    }

    uint8 GetHighestPlayerLevelInZone(Map const* map, uint32 zoneId)
    {
        uint8 highest = 0;

        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player const* player = itr->GetSource();
            if (!player || !player->IsInWorld() || player->IsGameMaster())
                continue;

            if (player->GetZoneId() != zoneId)
                continue;

            uint8 level = player->GetLevel();
            if (level > highest)
                highest = level;
        }

        return highest;
    }
}

void ContinentLevelScaling::OnCreatureUpdate(Creature* creature, uint32 diff)
{
    if (!s_enabled || !creature || !creature->IsAlive())
        return;

    if (creature->ContinentScalingTimer > diff)
    {
        creature->ContinentScalingTimer -= diff;
        return;
    }
    creature->ContinentScalingTimer = s_updateInterval;

    Map* map = creature->GetMap();
    if (!map || !IsScalableMap(map->GetId()))
        return;

    if (creature->IsPet() || creature->IsTotem() || creature->IsSummon())
        return;

    if (s_skipInCombat && creature->IsInCombat())
        return;

    CreatureTemplate const* cInfo = creature->GetCreatureTemplate();
    if (!cInfo)
        return;

    if (s_skipCivilian && (cInfo->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN))
        return;

    if (s_skipWorldBoss && cInfo->rank == CREATURE_ELITE_WORLDBOSS)
        return;

    if (!creature->ContinentScalingBaselineCaptured)
    {
        creature->ContinentScalingBaseLevel = creature->GetLevel();
        creature->ContinentScalingBaselineCaptured = true;
    }

    uint8 highestPlayerLevel = GetHighestPlayerLevelInZone(map, creature->GetZoneId());

    uint8 targetLevel;
    if (highestPlayerLevel == 0)
        targetLevel = creature->ContinentScalingBaseLevel;
    else
        targetLevel = std::min(s_maxLevel, std::max(s_minLevel, highestPlayerLevel));

    if (targetLevel == creature->GetLevel())
        return;

    float healthPct = creature->GetHealthPct();

    creature->SetLevel(targetLevel);
    creature->UpdateLevelDependantStats();

    creature->SetHealth(std::max<uint32>(1, uint32(creature->GetMaxHealth() * (healthPct / 100.0f))));
}

class ContinentLevelScaling_WorldScript : public WorldScript
{
public:
    ContinentLevelScaling_WorldScript() : WorldScript("ContinentLevelScaling_WorldScript") { }

    void OnConfigLoad(bool reload) override
    {
        LoadContinentLevelScalingConfig(reload);
    }
};

void AddSC_ContinentLevelScaling()
{
    new ContinentLevelScaling_WorldScript();
}

void InitContinentLevelScalingSystem()
{
    AddSC_ContinentLevelScaling();
}
