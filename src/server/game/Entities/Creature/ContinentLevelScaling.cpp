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

#include "ContinentLevelScaling.h"
#include "Config.h"
#include "Creature.h"
#include "CreatureData.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "Unit.h"
#include "World.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace
{
    bool s_enabled = false;
    uint8 s_minLevel = 1;
    uint8 s_maxLevel = 90;
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
        LoadMapIdsFromString(sConfigMgr->GetStringDefault("ContinentLevelScaling.MapIds", "0,1,530,571,754,725,732,736,751,764,765,750,734,737,763,791,794,779,735,775,776,778,795,796,797,807,811,819,821,814,815,820"));

        s_minLevel = uint8(sConfigMgr->GetIntDefault("ContinentLevelScaling.MinLevel", 1));
        s_maxLevel = uint8(sConfigMgr->GetIntDefault("ContinentLevelScaling.MaxLevel", 90));

        s_skipCivilian = sConfigMgr->GetBoolDefault("ContinentLevelScaling.SkipCivilian", true);
        s_skipWorldBoss = sConfigMgr->GetBoolDefault("ContinentLevelScaling.SkipWorldBoss", true);

        if (s_minLevel < 1)
            s_minLevel = 1;
        if (s_maxLevel < s_minLevel)
            s_maxLevel = s_minLevel;
        if (s_maxLevel > MAX_LEVEL)
            s_maxLevel = uint8(MAX_LEVEL);

        SC_LOG_INFO("server.loading", ">> ContinentLevelScaling: {} ({} map(s), levels {}-{}, on-aggro).",
            s_enabled ? "enabled" : "disabled", s_scaledMaps.size(), s_minLevel, s_maxLevel);
    }

    bool IsScalableMap(uint32 mapId)
    {
        return s_scaledMaps.find(mapId) != s_scaledMaps.end();
    }

    Player* ResolveResponsiblePlayer(Unit* unit)
    {
        if (!unit)
            return nullptr;

        if (Player* player = unit->ToPlayer())
            return player;

        return unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    uint8 GetHighestEngagedPlayerLevel(Creature* creature, Unit* target)
    {
        uint8 highest = 0;

        if (Player* player = ResolveResponsiblePlayer(target))
            highest = player->GetLevel();

        for (ThreatReference const* ref : creature->GetThreatManager().GetSortedThreatList())
        {
            if (Player* player = ResolveResponsiblePlayer(ref->GetVictim()))
                highest = std::max(highest, player->GetLevel());
        }

        return highest;
    }

    bool IsEligible(Creature* creature)
    {
        if (!s_enabled || !creature)
            return false;

        Map* map = creature->GetMap();
        if (!map || !IsScalableMap(map->GetId()))
            return false;

        if (creature->IsPet() || creature->IsTotem() || creature->IsSummon())
            return false;

        CreatureTemplate const* cInfo = creature->GetCreatureTemplate();
        if (!cInfo)
            return false;

        if (s_skipCivilian && (cInfo->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN))
            return false;

        if (s_skipWorldBoss && cInfo->rank == CREATURE_ELITE_WORLDBOSS)
            return false;

        return true;
    }

    void ApplyLevel(Creature* creature, uint8 targetLevel)
    {
        if (targetLevel == creature->GetLevel())
            return;

        float healthPct = creature->GetHealthPct();

        creature->SetLevel(targetLevel);
        creature->UpdateLevelDependantStats();

        creature->SetHealth(std::max<uint32>(1, uint32(creature->GetMaxHealth() * (healthPct / 100.0f))));
    }
}

void ContinentLevelScaling::OnCreatureEngage(Creature* creature, Unit* target)
{
    if (!IsEligible(creature))
        return;

    if (!creature->ContinentScalingBaselineCaptured)
    {
        creature->ContinentScalingBaseLevel = creature->GetLevel();
        creature->ContinentScalingBaselineCaptured = true;
    }

    uint8 highestPlayerLevel = GetHighestEngagedPlayerLevel(creature, target);
    if (highestPlayerLevel == 0)
        return;

    uint8 targetLevel = std::min(s_maxLevel, std::max(s_minLevel, highestPlayerLevel));
    ApplyLevel(creature, targetLevel);
}

void ContinentLevelScaling::OnCreatureDisengage(Creature* creature)
{
    if (!creature || !creature->ContinentScalingBaselineCaptured)
        return;

    if (!IsEligible(creature))
        return;

    ApplyLevel(creature, creature->ContinentScalingBaseLevel);
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
