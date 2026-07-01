/*
* Copyright (C) 2020-2021 Trickerer <https://github.com/trickerer/>
* Copyright (C) 2012 CVMagic <http://www.trinitycore.org/f/topic/6551-vas-autobalance/>
* Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* Copyright (C) 1985-2010 {VAS} KalCorp  <http://vasserver.dyndns.org/>
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
* Description: This script is intended to scale all non-player controlled creatures
* based on number of players in zone (world map) or map (dungeons).
*/

#include "AutoBalance.h"
#include "CustomData.h"
#include "Chat.h"
#include "Config.h"
#include "Group.h"
#include "Language.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"

#if MOD_PRESENT_NPCBOTS == 1
# include "botmgr.h"
#endif

#if !defined(_MSC_VER) || _MSC_VER >= 1600
# define SCRIPT_VERSION_LAST
#endif

#ifdef SCRIPT_VERSION_LAST
# define NOTHING nullptr
# define UNORDERED_MAP std::unordered_map
# define GetGUIDLow GetGUID().GetCounter
# define area_name AreaName
#else
# define NOTHING NULL
# define GetLevel getLevel
# define GetPowerType getPowerType
# define SetPowerType setPowerType
# define SetStatFlatModifier SetModifierValue
#endif

// The map values correspond with the .AutoBalance.XX.Name entries in the configuration file.
static std::map<uint32, int32> forcedCreatureIds;
static int8 PlayerCountDifficultyOffset, higherOffset, lowerOffset;
static bool Is_AB_enabled, AnnounceAB, LevelScaling, LevelEndGameBoost, DungeonsOnly, PlayerChangeNotify,
LevelUseDb, DungeonScaleDownXP, CountNpcBots;
static float GlobalRate, HealthMultiplier, ManaMultiplier, ArmorMultiplier, DamageMultiplier, MinHPModifier,
    MinManaModifier, MinDamageModifier, InflectionPoint, InflectionPointRaid, InflectionPointRaid10M,
    InflectionPointRaid25M, InflectionPointHeroic, InflectionPointRaidHeroic,
    InflectionPointRaid10MHeroic, InflectionPointRaid25MHeroic, BossInflectionMult;

void LoadForcedCreatureIdsFromString(std::string const& creatureIds, uint32 forcedPlayerCount)
{
    std::string delimitedValue;
    std::stringstream creatureIdsStream;

    creatureIdsStream.str(creatureIds);

    while (std::getline(creatureIdsStream, delimitedValue, ','))
        forcedCreatureIds[(uint32)atoi(delimitedValue.c_str())] = forcedPlayerCount;
}

int32 GetForcedNumPlayers(uint32 creatureId)
{
    // Don't want the forcedCreatureIds map to blowup to a massive empty array
    if (forcedCreatureIds.find(creatureId) == forcedCreatureIds.end())
        return -1;

    return forcedCreatureIds[creatureId];
}

AreaTableEntry const* GetAreaEntryById(uint32 id)
{
#ifdef SCRIPT_VERSION_LAST
    return sAreaTableStore.LookupEntry(id);
#else
    return GetAreaEntryByAreaID(id);
#endif
}

void GetAreaLevel(Map const* map, uint8 areaid, uint8 &minlevel, uint8 &maxlevel)
{
    LFGDungeonEntry const* dungeon = GetLFGDungeon(map->GetId(), map->GetDifficulty());
    if (dungeon && (map->IsDungeon() || map->IsRaid()))
    {
#ifdef SCRIPT_VERSION_LAST
        minlevel = dungeon->MinLevel;
        maxlevel = dungeon->TargetLevel ? dungeon->TargetLevel : dungeon->MaxLevel;
#else
        minlevel = dungeon->minlevel;
        maxlevel = dungeon->reclevel ? dungeon->reclevel : dungeon->maxlevel;
#endif
    }

    if (!minlevel && !maxlevel)
    {
        AreaTableEntry const* areaEntry = GetAreaEntryById(areaid);
#ifdef SCRIPT_VERSION_LAST
        if (areaEntry && areaEntry->ExplorationLevel > 0)
        {
            minlevel = areaEntry->ExplorationLevel;
            maxlevel = areaEntry->ExplorationLevel;
        }
#else
        if (areaEntry && areaEntry->area_level > 0)
        {
            minlevel = areaEntry->area_level;
            maxlevel = areaEntry->area_level;
        }
#endif
    }
}

float GetCreatureHealthMod(int32 rank)
{
    switch (rank)
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

class AutoBalance_WorldScript : public WorldScript
{
public:
    AutoBalance_WorldScript() : WorldScript("AutoBalance_WorldScript") { }

    void OnConfigLoad(bool reload) override
    {
        InitAutoBalanceSystem(reload);
    }

private:
    void InitAutoBalanceSystem(bool reload)
    {
        if (!reload)
            SC_LOG_INFO("server.loading", "Starting Autobalance system...");

        LoadABConfig(reload);
        SC_LOG_INFO("server.loading", ">> Autobalance config loaded.");

#if MOD_PRESENT_NPCBOTS == 1
        SC_LOG_INFO("server.loading", ">>  Found NPCBots.");
#endif

        if (!Is_AB_enabled)
            SC_LOG_INFO("server.loading", ">> Autobalance system is disabled.");
    }

    void LoadABConfig(bool /*reload*/)
    {
        forcedCreatureIds.clear();
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.ForcedID40", ""), 40);
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.ForcedID25", ""), 25);
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.ForcedID10", ""), 10);
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.ForcedID5", ""), 5);
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.ForcedID2", ""), 2);
        LoadForcedCreatureIdsFromString(sConfigMgr->GetStringDefault("AutoBalance.DisabledID", ""), 0);

        Is_AB_enabled = sConfigMgr->GetBoolDefault("AutoBalance.enable", true);
        AnnounceAB = sConfigMgr->GetBoolDefault("AutoBalance.Announce.enable", true);
        LevelScaling = sConfigMgr->GetBoolDefault("AutoBalance.levelScaling", true);
        LevelEndGameBoost = sConfigMgr->GetBoolDefault("AutoBalance.LevelEndGameBoost", true);
        DungeonsOnly = sConfigMgr->GetBoolDefault("AutoBalance.DungeonsOnly", true);
        PlayerChangeNotify = sConfigMgr->GetBoolDefault("AutoBalance.PlayerChangeNotify", true);
        LevelUseDb = sConfigMgr->GetBoolDefault("AutoBalance.levelUseDbValuesWhenExists", true);
        DungeonScaleDownXP = sConfigMgr->GetBoolDefault("AutoBalance.DungeonScaleDownXP", false);
        CountNpcBots = sConfigMgr->GetBoolDefault("AutoBalance.CountNpcBots", true);

        PlayerCountDifficultyOffset = sConfigMgr->GetIntDefault("AutoBalance.playerCountDifficultyOffset", 0);
        higherOffset = sConfigMgr->GetIntDefault("AutoBalance.levelHigherOffset", 3);
        lowerOffset = sConfigMgr->GetIntDefault("AutoBalance.levelLowerOffset", 0);

        InflectionPoint = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPoint", 0.5f);
        InflectionPointRaid = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaid", InflectionPoint);
        InflectionPointRaid25M = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaid25M", InflectionPointRaid);
        InflectionPointRaid10M = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaid10M", InflectionPointRaid);
        InflectionPointHeroic = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointHeroic", InflectionPoint);
        InflectionPointRaidHeroic = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaidHeroic", InflectionPointRaid);
        InflectionPointRaid25MHeroic = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaid25MHeroic", InflectionPointRaid25M);
        InflectionPointRaid10MHeroic = sConfigMgr->GetFloatDefault("AutoBalance.InflectionPointRaid10MHeroic", InflectionPointRaid10M);
        BossInflectionMult = sConfigMgr->GetFloatDefault("AutoBalance.BossInflectionMult", 1.0f);
        GlobalRate = sConfigMgr->GetFloatDefault("AutoBalance.rate.global", 1.0f);
        HealthMultiplier = sConfigMgr->GetFloatDefault("AutoBalance.rate.health", 1.0f);
        ManaMultiplier = sConfigMgr->GetFloatDefault("AutoBalance.rate.mana", 1.0f);
        ArmorMultiplier = sConfigMgr->GetFloatDefault("AutoBalance.rate.armor", 1.0f);
        DamageMultiplier = sConfigMgr->GetFloatDefault("AutoBalance.rate.damage", 1.0f);
        MinHPModifier = sConfigMgr->GetFloatDefault("AutoBalance.MinHPModifier", 0.1f);
        MinManaModifier = sConfigMgr->GetFloatDefault("AutoBalance.MinManaModifier", 0.1f);
        MinDamageModifier = sConfigMgr->GetFloatDefault("AutoBalance.MinDamageModifier", 0.1f);
    }
};

typedef UNORDERED_MAP<uint32, uint32> ZonePlayersMap;
ZonePlayersMap zonePlayers;
typedef UNORDERED_MAP<uint32, uint8> ZoneLevelMap;
ZoneLevelMap zoneLevels;
typedef UNORDERED_MAP<uint32, uint32> PlayersRecalcMap;
PlayersRecalcMap recalcTimers;
#define PLAYERS_COUNT_RECALC_TIMER 2500

class AutoBalance_PlayerScript : public PlayerScript
{
public:
    AutoBalance_PlayerScript() : PlayerScript("AutoBalance_PlayerScript") { }

#ifdef SCRIPT_VERSION_LAST
    void OnLogin(Player* player, bool /*firstLogin*/) override
#else
    void OnLogin(Player* player) override
#endif
    {
        if (AnnounceAB)
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00AutoBalance |rmodule.");
    }

    void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        if (!Is_AB_enabled || !player || !LevelScaling)
            return;

        MapCustomData* mapABInfo = &player->GetMap()->CustomData;
        if (!player->GetMap()->GetEntry()->IsWorldMap())
        {
            if (mapABInfo->mapLevel < player->GetLevel())
                mapABInfo->mapLevel = player->GetLevel();
        }
        else if (zoneLevels.count(player->GetZoneId()) == 0 || zoneLevels[player->GetZoneId()] < player->GetLevel())
            zoneLevels[player->GetZoneId()] = player->GetLevel();
    }

    void OnGiveXP(Player* player, uint32 &amount, Unit* victim) override
    {
        if (victim && DungeonScaleDownXP)
        {
            Map* map = player->GetMap();
            if (map->IsDungeon())
            {
                // Ensure that the players always get the same XP, even when entering the dungeon alone
                uint32 maxPlayerCount = ((InstanceMap*)sMapMgr->FindMap(map->GetId(), map->GetInstanceId()))->GetMaxPlayers();
                uint32 currentPlayerCount = map->GetPlayersCountExceptGMs();
                amount *= float(currentPlayerCount) / maxPlayerCount;
            }
        }
    }
};

class AutoBalance_UnitScript : public UnitScript
{
public:
    AutoBalance_UnitScript() : UnitScript("AutoBalance_UnitScript") { }

    void ModifyHealRecieved(Unit* healer, Unit* target, uint32 &heal) override
    {
        ModifyAmount(healer, target, heal);
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32 &damage) override
    {
        ModifyAmount(attacker, target, damage);
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32 &damage) override
    {
        ModifyAmount(attacker, target, damage);
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32 &damage) override
    {
        if (damage < 0)
            return;

        uint32 val = uint32(damage);
        ModifyAmount(attacker, target, val);
        damage = val;
    }

private:
    void ModifyAmount(Unit* source, Unit* /*target*/, uint32& value) const
    {
        if (!Is_AB_enabled)
            return;

        if (!source || !value || source->GetTypeId() != TYPEID_UNIT || !source->IsInWorld() || source->IsControlledByPlayer())
            return;

        if (DungeonsOnly && !source->GetMap()->Instanceable())
            return;

        float damageMultiplier = source->ToCreature()->CustomData.damageMultiplier;
        if (damageMultiplier == 1.0f)
            return;

        value = std::max<uint32>(value * damageMultiplier, 1);
    }
};

uint32 GetPlayersCountInZone(uint32 zoneId)
{
    return zonePlayers.count(zoneId) != 0 ? zonePlayers[zoneId] : 0;
}
uint32 GetMaxLevelInZone(uint32 zoneId)
{
    return zoneLevels.count(zoneId) != 0 ? zoneLevels[zoneId] : 0;
}

uint32 CountedControlledCreatures(Player const* player)
{
    uint32 count = 0;

#if MOD_PRESENT_NPCBOTS == 1
    if (CountNpcBots && player->HaveBot())
    {
        BotMap const* botmap = player->GetBotMgr()->GetBotMap();
        for (BotMap::const_iterator itr = botmap->begin(); itr != botmap->end(); ++itr)
            if (itr->second && itr->second->IsInMap(player))
                count++;
    }
#endif

    return count;
}

bool CanModifyCreatureAttributes(Creature const* creature)
{
    if (!creature || !creature->IsInWorld() || creature->IsControlledByPlayer() ||
        creature->GetCreatureType() == CREATURE_TYPE_CRITTER)
        return false;

#if MOD_PRESENT_NPCBOTS == 1
    if (creature->IsNPCBotOrPet())
        return false;
#endif

    return true;
}

class AutoBalance_AllMapScript : public AllMapScript
{
public:
    AutoBalance_AllMapScript() : AllMapScript("AutoBalance_AllMapScript") { }

    //void OnPlayerEnterAll(Map* map, Player* player) override
    //{
    //    if (!Is_AB_enabled)
    //        return;

    //    if (!player)
    //    {
    //        SC_LOG_ERROR("scripts", "OnPlayerEnterAll: no player (mapid %u)", map->GetId());
    //        return;
    //    }

    //    if (player->IsGameMaster())
    //        return;

    //    MapCustomData* mapABInfo = &map->CustomData;
    //    uint32 pcount;
    //    // always check level, even if not conf Is_AB_enabled
    //    // because we can enable at runtime and we need this information
    //    if (!map->GetEntry()->IsWorldMap())
    //    {
    //        mapABInfo->playerCount++;
    //        if (player->GetLevel() > mapABInfo->mapLevel)
    //            mapABInfo->mapLevel = player->GetLevel();

    //        pcount = mapABInfo->playerCount;
    //    }
    //    else
    //    {
    //        zonePlayers[player->GetZoneId()]++;
    //        if (zoneLevels.count(player->GetZoneId()) == 0 || zoneLevels[player->GetZoneId()] < player->GetLevel())
    //            zoneLevels[player->GetZoneId()] = player->GetLevel();

    //        pcount = zonePlayers[player->GetZoneId()];
    //    }

    //    if (PlayerChangeNotify)
    //    {
    //        Map::PlayerList const& playerList = map->GetPlayers();
    //        if (!playerList.isEmpty())
    //        {
    //            for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
    //            {
    //                if (Player const* playerHandle = citr->GetSource())
    //                {
    //                    ChatHandler(playerHandle->GetSession()).PSendSysMessage(
    //                        "|cffFF0000 [AutoBalance]|r|cffFF8000 %s entered %s. New player count in map or zone: %u (Player Difficulty Offset = %u)|r",
    //                        player->GetName().c_str(), map->GetMapName(), pcount + PlayerCountDifficultyOffset, PlayerCountDifficultyOffset);
    //                }
    //            }
    //        }
    //    }
    //}

    //void OnPlayerLeaveAll(Map* map, Player* player) override
    //{
    //    if (!Is_AB_enabled)
    //        return;

    //    if (!player)
    //    {
    //        SC_LOG_ERROR("scripts", "OnPlayerLeaveAll: no player (mapid %u)", map->GetId());
    //        return;
    //    }

    //    if (player->IsGameMaster())
    //        return;

    //    MapCustomData* mapABInfo = &map->CustomData;
    //    uint32 pcount;
    //    // always check level, even if not conf Is_AB_enabled
    //    // because we can enable at runtime and we need this information
    //    if (!map->GetEntry()->IsWorldMap())
    //    {
    //        mapABInfo->playerCount--;
    //        if (mapABInfo->playerCount == 0)
    //        {
    //            mapABInfo->mapLevel = 0;
    //            return;
    //        }
    //        pcount = mapABInfo->playerCount;
    //    }
    //    else
    //    {
    //        zonePlayers[player->GetZoneId()]--;
    //        if (zonePlayers[player->GetZoneId()] == 0)
    //        {
    //            zoneLevels[player->GetZoneId()] = 0;
    //            return;
    //        }
    //        pcount = zonePlayers[player->GetZoneId()];
    //    }

    //    if (PlayerChangeNotify)
    //    {
    //        Map::PlayerList const& playerList = map->GetPlayers();
    //        if (!playerList.isEmpty())
    //        {
    //            for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
    //            {
    //                if (Player const* playerHandle = citr->GetSource())
    //                {
    //                    ChatHandler(playerHandle->GetSession()).PSendSysMessage(
    //                        "|cffFF0000 [AutoBalance]|r|cffFF8000 %s left %s. New player count in map or zone: %u (Player Difficulty Offset = %u)|r",
    //                        player->GetName().c_str(), map->GetMapName(), pcount + PlayerCountDifficultyOffset, PlayerCountDifficultyOffset);
    //                }
    //            }
    //        }
    //    }
    //}

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        recalcTimers[map->GetId()] = 0;

        if (player->IsGameMaster())
            return;

        if (PlayerChangeNotify)
        {
            Map::PlayerList const& playerList = map->GetPlayers();
            if (!playerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                {
                    if (Player const* playerHandle = citr->GetSource())
                    {
                        if (playerHandle == player)
                            continue;

                        ChatHandler(playerHandle->GetSession()).PSendSysMessage("",
                            player->GetName().c_str(), map->GetMapName(), !map->GetEntry()->IsWorldMap() ? " (dungeon)" : "");
                    }
                }
            }

            ChatHandler(player->GetSession()).PSendSysMessage("",
                player->GetName().c_str(), map->GetMapName(), !map->GetEntry()->IsWorldMap() ? " (dungeon)" : "");
        }
    }

    void OnPlayerLeaveAll(Map* map, Player* player) override
    {
        recalcTimers[map->GetId()] = 0;

        if (player->IsGameMaster())
            return;

        if (PlayerChangeNotify)
        {
            Map::PlayerList const& playerList = map->GetPlayers();
            if (!playerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                {
                    if (Player const* playerHandle = citr->GetSource())
                    {
                        if (playerHandle == player)
                            continue;

                        ChatHandler(playerHandle->GetSession()).PSendSysMessage("",
                            player->GetName().c_str(), map->GetMapName(), !map->GetEntry()->IsWorldMap() ? " (dungeon)" : "");
                    }
                }
            }

            ChatHandler(player->GetSession()).PSendSysMessage("",
                player->GetName().c_str(), map->GetMapName(), !map->GetEntry()->IsWorldMap() ? " (dungeon)" : "");
        }
    }

    void OnAllUpdate(Map* map, uint32 diff) override
    {
        if (!Is_AB_enabled)
            return;

        if (!map->GetEntry())
            return;

        if (!map->GetEntry()->IsWorldMap() && map->GetPlayers().isEmpty())
            return;

        if (recalcTimers[map->GetId()] >= PLAYERS_COUNT_RECALC_TIMER)
        {
            //SC_LOG_ERROR("scripts", "updating %u", map->GetId());
            recalcTimers[map->GetId()] = urand(0, 1000);

            Map::PlayerList const& playerList = map->GetPlayers();
            if (!playerList.isEmpty())
            {
                if (map->GetEntry()->IsWorldMap())
                {
                    ZonePlayersMap newpmap;
                    ZoneLevelMap newlmap;
                    for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                    {
                        Player const* player = citr->GetSource();
                        if (player && player->IsInWorld())
                        {
                            uint32 zoneId = player->GetZoneId();
                            newpmap[zoneId]++;
                            newpmap[zoneId] += CountedControlledCreatures(player);

                            if (newlmap.count(zoneId) == 0 || newlmap[zoneId] < player->GetLevel())
                                newlmap[zoneId] = player->GetLevel();
                        }
                    }

                    if (PlayerChangeNotify)
                    {
                        for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                        {
                            Player const* player = citr->GetSource();
                            if (player)
                            {
                                uint32 zoneId = player->GetZoneId();
                                if (zonePlayers.count(zoneId) == 0 || zonePlayers[zoneId] != newpmap[zoneId])
                                {
                                    ChatHandler(player->GetSession()).PSendSysMessage(
                                        "",
                                        zoneId ? GetAreaEntryById(zoneId)->area_name[0] : "Unknown", newpmap[zoneId] + PlayerCountDifficultyOffset, PlayerCountDifficultyOffset);
                                }
                            }
                        }
                    }

                    for (ZonePlayersMap::iterator it = zonePlayers.begin(); it != zonePlayers.end(); ++it)
                        if (newpmap.count(it->first) == 0)
                            it->second = 0;

                    for (ZonePlayersMap::const_iterator it = newpmap.begin(); it != newpmap.end(); ++it)
                        zonePlayers[it->first] = it->second;

                    for (ZoneLevelMap::iterator it = zoneLevels.begin(); it != zoneLevels.end(); ++it)
                        if (newlmap.count(it->first) == 0)
                            it->second = 0;

                    for (ZoneLevelMap::const_iterator it = newlmap.begin(); it != newlmap.end(); ++it)
                        zoneLevels[it->first] = it->second;
                }
                else
                {
                    uint32 pcount = 0;
                    uint8 mlevel = 0;

                    for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                    {
                        Player const* player = citr->GetSource();
                        if (player && player->IsInWorld() && !player->IsGameMaster())
                        {
                            pcount++;
                            pcount += CountedControlledCreatures(player);
                            if (player->GetLevel() > mlevel)
                                mlevel = player->GetLevel();
                        }
                    }

                    if (map->CustomData.playerCount == pcount && map->CustomData.mapLevel == mlevel)
                        return;

                    if (PlayerChangeNotify && map->CustomData.playerCount != pcount)
                    {
                        Map::PlayerList const& playerList = map->GetPlayers();
                        if (!playerList.isEmpty())
                        {
                            for (Map::PlayerList::const_iterator citr = playerList.begin(); citr != playerList.end(); ++citr)
                            {
                                if (Player const* playerHandle = citr->GetSource())
                                {
                                    ChatHandler(playerHandle->GetSession()).PSendSysMessage(
                                        "",
                                        map->GetMapName(), pcount + PlayerCountDifficultyOffset, PlayerCountDifficultyOffset);
                                }
                            }
                        }
                    }

                    map->CustomData.playerCount = pcount;
                    map->CustomData.mapLevel = mlevel;
                }
            }
        } else recalcTimers[map->GetId()] += diff;
    }
};

static uint32 DEBUG_CREATURE = 9999999;
class AutoBalance_AllCreatureScript : public AllCreatureScript
{
public:
    AutoBalance_AllCreatureScript() : AllCreatureScript("AutoBalance_AllCreatureScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!Is_AB_enabled)
            return;

        ModifyCreatureAttributes(creature);
    }

private:
    bool _checkLevelOffset(uint8 selectedLevel, uint8 targetLevel)
    {
        return selectedLevel &&
            ((targetLevel >= selectedLevel && targetLevel <= (selectedLevel + higherOffset)) ||
            (targetLevel <= selectedLevel && targetLevel >= (selectedLevel - lowerOffset)));
    }

    void ModifyCreatureAttributes(Creature* creature, bool resetSelLevel = false)
    {
        if (!CanModifyCreatureAttributes(creature))
            return;

        if (DungeonsOnly && !creature->GetMap()->Instanceable())
            return;

        MapCustomData* mapABInfo = &creature->GetMap()->CustomData;
        bool isWorldMap = creature->GetMap()->GetEntry()->IsWorldMap();
        if (!isWorldMap)
        {
            if (!mapABInfo->mapLevel || !mapABInfo->playerCount)
                return;
        }
        else if (zoneLevels.count(creature->GetZoneId()) == 0 || zoneLevels[creature->GetZoneId()] == 0 ||
                zonePlayers.count(creature->GetZoneId()) == 0 || zonePlayers[creature->GetZoneId()] == 0)
                return;

        CreatureTemplate const* creatureTemplate = creature->GetCreatureTemplate();
        InstanceMap* instanceMap = !isWorldMap ? (InstanceMap*)sMapMgr->FindMap(creature->GetMapId(), creature->GetInstanceId()) : NOTHING;
        int32 forcedNumPlayers = GetForcedNumPlayers(creatureTemplate->Entry);
        uint32 maxNumberOfPlayers = instanceMap ? instanceMap->GetMaxPlayers() : MAX_GROUP_SIZE;

        if (forcedNumPlayers > 0)
            maxNumberOfPlayers = forcedNumPlayers; // Force maxNumberOfPlayers to be changed to match the Configuration entries ForcedID2, ForcedID5, ForcedID10, ForcedID20, ForcedID25, ForcedID40
        else if (forcedNumPlayers == 0)
            return; // forcedNumPlayers 0 means that the creature is contained in DisabledID -> no scaling

        CreatureCustomData* creatureABInfo = &creature->CustomData;
        if (resetSelLevel || (creatureABInfo->entry != 0 && creatureABInfo->entry != creature->GetEntry()))
            creatureABInfo->selectedLevel = 0; // force a recalculation

        if (!creature->IsAlive())
            return;

        uint32 curCount = (instanceMap ? mapABInfo->playerCount : GetPlayersCountInZone(creature->GetZoneId())) + PlayerCountDifficultyOffset;
        uint8 level = !isWorldMap ? mapABInfo->mapLevel : zoneLevels[creature->GetZoneId()];
        uint8 bonusLevel = creatureTemplate->rank == CREATURE_ELITE_WORLDBOSS ? 3 : 0;

        // already scaled
        if (creatureABInfo->selectedLevel > 0)
        {
            if (LevelScaling)
            {
                if (_checkLevelOffset(level + bonusLevel, creature->GetLevel()) &&
                    _checkLevelOffset(creatureABInfo->selectedLevel, creature->GetLevel()) &&
                    creatureABInfo->instancePlayerCount == curCount)
                    return;
            }
            else if (creatureABInfo->instancePlayerCount == curCount)
                return;
        }

        creatureABInfo->instancePlayerCount = curCount;
        // no players in map, do not modify attributes
        if (!creatureABInfo->instancePlayerCount)
            return;

        uint8 originalLevel = (creatureTemplate->minlevel + creatureTemplate->maxlevel) / 2;
        uint8 areaMinLvl = originalLevel, areaMaxLvl = originalLevel;
        GetAreaLevel(creature->GetMap(), creature->GetAreaId(), areaMinLvl, areaMaxLvl);

        // avoid level changing for critters and special creatures (spell summons etc.) in instances
        bool skipLevel = (creatureTemplate->maxlevel <= 1 && areaMinLvl >= 5);

        if (LevelScaling && (!DungeonsOnly || creature->GetMap()->Instanceable()) && !skipLevel && !_checkLevelOffset(level, originalLevel))
        {
            // change level only whithin the offsets and when in dungeon/raid
            if (level != creatureABInfo->selectedLevel || creatureABInfo->selectedLevel != creature->GetLevel())
            {
                // keep bosses +3 level
                creatureABInfo->selectedLevel = level + bonusLevel;
                creature->SetLevel(creatureABInfo->selectedLevel);
            }
        }
        else
            creatureABInfo->selectedLevel = creature->GetLevel();

        creatureABInfo->entry = creature->GetEntry();
        bool useDefStats = (LevelUseDb &&
            creature->GetLevel() >= creatureTemplate->minlevel &&
            creature->GetLevel() <= creatureTemplate->maxlevel);

        CreatureBaseStats const* origCreatureStats = sObjectMgr->GetCreatureBaseStats(originalLevel, creatureTemplate->unit_class);
        CreatureBaseStats const* creatureStats = sObjectMgr->GetCreatureBaseStats(creatureABInfo->selectedLevel, creatureTemplate->unit_class);

        uint32 baseHealth = origCreatureStats->GenerateHealth(creatureTemplate);
        uint32 baseMana = origCreatureStats->GenerateMana(creatureTemplate);
        uint32 scaledHealth = 0;
        uint32 scaledMana = 0;

        // Note: InflectionPoint handle the number of players required to get 50% health.
        //       you'd adjust this to raise or lower the hp modifier for per additional player in a non-whole group.
        //
        //       diff modify the rate of percentage increase between
        //       number of players. Generally the closer to the value of 1 you have this
        //       the less gradual the rate will be. For example in a 5 man it would take 3
        //       total players to face a mob at full health.
        //
        //       The +1 and /2 values raise the TanH function to a positive range and make
        //       sure the modifier never goes above the value or 1.0 or below 0.
        //
        float defaultMultiplier = 1.0f;
        if (creatureABInfo->instancePlayerCount < maxNumberOfPlayers)
        {
            float inflectionValue = maxNumberOfPlayers;

            if (instanceMap)
            {
                if (instanceMap->IsHeroic())
                {
                    if (instanceMap->IsRaid())
                    {
                        switch (instanceMap->GetMaxPlayers())
                        {
                            case 10: inflectionValue *= InflectionPointRaid10MHeroic; break;
                            case 25: inflectionValue *= InflectionPointRaid25MHeroic; break;
                            default: inflectionValue *= InflectionPointRaidHeroic;    break;
                        }
                    }
                    else
                        inflectionValue *= InflectionPointHeroic;
                }
                else
                {
                    if (instanceMap->IsRaid())
                    {
                        switch (instanceMap->GetMaxPlayers())
                        {
                            case 10: inflectionValue *= InflectionPointRaid10M; break;
                            case 25: inflectionValue *= InflectionPointRaid25M; break;
                            default: inflectionValue *= InflectionPointRaid;    break;
                        }
                    }
                    else
                        inflectionValue *= InflectionPoint;
                }
            }
            else
                inflectionValue *= InflectionPoint;

            if (creature->IsDungeonBoss())
                inflectionValue *= BossInflectionMult;

            float diff = (maxNumberOfPlayers / 5.f) * 1.5f;
            defaultMultiplier = (tanh((float(creatureABInfo->instancePlayerCount) - inflectionValue) / diff) + 1.0f) / 2.0f;
        }

        creatureABInfo->healthMultiplier = HealthMultiplier * defaultMultiplier * GlobalRate;

        if (creature->GetGUIDLow() == DEBUG_CREATURE)
            SC_LOG_ERROR("scripts", "AB: hpmult: %.3f origlvl %u areaminlvl %u areamaxlvl %u",
            creatureABInfo->healthMultiplier, originalLevel, areaMinLvl, areaMaxLvl);

        if (creatureABInfo->healthMultiplier < MinHPModifier)
            creatureABInfo->healthMultiplier = MinHPModifier;

        float hpStatsRate = GetCreatureHealthMod(creatureTemplate->rank);
        if (!useDefStats && LevelScaling && !skipLevel)
        {
            float newBaseHealth = 0.f;
            if (level <= 60)
                newBaseHealth = creatureStats->BaseHealth[0];
            else if (level <= 70)
                newBaseHealth = creatureStats->BaseHealth[1];
            else
            {
                newBaseHealth = creatureStats->BaseHealth[2];
                if (LevelEndGameBoost) // special increasing for end-game contents
                    newBaseHealth *= creatureABInfo->selectedLevel >= 75 && originalLevel < 75 ? (creatureABInfo->selectedLevel - 70) * 0.3f : 1.f;
            }

            float newHealth = newBaseHealth * creatureTemplate->ModHealth;
            if (creature->GetGUIDLow() == DEBUG_CREATURE)
                SC_LOG_ERROR("scripts", "AB: level %u basehealth %u newBaseHealth %.3f, mod %.3f, newHp %.3f",
                uint32(level), baseHealth, newBaseHealth, creatureTemplate->ModHealth, newHealth);
            // allows health to be different with creatures that originally
            // differentiate their health by different level instead of multiplier field.
            // especially in dungeons. The health reduction decrease if original level is similar to the area max level
            if (originalLevel >= areaMinLvl && originalLevel < areaMaxLvl)
            {
                // never more than 30%
                float reduction = newHealth / float(areaMaxLvl - areaMinLvl) * (float(areaMaxLvl - originalLevel) * 0.3f);
                if (reduction > 0 && reduction < newHealth)
                    newHealth -= reduction;
            }
            hpStatsRate *= newHealth / float(baseHealth);
        }

        if (creature->GetGUIDLow() == DEBUG_CREATURE)
            SC_LOG_ERROR("scripts", "AB: hpStatsRate: %.3f", hpStatsRate);

        creatureABInfo->healthMultiplier *= hpStatsRate;
        scaledHealth = (baseHealth * creatureABInfo->healthMultiplier) + 0.5f;

        if (creature->GetGUIDLow() == DEBUG_CREATURE)
            SC_LOG_ERROR("scripts", "AB: scaledHealth: %u", scaledHealth);

        // Getting the list of Classes in this group
        // This will be used later on to determine what additional scaling will be required based on the ratio of tank/dps/healer
        // Update playerClassList with the list of all the participating Classes
        // GetPlayerClassList(creature, playerClassList);

        float manaStatsRate = 1.0f;
        if (!useDefStats && baseMana > 0 && LevelScaling && !skipLevel)
        {
            float newMana = creatureStats->GenerateMana(creatureTemplate);
            manaStatsRate = newMana / float(baseMana);
        }

        creatureABInfo->manaMultiplier = manaStatsRate * ManaMultiplier * defaultMultiplier * GlobalRate;
        if (creatureABInfo->manaMultiplier < MinManaModifier)
            creatureABInfo->manaMultiplier = MinManaModifier;

        scaledMana = baseMana * creatureABInfo->manaMultiplier + 0.5f;
        float damageMul = defaultMultiplier * GlobalRate * DamageMultiplier;
        // Can not be less then Min_D_Mod
        if (damageMul < MinDamageModifier)
            damageMul = MinDamageModifier;

        creatureABInfo->armorMultiplier = defaultMultiplier * GlobalRate * ArmorMultiplier;
        uint32 newBaseArmor = 0.5f + (creatureABInfo->armorMultiplier *
                                    ((useDefStats || !LevelScaling || skipLevel) ? origCreatureStats->GenerateArmor(creatureTemplate)
                                                                                 : creatureStats->GenerateArmor(creatureTemplate)));
        uint32 prevMaxHealth = creature->GetMaxHealth();
        uint32 prevMaxPower = creature->GetMaxPower(POWER_MANA);
        uint32 prevHealth = creature->GetHealth();
        uint32 prevPower = creature->GetPower(POWER_MANA);
        Powers pType = creature->GetPowerType();

        float basedamage = creatureStats->GenerateBaseDamage(creatureTemplate);
        if (LevelEndGameBoost && level > 70 && creatureABInfo->selectedLevel >= 75 && originalLevel < 75 && !creature->GetMap()->IsRaid())
            basedamage *= float(creatureABInfo->selectedLevel - 70) * 0.3f;

        float weaponBaseMinDamage = basedamage;
        float weaponBaseMaxDamage = basedamage * 1.5f;

        creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, weaponBaseMinDamage);
        creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);
        creature->SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, weaponBaseMinDamage);
        creature->SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);
        creature->SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, weaponBaseMinDamage);
        creature->SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);
        creature->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER, BASE_VALUE, creatureStats->AttackPower);
        creature->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, creatureStats->RangedAttackPower);

        creature->SetArmor(newBaseArmor);
        creature->SetStatFlatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(newBaseArmor));
        creature->SetCreateHealth(scaledHealth);
        creature->SetMaxHealth(scaledHealth);
        creature->ResetPlayerDamageReq();
        creature->SetCreateMana(scaledMana);
        creature->SetMaxPower(POWER_MANA, scaledMana);
        creature->SetStatFlatModifier(UNIT_MOD_ENERGY, BASE_VALUE, 100.0f);
        creature->SetStatFlatModifier(UNIT_MOD_RAGE, BASE_VALUE, 100.0f);
        creature->SetStatFlatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(scaledHealth));
        creature->SetStatFlatModifier(UNIT_MOD_MANA, BASE_VALUE, float(scaledMana));
        creatureABInfo->damageMultiplier = damageMul;

        uint32 scaledCurHealth = (prevHealth && prevMaxHealth) ? float(scaledHealth) / float(prevMaxHealth) * float(prevHealth) : 0;
        uint32 scaledCurPower = (prevPower && prevMaxPower) ? float(scaledMana) / float(prevMaxPower) * float(prevPower) : 0;

        if (creature->GetGUIDLow() == DEBUG_CREATURE)
            SC_LOG_ERROR("scripts", "AB: prevHealth %u, prevMaxHealth %u, scaledHealth %u, scaledCurHealth %u",
                prevHealth, prevMaxHealth, scaledHealth, scaledCurHealth);
        if (creature->GetGUIDLow() == DEBUG_CREATURE)
            SC_LOG_ERROR("scripts", "AB: prevPower %u, prevMaxPower %u, scaledMana %u, scaledCurPower %u",
                prevPower, prevMaxPower, scaledMana, scaledCurPower);

        creature->SetHealth(scaledCurHealth);
        if (pType == POWER_MANA)
            creature->SetPower(POWER_MANA, scaledCurPower);
        else
        {
            // fix creatures with different power types
            creature->SetPowerType(pType);
        }

        creature->UpdateAllStats();
    }
};

#ifdef SCRIPT_VERSION_LAST
using namespace Syphrena::ChatCommands;
#endif

#define GM_COMMANDS rbac::RBACPermissions(197)

class AutoBalance_CommandScript : public CommandScript
{
public:
    AutoBalance_CommandScript() : CommandScript("AutoBalance_CommandScript") { }

#ifdef SCRIPT_VERSION_LAST
    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable ABCommandTable =
        {
            { "mstats",     HandleABMStatsCommand,          GM_COMMANDS, Console::Yes },
            { "zstats",     HandleABZStatsCommand,          GM_COMMANDS, Console::Yes },
            { "setoffset",  HandleABSetOffsetCommand,       GM_COMMANDS, Console::Yes },
            { "getoffset",  HandleABGetOffsetCommand,       GM_COMMANDS, Console::Yes },
            { "mapstat",    HandleABMapStatsCommand,        GM_COMMANDS, Console::No  },
            { "crstat",     HandleABCreatureStatsCommand,   GM_COMMANDS, Console::No  },
        };

        static ChatCommandTable commandTable =
        {
            { "vas",        ABCommandTable                                            },
        };
#else
    ChatCommand* GetCommands() const
    {
        static ChatCommand ABCommandTable[] =
        {
            { "mstats",     GM_COMMANDS, true, &HandleABMStatsCommand,        "Lists all registered map update timers",                                                             NULL },
            { "zstats",     GM_COMMANDS, true, &HandleABZStatsCommand,        "Lists all registered zones with players count",                                                             NULL },
            { "setoffset",  GM_COMMANDS, true, &HandleABSetOffsetCommand,     "Sets the global Player Difficulty Offset for instances. Example: (You + offset(1) = 2 player difficulty).", NULL },
            { "getoffset",  GM_COMMANDS, true, &HandleABGetOffsetCommand,     "Shows current global player offset value",                                                                  NULL },
            { "mapstat",    GM_COMMANDS, false,&HandleABMapStatsCommand,      "Shows current autobalance information for this map-",                                                       NULL },
            { "crstat",     GM_COMMANDS, false,&HandleABCreatureStatsCommand, "Shows current autobalance information for selected creature.",                                              NULL },
            { NULL,         0,           false,NULL,                          "",                                                                                                          NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "vas",        GM_COMMANDS, true, NULL,                          "", ABCommandTable },
            { NULL,         0,           false,NULL,                          "",           NULL }
        };
#endif

        return commandTable;
    }

    static bool HandleABMStatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (recalcTimers.empty())
        {
            handler->SendSysMessage("No registered maps");
            return true;
        }

        for (PlayersRecalcMap::const_iterator itr = recalcTimers.begin(); itr != recalcTimers.end(); ++itr)
        {
            MapEntry const* m = sMapStore.LookupEntry(itr->first);
            handler->PSendSysMessage("Map: %s (%u), cur time: %u",
#ifdef SCRIPT_VERSION_LAST
                m->MapName[0],
#else
                m->name[0],
#endif
                itr->first, itr->second);
        }

        return true;
    }

    static bool HandleABZStatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (zonePlayers.empty())
        {
            handler->SendSysMessage("No registered zones");
            return true;
        }

        for (ZonePlayersMap::const_iterator itr = zonePlayers.begin(); itr != zonePlayers.end(); ++itr)
        {
            uint8 lvl = zoneLevels.count(itr->first) != 0 ? zoneLevels[itr->first] : 0;
            handler->PSendSysMessage("Zone: %s, players: %u, maxlvl: %u",
                GetAreaEntryById(itr->first)->area_name[0], itr->second, uint32(lvl));
        }

        return true;
    }

    static bool HandleABSetOffsetCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(".vas setoffset #");
            handler->PSendSysMessage("Sets the Player Difficulty Offset for instances. Example: (You + offset(1) = 2 player difficulty).");
            return false;
        }
        char* offset = strtok((char*)args, " ");
        int32 offseti = -1;

        if (!offset)
        {
            handler->PSendSysMessage("Error changing Player Difficulty Offset! Please try again.");
            return false;
        }

        offseti = (uint32)atoi(offset);
        handler->PSendSysMessage("Changing Player Difficulty Offset to %i.", offseti);
        PlayerCountDifficultyOffset = offseti;

        return true;
    }

    static bool HandleABGetOffsetCommand(ChatHandler* handler, const char* /*args*/)
    {
        handler->PSendSysMessage("Current Player Difficulty Offset = %i", PlayerCountDifficultyOffset);
        return true;
    }

    static bool HandleABMapStatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* pl = handler->getSelectedPlayer();
        if (!pl)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        MapCustomData* mapABInfo = &pl->GetMap()->CustomData;
        handler->PSendSysMessage("Players in  this %s: %u", pl->GetMap()->GetEntry()->IsWorldMap() ? "zone" : "map",
            pl->GetMap()->GetEntry()->IsWorldMap() ? GetPlayersCountInZone(pl->GetZoneId()) : mapABInfo->playerCount);
        handler->PSendSysMessage("Max level of players in this %s: %u", pl->GetMap()->GetEntry()->IsWorldMap() ? "zone" : "map",
            pl->GetMap()->GetEntry()->IsWorldMap() ? GetMaxLevelInZone(pl->GetZoneId()) : mapABInfo->mapLevel);

        return true;
    }

    static bool HandleABCreatureStatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureCustomData const* creatureABInfo = &target->CustomData;
        handler->PSendSysMessage("Effective player Count: %u", creatureABInfo->instancePlayerCount);
        handler->PSendSysMessage("Selected level: %u", creatureABInfo->selectedLevel);
        handler->PSendSysMessage("Damage multiplier: %.3f", creatureABInfo->damageMultiplier);
        handler->PSendSysMessage("Health multiplier: %.3f", creatureABInfo->healthMultiplier);
        handler->PSendSysMessage("Mana multiplier: %.3f", creatureABInfo->manaMultiplier);
        handler->PSendSysMessage("Armor multiplier: %.3f", creatureABInfo->armorMultiplier);

        return true;
    }
};

void AddSC_AutoBalance()
{
    new AutoBalance_WorldScript;
    new AutoBalance_PlayerScript;
    new AutoBalance_UnitScript;
    new AutoBalance_AllCreatureScript;
    new AutoBalance_AllMapScript;
    new AutoBalance_CommandScript;
}

void InitAutoBalanceSystem()
{
#ifdef SCRIPT_VERSION_LAST
    std::string const& oldcontext = sScriptMgr->GetCurrentScriptContext();
    sScriptMgr->SetScriptContext("__autobalance__");
    AddSC_AutoBalance();
    sScriptMgr->SetScriptContext(oldcontext);
#else
    AddSC_AutoBalance();
#endif
}
