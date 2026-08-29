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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "TemporarySummon.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "WorldSession.h"
#include "Duration.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "Common.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BloodArena
{
    // -------------------------------------------------------------------------
    // Arène
    // -------------------------------------------------------------------------

    static uint32 const ARENA_MAP_ID = 530;

    static float const ARENA_CENTER_X = 2841.776367f;
    static float const ARENA_CENTER_Y = 5925.458496f;
    static float const ARENA_CENTER_Z = 11.317265f;
    static float const ARENA_CENTER_O = 0.833710f;

    static float const PLAYER_START_RADIUS = 5.0f;
    static float const REQUIRED_GROUP_RANGE = 60.0f;

    // Sortie volontaire : point de détection légèrement à l'intérieur
    // de chaque porte afin de remettre la phase normale AVANT la porte.
    static float const EXIT_TRIGGER_INNER_OFFSET = 2.5f;
    static float const EXIT_TRIGGER_RADIUS = 2.0f;
    static float const RETURN_IN_FRONT_OF_NPC_DISTANCE = 3.0f;

    struct ArenaDoor
    {
        float x;
        float y;
        float z;
        float o;
    };

    // Portes relevées en jeu avec .gps.
    static ArenaDoor const ARENA_DOORS[] =
    {
        { 2884.213867f, 5945.374512f, 4.469271f, 0.875168f },
        { 2792.034668f, 5912.339355f, 4.315041f, 3.993200f }
    };

    static uint32 const ARENA_DOOR_COUNT =
        sizeof(ARENA_DOORS) / sizeof(ARENA_DOORS[0]);

    // -------------------------------------------------------------------------
    // Timers
    // -------------------------------------------------------------------------

    static uint32 const START_DELAY_MS = 3000;
    static uint32 const ARRIVAL_TIMEOUT_MS = 60 * 1000;
    static uint32 const INTERWAVE_DELAY_MS = 5000;
    static uint32 const FIXED_WAVE_INTERVAL_MS = 2 * 60 * 1000;

    static uint32 const TIMER_DURATION_MS = 30 * 60 * 1000;
    static uint32 const TIMER_TOTAL_WAVES = 15;
    static uint32 const TIMER_BOSS_EVERY = 5;

    static uint32 const INFINITE_MAX_WAVES = 10000;
    static uint32 const INFINITE_BOSS_EVERY = 10;


    // Vérification du changement de mois une fois par minute.
    static uint32 const MONTH_CHECK_INTERVAL_MS = 60 * 1000;
    static uint32 const LEADERBOARD_LIMIT = 10;

    // -------------------------------------------------------------------------
    // Limites de créatures
    // -------------------------------------------------------------------------

    // Mode infini allégé.
    static uint32 const MAX_ACTIVE_TRASH_INFINITE = 30;

    // Mode timer: on conserve une limite plus haute.
    static uint32 const MAX_ACTIVE_TRASH_TIMER = 80;

    static uint32 const SUMMON_DESPAWN_MS = 60 * 60 * 1000;

    // -------------------------------------------------------------------------
    // Phases réservées à l'arène
    // -------------------------------------------------------------------------

    static uint32 const PHASE_POOL[] =
    {
        4, 8, 16, 32, 64, 128, 256, 512, 1024
    };

    static uint32 const PHASE_POOL_COUNT =
        sizeof(PHASE_POOL) / sizeof(PHASE_POOL[0]);

    // -------------------------------------------------------------------------
    // Pool SQL des créatures
    // -------------------------------------------------------------------------

    enum ArenaCreatureType : uint8
    {
        ARENA_CREATURE_TRASH = 1,
        ARENA_CREATURE_BOSS  = 2
    };

    // mode_mask dans blood_arena_creatures :
    // 1 = Infini, 2 = Timer, 3 = les deux.
    static uint8 const ARENA_MODE_MASK_INFINITE = 1;
    static uint8 const ARENA_MODE_MASK_TIMER    = 2;
    static uint8 const ARENA_MODE_MASK_BOTH     = 3;

    struct ArenaCreaturePoolEntry
    {
        uint32 id = 0;
        uint8 creatureType = ARENA_CREATURE_TRASH;
        uint32 creatureEntry = 0;
        uint8 modeMask = ARENA_MODE_MASK_BOTH;
        uint32 minWave = 1;
        uint32 maxWave = 0; // 0 = aucune limite
        uint32 weight = 1;
        uint32 sortOrder = 0;
    };

    // -------------------------------------------------------------------------
    // Récompenses
    // -------------------------------------------------------------------------

    static uint32 const ITEM_INFUSION_CRYSTAL = 339505;
    static uint32 const ITEM_STONE_KEEPER_SHARD = 43228;

    // Timer
    static uint32 const TIMER_REWARD_UNDER_10_MIN = 28000;
    static uint32 const TIMER_REWARD_UNDER_20_MIN = 14000;
    static uint32 const TIMER_REWARD_20_TO_30_MIN = 10000;

    // Infini
    static uint32 const INFINITE_NORMAL_WAVE_CRYSTALS = 100;
    static uint32 const FIRST_BOSS_CRYSTALS = 2500;
    static uint32 const FIRST_BOSS_SHARDS = 50;

    // Sécurité pour l'exponentielle des boss.
    static uint64 const MAX_SINGLE_ITEM_REWARD = 4000000000ULL;

    // -------------------------------------------------------------------------
    // Scaling
    // -------------------------------------------------------------------------

    static float const GROUP_HEALTH_PER_EXTRA_PLAYER = 0.50f;
    static float const GROUP_DAMAGE_PER_EXTRA_PLAYER = 0.30f;
    static float const GROUP_ARMOR_PER_EXTRA_PLAYER  = 0.15f;

    static float const INFINITE_HEALTH_PER_WAVE = 0.10f;
    static float const INFINITE_DAMAGE_PER_WAVE = 0.06f;
    static float const INFINITE_ARMOR_PER_WAVE  = 0.04f;

    static float const TIMER_HEALTH_PER_WAVE = 0.12f;
    static float const TIMER_DAMAGE_PER_WAVE = 0.08f;
    static float const TIMER_ARMOR_PER_WAVE  = 0.05f;

    static float const BOSS_HEALTH_MULTIPLIER = 3.00f;
    static float const BOSS_DAMAGE_MULTIPLIER = 1.35f;
    static float const BOSS_ARMOR_MULTIPLIER  = 1.20f;

    // -------------------------------------------------------------------------
    // Enums
    // -------------------------------------------------------------------------

    enum ArenaMode : uint8
    {
        MODE_INFINITE = 1,
        MODE_TIMER    = 2
    };

    enum WaveProgression : uint8
    {
        PROGRESSION_LAST_KILL = 1,
        PROGRESSION_FIXED_2_MINUTES = 2
    };

    enum GossipActions : uint32
    {
        ACTION_INFINITE_LAST_KILL = 1001,
        ACTION_INFINITE_FIXED     = 1002,
        ACTION_TIMER_LAST_KILL    = 1003,
        ACTION_TIMER_FIXED        = 1004,

        ACTION_LEADERBOARD_MENU     = 1100,
        ACTION_LEADERBOARD_INFINITE = 1101,
        ACTION_LEADERBOARD_TIMER    = 1102,
        ACTION_BACK_MAIN            = 1199,

        ACTION_ADMIN_MENU             = 1200,
        ACTION_ADMIN_RELOAD_CREATURES = 1201
    };

    // -------------------------------------------------------------------------
    // Structures de session
    // -------------------------------------------------------------------------

    struct PlayerSnapshot
    {
        ObjectGuid guid;

        uint32 oldMapId = 0;
        float oldX = 0.0f;
        float oldY = 0.0f;
        float oldZ = 0.0f;
        float oldO = 0.0f;

        uint32 oldPhaseMask = 1;
    };

    struct Session
    {
        uint64 id = 0;

        ArenaMode mode = MODE_INFINITE;
        WaveProgression progression = PROGRESSION_LAST_KILL;

        uint32 phaseMask = 1;
        uint32 wave = 0;
        uint32 highestLevel = 1;
        uint32 playerCount = 1;

        uint32 remainingMs = 0;
        uint32 nextWaveDelayMs = 0;
        uint32 fixedWaveTimerMs = 0;

        // Nouveau V1.2 : on attend l'arrivée réelle des joueurs en map 530.
        bool waitingForArrival = false;
        uint32 arrivalTimeoutMs = 0;

        bool waitingNextWave = false;
        bool started = false;
        bool completed = false;
        bool configurationError = false;

        // Point de retour commun : devant le Maître de l'Arène de sang
        // utilisé pour lancer CETTE session.
        uint32 returnMapId = 0;
        float returnX = 0.0f;
        float returnY = 0.0f;
        float returnZ = 0.0f;
        float returnO = 0.0f;


        // Identité figée du groupe au lancement pour le classement.
        // Un joueur solo est simplement un groupe d'une personne.
        std::string groupKey;
        std::string groupNames;

        // Plus haute vague réellement terminée en mode infini.
        uint32 bestClearedWave = 0;

        std::vector<ObjectGuid> players;
        std::vector<PlayerSnapshot> snapshots;
        std::vector<ObjectGuid> summons;

        // Suivi des vagues pour les récompenses du mode infini,
        // y compris si plusieurs vagues se chevauchent.
        std::unordered_map<ObjectGuid::LowType, uint32> summonWave;
        std::unordered_map<uint32, uint32> waveSpawnCount;
    };

    // -------------------------------------------------------------------------
    // Gestionnaire
    // -------------------------------------------------------------------------

    class ArenaManager
    {
    public:
        static ArenaManager& Instance()
        {
            static ArenaManager instance;
            return instance;
        }

        bool IsPlayerInArena(Player* player) const
        {
            if (!player)
                return false;

            ObjectGuid::LowType lowGuid = player->GetGUID().GetCounter();

            return _playerToSession.find(lowGuid) !=
                   _playerToSession.end();
        }

        bool IsArenaPhase(uint32 phaseMask) const
        {
            for (uint32 i = 0; i < PHASE_POOL_COUNT; ++i)
            {
                if (PHASE_POOL[i] == phaseMask)
                    return true;
            }

            return false;
        }

        std::string GetCurrentSeasonKey() const
        {
            std::time_t now = std::time(nullptr);
            std::tm* localTime = std::localtime(&now);

            if (!localTime)
                return "000000";

            char buffer[7] = { 0 };
            std::strftime(buffer, sizeof(buffer), "%Y%m", localTime);
            return std::string(buffer);
        }

        std::string GetSeasonLabel(std::string const& seasonKey) const
        {
            if (seasonKey.size() != 6)
                return seasonKey;

            return seasonKey.substr(0, 4) + "-" + seasonKey.substr(4, 2);
        }

        void InitializeDatabase()
        {
            // DDL exécuté une seule fois au démarrage.
            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_scores` ("
                "`season_key` CHAR(6) NOT NULL,"
                "`mode` TINYINT UNSIGNED NOT NULL,"
                "`group_key` VARCHAR(255) NOT NULL,"
                "`group_names` VARCHAR(512) NOT NULL,"
                "`player_count` SMALLINT UNSIGNED NOT NULL DEFAULT 1,"
                "`progression` TINYINT UNSIGNED NOT NULL DEFAULT 1,"
                "`best_wave` INT UNSIGNED NOT NULL DEFAULT 0,"
                "`best_time_ms` INT UNSIGNED NOT NULL DEFAULT 0,"
                "`updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                "PRIMARY KEY (`season_key`,`mode`,`group_key`),"
                "KEY `idx_ba_inf` (`season_key`,`mode`,`best_wave`),"
                "KEY `idx_ba_timer` (`season_key`,`mode`,`best_time_ms`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_state` ("
                "`id` TINYINT UNSIGNED NOT NULL,"
                "`current_season` CHAR(6) NOT NULL,"
                "PRIMARY KEY (`id`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_monthly_winners` ("
                "`season_key` CHAR(6) NOT NULL,"
                "`mode` TINYINT UNSIGNED NOT NULL,"
                "`group_names` VARCHAR(512) NOT NULL,"
                "`score_value` INT UNSIGNED NOT NULL DEFAULT 0,"
                "`progression` TINYINT UNSIGNED NOT NULL DEFAULT 1,"
                "`created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY (`season_key`,`mode`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_gm_notices` ("
                "`season_key` CHAR(6) NOT NULL,"
                "`notice_text` TEXT NOT NULL,"
                "`created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY (`season_key`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_gm_notice_read` ("
                "`season_key` CHAR(6) NOT NULL,"
                "`account_id` INT UNSIGNED NOT NULL,"
                "`read_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY (`season_key`,`account_id`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            // V1.6 : bestiaire administrable depuis HeidiSQL.
            WorldDatabase.DirectExecute(
                "CREATE TABLE IF NOT EXISTS `blood_arena_creatures` ("
                "`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,"
                "`creature_type` TINYINT UNSIGNED NOT NULL COMMENT '1=trash, 2=boss',"
                "`creature_entry` INT UNSIGNED NOT NULL,"
                "`mode_mask` TINYINT UNSIGNED NOT NULL DEFAULT 3 COMMENT '1=infini, 2=timer, 3=les deux',"
                "`min_wave` INT UNSIGNED NOT NULL DEFAULT 1,"
                "`max_wave` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=aucune limite',"
                "`weight` INT UNSIGNED NOT NULL DEFAULT 1,"
                "`enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,"
                "`sort_order` INT UNSIGNED NOT NULL DEFAULT 0,"
                "`note` VARCHAR(255) NOT NULL DEFAULT '',"
                "PRIMARY KEY (`id`),"
                "KEY `idx_ba_creature_load` (`enabled`,`creature_type`,`sort_order`),"
                "KEY `idx_ba_creature_entry` (`creature_entry`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");

            SeedDefaultCreaturePoolIfEmpty();
            ReloadCreaturePool();

            CheckMonthlyRollover();
        }

        bool ReloadCreaturePool()
        {
            std::vector<ArenaCreaturePoolEntry> newTrashPool;
            std::vector<ArenaCreaturePoolEntry> newBossPool;

            QueryResult result = WorldDatabase.Query(
                "SELECT `id`,`creature_type`,`creature_entry`,`mode_mask`,"
                "`min_wave`,`max_wave`,`weight`,`sort_order` "
                "FROM `blood_arena_creatures` "
                "WHERE `enabled`=1 "
                "ORDER BY `creature_type`,`sort_order`,`id`");

            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();

                    ArenaCreaturePoolEntry poolEntry;
                    poolEntry.id = fields[0].GetUInt32();
                    poolEntry.creatureType = fields[1].GetUInt8();
                    poolEntry.creatureEntry = fields[2].GetUInt32();
                    poolEntry.modeMask = fields[3].GetUInt8();
                    poolEntry.minWave = fields[4].GetUInt32();
                    poolEntry.maxWave = fields[5].GetUInt32();
                    poolEntry.weight = fields[6].GetUInt32();
                    poolEntry.sortOrder = fields[7].GetUInt32();

                    if (poolEntry.creatureEntry == 0)
                        continue;

                    // Évite qu'une faute de frappe SQL provoque des vagues vides.
                    if (!sObjectMgr->GetCreatureTemplate(poolEntry.creatureEntry))
                    {
                        SC_LOG_INFO(
                            "server.worldserver",
                            "[BloodArena] Pool ignore : creature_template {} inexistante (ligne {}).",
                            poolEntry.creatureEntry,
                            poolEntry.id);

                        continue;
                    }

                    if (poolEntry.creatureType != ARENA_CREATURE_TRASH &&
                        poolEntry.creatureType != ARENA_CREATURE_BOSS)
                    {
                        continue;
                    }

                    if (poolEntry.modeMask == 0)
                        poolEntry.modeMask = ARENA_MODE_MASK_BOTH;

                    if (poolEntry.weight == 0)
                        poolEntry.weight = 1;

                    if (poolEntry.minWave == 0)
                        poolEntry.minWave = 1;

                    if (poolEntry.maxWave > 0 &&
                        poolEntry.maxWave < poolEntry.minWave)
                    {
                        continue;
                    }

                    if (poolEntry.creatureType == ARENA_CREATURE_BOSS)
                        newBossPool.push_back(poolEntry);
                    else
                        newTrashPool.push_back(poolEntry);

                } while (result->NextRow());
            }

            _trashPool.swap(newTrashPool);
            _bossPool.swap(newBossPool);

            SC_LOG_INFO(
                "server.worldserver",
                "[BloodArena] Pool recharge : {} trash(s), {} boss.",
                uint32(_trashPool.size()),
                uint32(_bossPool.size()));

            return !_trashPool.empty() && !_bossPool.empty();
        }

        uint32 GetLoadedTrashCount() const
        {
            return static_cast<uint32>(_trashPool.size());
        }

        uint32 GetLoadedBossCount() const
        {
            return static_cast<uint32>(_bossPool.size());
        }

        void NotifyUnreadGmNotices(Player* player)
        {
            if (!player || !player->GetSession())
                return;

            WorldSession* session = player->GetSession();

            if (session->GetSecurity() < SEC_GAMEMASTER)
                return;

            uint32 accountId = session->GetAccountId();

            QueryResult result = WorldDatabase.PQuery(
                "SELECT n.`season_key`, n.`notice_text` "
                "FROM `blood_arena_gm_notices` n "
                "LEFT JOIN `blood_arena_gm_notice_read` r "
                "ON r.`season_key` = n.`season_key` AND r.`account_id` = {} "
                "WHERE r.`account_id` IS NULL "
                "ORDER BY n.`season_key` ASC",
                accountId);

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();

                std::string seasonKey = fields[0].GetString();
                std::string noticeText = fields[1].GetString();

                ChatHandler(session).SendSysMessage(noticeText.c_str());

                WorldDatabase.PExecute(
                    "INSERT IGNORE INTO `blood_arena_gm_notice_read` "
                    "(`season_key`,`account_id`) VALUES ('{}',{})",
                    seasonKey,
                    accountId);

            } while (result->NextRow());
        }

        void ShowLeaderboard(
            Player* player,
            Creature* creature,
            ArenaMode mode) const
        {
            if (!player || !creature)
                return;

            ClearGossipMenuFor(player);

            std::string season = GetCurrentSeasonKey();
            std::string title =
                mode == MODE_INFINITE
                    ? "Top 10 - Vagues infinies - " + GetSeasonLabel(season)
                    : "Top 10 - Timer - " + GetSeasonLabel(season);

            AddGossipItemFor(
                player,
                GOSSIP_ICON_CHAT,
                title,
                GOSSIP_SENDER_MAIN,
                mode == MODE_INFINITE
                    ? ACTION_LEADERBOARD_INFINITE
                    : ACTION_LEADERBOARD_TIMER);

            QueryResult result;

            if (mode == MODE_INFINITE)
            {
                result = WorldDatabase.PQuery(
                    "SELECT `group_names`,`best_wave`,`progression` "
                    "FROM `blood_arena_scores` "
                    "WHERE `season_key`='{}' AND `mode`={} AND `best_wave`>0 "
                    "ORDER BY `best_wave` DESC, `updated_at` ASC "
                    "LIMIT {}",
                    season,
                    uint32(MODE_INFINITE),
                    LEADERBOARD_LIMIT);
            }
            else
            {
                result = WorldDatabase.PQuery(
                    "SELECT `group_names`,`best_time_ms`,`progression` "
                    "FROM `blood_arena_scores` "
                    "WHERE `season_key`='{}' AND `mode`={} AND `best_time_ms`>0 "
                    "ORDER BY `best_time_ms` ASC, `updated_at` ASC "
                    "LIMIT {}",
                    season,
                    uint32(MODE_TIMER),
                    LEADERBOARD_LIMIT);
            }

            uint32 rank = 1;

            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();

                    std::string groupNames = fields[0].GetString();
                    uint32 score = fields[1].GetUInt32();
                    uint32 progression = fields[2].GetUInt32();

                    if (groupNames.size() > 110)
                        groupNames = groupNames.substr(0, 107) + "...";

                    std::ostringstream row;

                    row << "#" << rank << " - " << groupNames << " - ";

                    if (mode == MODE_INFINITE)
                    {
                        row << "Vague " << score;
                    }
                    else
                    {
                        row << FormatDuration(score);
                    }

                    row << " ["
                        << (progression == PROGRESSION_FIXED_2_MINUTES
                            ? "2 min"
                            : "Dernier mob")
                        << "]";

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        row.str(),
                        GOSSIP_SENDER_MAIN,
                        mode == MODE_INFINITE
                            ? ACTION_LEADERBOARD_INFINITE
                            : ACTION_LEADERBOARD_TIMER);

                    ++rank;

                } while (result->NextRow());
            }

            if (rank == 1)
            {
                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Aucun score enregistre pour ce mois.",
                    GOSSIP_SENDER_MAIN,
                    mode == MODE_INFINITE
                        ? ACTION_LEADERBOARD_INFINITE
                        : ACTION_LEADERBOARD_TIMER);
            }

            AddGossipItemFor(
                player,
                GOSSIP_ICON_CHAT,
                "< Retour au menu principal",
                GOSSIP_SENDER_MAIN,
                ACTION_BACK_MAIN);

            SendGossipMenuFor(
                player,
                68,
                creature->GetGUID());
        }

        bool StartSession(
            Player* starter,
            Creature* arenaMaster,
            ArenaMode mode,
            WaveProgression progression,
            std::string& error)
        {
            if (!starter)
            {
                error = "Joueur invalide.";
                return false;
            }

            if (!arenaMaster)
            {
                error = "Maitre de l'Arene invalide.";
                return false;
            }

            if (IsPlayerInArena(starter))
            {
                error = "Tu participes deja a une arene.";
                return false;
            }

            std::vector<Player*> members;

            if (!CollectEligiblePlayers(starter, members, error))
                return false;

            uint32 phaseMask = GetFreePhaseMask();

            if (phaseMask == 0)
            {
                error =
                    "Toutes les phases de l'arene sont actuellement utilisees.";
                return false;
            }

            Session session;

            session.id = ++_nextSessionId;
            session.mode = mode;
            session.progression = progression;

            session.phaseMask = phaseMask;
            session.wave = 0;
            session.highestLevel = 1;
            session.playerCount =
                static_cast<uint32>(members.size());

            BuildGroupIdentity(
                members,
                session.groupKey,
                session.groupNames);

            // Mémorise le PNJ qui a réellement lancé cette session.
            // Ainsi un Maître 32748 placé à Hurlevent, Orgrimmar, etc.
            // devient automatiquement le point de retour de son groupe.
            session.returnMapId = arenaMaster->GetMapId();
            session.returnO = arenaMaster->GetOrientation();
            session.returnX = arenaMaster->GetPositionX() +
                std::cos(session.returnO) * RETURN_IN_FRONT_OF_NPC_DISTANCE;
            session.returnY = arenaMaster->GetPositionY() +
                std::sin(session.returnO) * RETURN_IN_FRONT_OF_NPC_DISTANCE;
            session.returnZ = arenaMaster->GetPositionZ();

            session.started = true;
            session.completed = false;

            // V1.2 : ne pas démarrer les vagues tant que tous les joueurs
            // n'ont pas réellement terminé leur téléport inter-map.
            session.waitingForArrival = true;
            session.arrivalTimeoutMs = ARRIVAL_TIMEOUT_MS;

            session.waitingNextWave = false;
            session.nextWaveDelayMs = 0;
            session.fixedWaveTimerMs = 0;

            if (mode == MODE_TIMER)
                session.remainingMs = TIMER_DURATION_MS;

            for (Player* member : members)
            {
                uint32 memberLevel = member->GetLevel();

                if (memberLevel > session.highestLevel)
                    session.highestLevel = memberLevel;

                PlayerSnapshot snapshot;

                snapshot.guid = member->GetGUID();
                snapshot.oldMapId = member->GetMapId();

                snapshot.oldX = member->GetPositionX();
                snapshot.oldY = member->GetPositionY();
                snapshot.oldZ = member->GetPositionZ();
                snapshot.oldO = member->GetOrientation();

                snapshot.oldPhaseMask =
                    member->GetPhaseMask();

                session.players.push_back(
                    member->GetGUID());

                session.snapshots.push_back(
                    snapshot);

                _playerToSession[
                    member->GetGUID().GetCounter()] =
                    session.id;
            }

            uint64 sessionId = session.id;

            _sessions[sessionId] = session;

            uint32 index = 0;
            uint32 total =
                static_cast<uint32>(members.size());

            for (Player* member : members)
            {
                Position startPosition =
                    GetPlayerStartPosition(index, total);

                ++index;

                // IMPORTANT V1.2 :
                // on ne change PAS la phase avant le TeleportTo.
                // Le joueur garde sa phase actuelle pendant le chargement.
                member->TeleportTo(
                    ARENA_MAP_ID,
                    startPosition.GetPositionX(),
                    startPosition.GetPositionY(),
                    startPosition.GetPositionZ(),
                    startPosition.GetOrientation());

                SendMessage(
                    member,
                    "|cff00ff00[Blood Arena]|r Session creee.");

                if (mode == MODE_INFINITE)
                {
                    SendMessage(
                        member,
                        "|cff00ff00[Blood Arena]|r Mode : vagues infinies.");
                }
                else
                {
                    SendMessage(
                        member,
                        "|cff00ff00[Blood Arena]|r Mode : 30 minutes / 15 vagues.");
                }

                if (progression == PROGRESSION_LAST_KILL)
                {
                    SendMessage(
                        member,
                        "|cff00ff00[Blood Arena]|r Vague suivante a la mort du dernier ennemi.");
                }
                else
                {
                    SendMessage(
                        member,
                        "|cff00ff00[Blood Arena]|r Nouvelle vague toutes les 2 minutes.");
                }

                SendMessage(
                    member,
                    "|cffff0000[Blood Arena]|r Aucun point d'experience pendant l'evenement.");

                SendMessage(
                    member,
                    "|cff00ffff[Blood Arena]|r Transfert vers l'arene en cours...");

                SendMessage(
                    member,
                    "|cffff8000[Blood Arena]|r Pour abandonner l'evenement, emprunte l'une des deux portes : tout le groupe sera renvoye devant le Maitre de l'Arene.");
            }

            return true;
        }

        void Update(uint32 diff)
        {
            UpdateMonthlyRollover(diff);

            std::vector<uint64> victories;

            std::vector<
                std::pair<uint64, std::string>
            > failures;

            // Sorties volontaires par les portes. Elles sont traitées
            // après la boucle des sessions pour ne pas invalider _sessions.
            std::vector<uint64> doorExits;

            for (auto& pair : _sessions)
            {
                Session& session = pair.second;

                if (!session.started ||
                    session.completed)
                {
                    continue;
                }

                // -------------------------------------------------------------
                // V1.2 : attendre la fin réelle du chargement inter-map
                // -------------------------------------------------------------

                if (session.waitingForArrival)
                {
                    bool allArrived = true;

                    for (ObjectGuid const& guid :
                         session.players)
                    {
                        Player* player =
                            ObjectAccessor::FindConnectedPlayer(
                                guid);

                        if (!player ||
                            !player->IsInWorld())
                        {
                            allArrived = false;
                            continue;
                        }

                        if (player->GetMapId() !=
                            ARENA_MAP_ID)
                        {
                            allArrived = false;
                            continue;
                        }

                        // Le joueur est réellement arrivé dans l'arène.
                        // On applique seulement maintenant la phase de session.
                        if (player->GetPhaseMask() !=
                            session.phaseMask)
                        {
                            player->SetPhaseMask(
                                session.phaseMask,
                                true);
                        }
                    }

                    if (allArrived)
                    {
                        session.waitingForArrival = false;

                        session.waitingNextWave = true;
                        session.nextWaveDelayMs =
                            START_DELAY_MS;

                        Broadcast(
                            session.id,
                            "|cff00ff00[Blood Arena]|r Tous les participants sont arrives. Debut dans 3 secondes.");
                    }
                    else
                    {
                        if (session.arrivalTimeoutMs <=
                            diff)
                        {
                            session.arrivalTimeoutMs = 0;

                            failures.push_back(
                                std::make_pair(
                                    session.id,
                                    "Echec : un participant n'a pas pu rejoindre l'arene."));
                        }
                        else
                        {
                            session.arrivalTimeoutMs -=
                                diff;
                        }
                    }

                    // Très important :
                    // aucun test de wipe / aucune vague pendant le chargement.
                    continue;
                }

                // -------------------------------------------------------------
                // Session active
                // -------------------------------------------------------------

                if (session.configurationError)
                {
                    failures.push_back(
                        std::make_pair(
                            session.id,
                            "Erreur de configuration : aucun monstre valide n'est disponible pour cette vague."));

                    continue;
                }

                // Un joueur qui rejoint volontairement l'une des deux portes
                // quitte l'évènement. Sa phase normale sera rétablie juste
                // avant la porte afin qu'il puisse continuer à pied.
                for (ObjectGuid const& guid : session.players)
                {
                    Player* player =
                        ObjectAccessor::FindConnectedPlayer(guid);

                    if (!player || !player->IsInWorld())
                        continue;

                    if (player->GetMapId() != ARENA_MAP_ID)
                        continue;

                    if (player->GetPhaseMask() != session.phaseMask)
                        continue;

                    if (IsPlayerAtExitDoor(player))
                    {
                        // Une seule personne à la porte suffit :
                        // tout le groupe quitte l'évènement.
                        doorExits.push_back(session.id);
                        break;
                    }
                }

                if (!HasAtLeastOneOnlinePlayer(
                        session))
                {
                    failures.push_back(
                        std::make_pair(
                            session.id,
                            "Tous les joueurs ont quitte la session."));

                    continue;
                }

                if (!HasAtLeastOneAlivePlayerInsideArena(
                        session))
                {
                    failures.push_back(
                        std::make_pair(
                            session.id,
                            "Defaite : tous les participants sont morts ou ont quitte l'arene."));

                    continue;
                }

                if (session.mode == MODE_TIMER)
                {
                    if (session.remainingMs <= diff)
                    {
                        session.remainingMs = 0;

                        failures.push_back(
                            std::make_pair(
                                session.id,
                                "Defaite : les 30 minutes sont ecoulees."));

                        continue;
                    }

                    session.remainingMs -= diff;
                }

                if (session.waitingNextWave)
                {
                    if (session.nextWaveDelayMs <= diff)
                    {
                        session.nextWaveDelayMs = 0;
                        SpawnNextWave(session);
                    }
                    else
                    {
                        session.nextWaveDelayMs -= diff;
                    }

                    continue;
                }

                uint32 alive =
                    CountAliveSummons(session);

                // Récompenses vagues infinies.
                ProcessInfiniteWaveRewards(session);

                alive =
                    CountAliveSummons(session);

                if (alive == 0 &&
                    HasReachedFinalWave(session))
                {
                    victories.push_back(session.id);
                    continue;
                }

                if (session.progression ==
                    PROGRESSION_LAST_KILL)
                {
                    if (alive == 0)
                        QueueNextWave(session);
                }
                else
                {
                    if (HasReachedFinalWave(session))
                        continue;

                    if (session.fixedWaveTimerMs <=
                        diff)
                    {
                        session.fixedWaveTimerMs = 0;

                        SpawnNextWave(session);
                    }
                    else
                    {
                        session.fixedWaveTimerMs -=
                            diff;
                    }
                }
            }

            for (uint64 sessionId :
                 victories)
            {
                EndSession(
                    sessionId,
                    true,
                    "Victoire !");
            }

            for (auto const& failure :
                 failures)
            {
                EndSession(
                    failure.first,
                    false,
                    failure.second);
            }

            for (uint64 sessionId : doorExits)
            {
                ExitGroupThroughDoor(sessionId);
            }
        }

        void EndSession(
            uint64 sessionId,
            bool success,
            std::string const& reason)
        {
            auto sessionIt =
                _sessions.find(sessionId);

            if (sessionIt ==
                _sessions.end())
            {
                return;
            }

            Session& session =
                sessionIt->second;

            session.completed = true;

            if (success && session.mode == MODE_TIMER)
                SaveTimerScore(session);

            CleanupSummons(session);

            uint32 timerReward = 0;
            uint32 elapsedMs = 0;

            if (success &&
                session.mode == MODE_TIMER)
            {
                elapsedMs =
                    TIMER_DURATION_MS -
                    session.remainingMs;

                if (elapsedMs <
                    10 * 60 * 1000)
                {
                    timerReward =
                        TIMER_REWARD_UNDER_10_MIN;
                }
                else if (elapsedMs <
                         20 * 60 * 1000)
                {
                    timerReward =
                        TIMER_REWARD_UNDER_20_MIN;
                }
                else
                {
                    timerReward =
                        TIMER_REWARD_20_TO_30_MIN;
                }
            }

            for (PlayerSnapshot const& snapshot :
                 session.snapshots)
            {
                _playerToSession.erase(
                    snapshot.guid.GetCounter());

                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        snapshot.guid);

                if (!player)
                    continue;

                if (timerReward > 0)
                {
                    GiveItemReward(
                        player,
                        ITEM_INFUSION_CRYSTAL,
                        timerReward,
                        "Cristaux d'infusion");

                    uint32 elapsedSeconds =
                        elapsedMs / 1000;

                    uint32 elapsedMinutes =
                        elapsedSeconds / 60;

                    uint32 elapsedRemainder =
                        elapsedSeconds % 60;

                    std::ostringstream message;

                    message
                        << "|cff00ff00[Blood Arena]|r Defi Timer termine en "
                        << elapsedMinutes
                        << "m "
                        << elapsedRemainder
                        << "s : +"
                        << timerReward
                        << " Cristaux d'infusion.";

                    SendMessage(
                        player,
                        message.str());
                }

                uint32 restorePhase =
                    snapshot.oldPhaseMask != 0
                        ? snapshot.oldPhaseMask
                        : 1;

                player->SetPhaseMask(
                    restorePhase,
                    true);

                player->TeleportTo(
                    snapshot.oldMapId,
                    snapshot.oldX,
                    snapshot.oldY,
                    snapshot.oldZ,
                    snapshot.oldO);

                if (success)
                {
                    SendMessage(
                        player,
                        "|cff00ff00[Blood Arena]|r " +
                        reason);
                }
                else
                {
                    SendMessage(
                        player,
                        "|cffff0000[Blood Arena]|r " +
                        reason);
                }
            }

            _sessions.erase(sessionIt);
        }

    private:
        ArenaManager()
            : _nextSessionId(0),
              _monthlyCheckTimerMs(0)
        {
        }

        std::unordered_map<
            uint64,
            Session
        > _sessions;

        std::unordered_map<
            ObjectGuid::LowType,
            uint64
        > _playerToSession;

        uint64 _nextSessionId;
        uint32 _monthlyCheckTimerMs;

        std::vector<ArenaCreaturePoolEntry> _trashPool;
        std::vector<ArenaCreaturePoolEntry> _bossPool;

        // ---------------------------------------------------------------------
        // Pool SQL des monstres / boss
        // ---------------------------------------------------------------------

        void SeedDefaultCreaturePoolIfEmpty()
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT `id` FROM `blood_arena_creatures` LIMIT 1");

            if (result)
                return;

            // Pool de la V1.5 utilisé comme valeurs initiales.
            WorldDatabase.DirectExecute(
                "INSERT INTO `blood_arena_creatures` "
                "(`creature_type`,`creature_entry`,`mode_mask`,`min_wave`,`max_wave`,`weight`,`enabled`,`sort_order`,`note`) "
                "VALUES "
                "(1,3,3,1,0,1,1,10,'Flesh Eater'),"
                "(1,30,3,1,0,1,1,20,'Forest Spider'),"
                "(1,38,3,1,0,1,1,30,'Defias Thug'),"
                "(1,46,3,1,0,1,1,40,'Murloc Forager'),"
                "(1,48,3,1,0,1,1,50,'Skeletal Warrior'),"
                "(1,92,3,1,0,1,1,60,'Rock Elemental'),"
                "(2,1720,3,1,0,1,1,10,'Bruegal Ironknuckle'),"
                "(2,1841,3,1,0,1,1,20,'Scarlet Executioner'),"
                "(2,1850,3,1,0,1,1,30,'Putridius')");
        }

        uint8 GetModeMask(ArenaMode mode) const
        {
            return
                mode == MODE_INFINITE
                    ? ARENA_MODE_MASK_INFINITE
                    : ARENA_MODE_MASK_TIMER;
        }

        uint32 SelectCreatureFromPool(
            std::vector<ArenaCreaturePoolEntry> const& pool,
            ArenaMode mode,
            uint32 wave,
            uint32 selectionIndex) const
        {
            uint8 requiredModeMask = GetModeMask(mode);
            uint64 totalWeight = 0;

            for (ArenaCreaturePoolEntry const& poolEntry : pool)
            {
                if ((poolEntry.modeMask & requiredModeMask) == 0)
                    continue;

                if (wave < poolEntry.minWave)
                    continue;

                if (poolEntry.maxWave > 0 &&
                    wave > poolEntry.maxWave)
                {
                    continue;
                }

                totalWeight += poolEntry.weight;
            }

            if (totalWeight == 0)
                return 0;

            // Sélection pondérée déterministe :
            // le poids fonctionne sans dépendre d'un générateur aléatoire,
            // et l'ordre reste stable après un reload SQL.
            uint64 ticket =
                static_cast<uint64>(selectionIndex) %
                totalWeight;

            for (ArenaCreaturePoolEntry const& poolEntry : pool)
            {
                if ((poolEntry.modeMask & requiredModeMask) == 0)
                    continue;

                if (wave < poolEntry.minWave)
                    continue;

                if (poolEntry.maxWave > 0 &&
                    wave > poolEntry.maxWave)
                {
                    continue;
                }

                if (ticket < poolEntry.weight)
                    return poolEntry.creatureEntry;

                ticket -= poolEntry.weight;
            }

            return 0;
        }

        // ---------------------------------------------------------------------
        // Classement mensuel / base de données
        // ---------------------------------------------------------------------

        std::string FormatDuration(uint32 milliseconds) const
        {
            uint32 totalSeconds = milliseconds / 1000;
            uint32 minutes = totalSeconds / 60;
            uint32 seconds = totalSeconds % 60;

            std::ostringstream stream;
            stream << minutes << "m ";

            if (seconds < 10)
                stream << "0";

            stream << seconds << "s";
            return stream.str();
        }

        void BuildGroupIdentity(
            std::vector<Player*> const& members,
            std::string& groupKey,
            std::string& groupNames) const
        {
            std::vector<
                std::pair<uint64, std::string>
            > identities;

            for (Player* member : members)
            {
                if (!member)
                    continue;

                identities.push_back(
                    std::make_pair(
                        static_cast<uint64>(
                            member->GetGUID().GetCounter()),
                        member->GetName()));
            }

            std::sort(
                identities.begin(),
                identities.end(),
                [](std::pair<uint64, std::string> const& a,
                   std::pair<uint64, std::string> const& b)
                {
                    return a.first < b.first;
                });

            std::ostringstream keyStream;
            std::ostringstream nameStream;

            for (size_t i = 0; i < identities.size(); ++i)
            {
                if (i > 0)
                {
                    keyStream << "-";
                    nameStream << ", ";
                }

                keyStream << identities[i].first;
                nameStream << identities[i].second;
            }

            groupKey = keyStream.str();
            groupNames = nameStream.str();
        }

        void SaveInfiniteScore(Session const& session)
        {
            if (session.mode != MODE_INFINITE ||
                session.bestClearedWave == 0 ||
                session.groupKey.empty())
            {
                return;
            }

            std::string season = GetCurrentSeasonKey();
            std::string groupKey = session.groupKey;
            std::string groupNames = session.groupNames;

            WorldDatabase.EscapeString(groupKey);
            WorldDatabase.EscapeString(groupNames);

            // L'UPDATE conditionnel garantit que le mode de progression
            // correspond toujours au meilleur score réellement enregistré.
            WorldDatabase.PExecute(
                "INSERT INTO `blood_arena_scores` "
                "(`season_key`,`mode`,`group_key`,`group_names`,`player_count`,"
                "`progression`,`best_wave`,`best_time_ms`) "
                "VALUES ('{}',{},'{}','{}',{},{},{},0) "
                "ON DUPLICATE KEY UPDATE "
                "`group_names`=VALUES(`group_names`),"
                "`player_count`=VALUES(`player_count`),"
                "`updated_at`=IF(VALUES(`best_wave`)>`best_wave`,CURRENT_TIMESTAMP,`updated_at`),"
                "`progression`=IF(VALUES(`best_wave`)>`best_wave`,VALUES(`progression`),`progression`),"
                "`best_wave`=GREATEST(`best_wave`,VALUES(`best_wave`))",
                season,
                uint32(MODE_INFINITE),
                groupKey,
                groupNames,
                session.playerCount,
                uint32(session.progression),
                session.bestClearedWave);
        }

        void SaveTimerScore(Session const& session)
        {
            if (session.mode != MODE_TIMER ||
                session.wave < TIMER_TOTAL_WAVES ||
                session.groupKey.empty())
            {
                return;
            }

            uint32 elapsedMs =
                TIMER_DURATION_MS - session.remainingMs;

            if (elapsedMs == 0)
                return;

            std::string season = GetCurrentSeasonKey();
            std::string groupKey = session.groupKey;
            std::string groupNames = session.groupNames;

            WorldDatabase.EscapeString(groupKey);
            WorldDatabase.EscapeString(groupNames);

            WorldDatabase.PExecute(
                "INSERT INTO `blood_arena_scores` "
                "(`season_key`,`mode`,`group_key`,`group_names`,`player_count`,"
                "`progression`,`best_wave`,`best_time_ms`) "
                "VALUES ('{}',{},'{}','{}',{},{},{},{}) "
                "ON DUPLICATE KEY UPDATE "
                "`group_names`=VALUES(`group_names`),"
                "`player_count`=VALUES(`player_count`),"
                "`updated_at`=IF(`best_time_ms`=0 OR VALUES(`best_time_ms`)<`best_time_ms`,CURRENT_TIMESTAMP,`updated_at`),"
                "`progression`=IF(`best_time_ms`=0 OR VALUES(`best_time_ms`)<`best_time_ms`,VALUES(`progression`),`progression`),"
                "`best_time_ms`=IF(`best_time_ms`=0 OR VALUES(`best_time_ms`)<`best_time_ms`,VALUES(`best_time_ms`),`best_time_ms`)",
                season,
                uint32(MODE_TIMER),
                groupKey,
                groupNames,
                session.playerCount,
                uint32(session.progression),
                TIMER_TOTAL_WAVES,
                elapsedMs);
        }

        void MarkGmNoticeRead(
            std::string const& seasonKey,
            uint32 accountId)
        {
            WorldDatabase.PExecute(
                "INSERT IGNORE INTO `blood_arena_gm_notice_read` "
                "(`season_key`,`account_id`) VALUES ('{}',{})",
                seasonKey,
                accountId);
        }

        void NotifyOnlineGms(
            std::string const& seasonKey,
            std::string const& noticeText)
        {
            for (auto const& pair :
                 sWorld->GetAllSessions())
            {
                WorldSession* session = pair.second;

                if (!session ||
                    session->GetSecurity() < SEC_GAMEMASTER)
                {
                    continue;
                }

                ChatHandler(session).
                    SendSysMessage(
                        noticeText.c_str());

                MarkGmNoticeRead(
                    seasonKey,
                    session->GetAccountId());
            }
        }

        void ArchiveAndNotifySeason(
            std::string const& seasonKey)
        {
            std::string infiniteNames;
            uint32 infiniteWave = 0;
            uint32 infiniteProgression = 1;

            QueryResult infiniteResult =
                WorldDatabase.PQuery(
                    "SELECT `group_names`,`best_wave`,`progression` "
                    "FROM `blood_arena_scores` "
                    "WHERE `season_key`='{}' AND `mode`={} AND `best_wave`>0 "
                    "ORDER BY `best_wave` DESC, `updated_at` ASC LIMIT 1",
                    seasonKey,
                    uint32(MODE_INFINITE));

            if (infiniteResult)
            {
                Field* fields = infiniteResult->Fetch();
                infiniteNames = fields[0].GetString();
                infiniteWave = fields[1].GetUInt32();
                infiniteProgression = fields[2].GetUInt32();

                std::string escaped = infiniteNames;
                WorldDatabase.EscapeString(escaped);

                WorldDatabase.DirectPExecute(
                    "REPLACE INTO `blood_arena_monthly_winners` "
                    "(`season_key`,`mode`,`group_names`,`score_value`,`progression`) "
                    "VALUES ('{}',{},'{}',{},{})",
                    seasonKey,
                    uint32(MODE_INFINITE),
                    escaped,
                    infiniteWave,
                    infiniteProgression);
            }

            std::string timerNames;
            uint32 timerTime = 0;
            uint32 timerProgression = 1;

            QueryResult timerResult =
                WorldDatabase.PQuery(
                    "SELECT `group_names`,`best_time_ms`,`progression` "
                    "FROM `blood_arena_scores` "
                    "WHERE `season_key`='{}' AND `mode`={} AND `best_time_ms`>0 "
                    "ORDER BY `best_time_ms` ASC, `updated_at` ASC LIMIT 1",
                    seasonKey,
                    uint32(MODE_TIMER));

            if (timerResult)
            {
                Field* fields = timerResult->Fetch();
                timerNames = fields[0].GetString();
                timerTime = fields[1].GetUInt32();
                timerProgression = fields[2].GetUInt32();

                std::string escaped = timerNames;
                WorldDatabase.EscapeString(escaped);

                WorldDatabase.DirectPExecute(
                    "REPLACE INTO `blood_arena_monthly_winners` "
                    "(`season_key`,`mode`,`group_names`,`score_value`,`progression`) "
                    "VALUES ('{}',{},'{}',{},{})",
                    seasonKey,
                    uint32(MODE_TIMER),
                    escaped,
                    timerTime,
                    timerProgression);
            }

            std::ostringstream notice;

            notice
                << "|cffff8000[Blood Arena - GM]|r Classement du mois "
                << GetSeasonLabel(seasonKey)
                << " termine. Merci d'attribuer manuellement la recompense aux meilleurs groupes.";

            if (!infiniteNames.empty())
            {
                notice
                    << " | Infini #1 : "
                    << infiniteNames
                    << " - vague "
                    << infiniteWave
                    << ".";
            }
            else
            {
                notice
                    << " | Infini : aucun score.";
            }

            if (!timerNames.empty())
            {
                notice
                    << " | Timer #1 : "
                    << timerNames
                    << " - "
                    << FormatDuration(timerTime)
                    << ".";
            }
            else
            {
                notice
                    << " | Timer : aucun score.";
            }

            std::string noticeText = notice.str();
            std::string escapedNotice = noticeText;

            WorldDatabase.EscapeString(escapedNotice);

            WorldDatabase.DirectPExecute(
                "REPLACE INTO `blood_arena_gm_notices` "
                "(`season_key`,`notice_text`) VALUES ('{}','{}')",
                seasonKey,
                escapedNotice);

            NotifyOnlineGms(
                seasonKey,
                noticeText);
        }

        void CheckMonthlyRollover()
        {
            std::string currentSeason =
                GetCurrentSeasonKey();

            QueryResult result =
                WorldDatabase.Query(
                    "SELECT `current_season` "
                    "FROM `blood_arena_state` "
                    "WHERE `id`=1");

            if (!result)
            {
                WorldDatabase.DirectPExecute(
                    "INSERT INTO `blood_arena_state` "
                    "(`id`,`current_season`) VALUES (1,'{}')",
                    currentSeason);

                return;
            }

            std::string storedSeason =
                result->Fetch()[0].GetString();

            if (storedSeason == currentSeason)
                return;

            if (!storedSeason.empty() &&
                storedSeason != "000000")
            {
                ArchiveAndNotifySeason(
                    storedSeason);
            }

            WorldDatabase.DirectPExecute(
                "UPDATE `blood_arena_state` "
                "SET `current_season`='{}' "
                "WHERE `id`=1",
                currentSeason);

            SC_LOG_INFO(
                "server.worldserver",
                "[BloodArena] Nouveau mois : classement {} initialise.",
                currentSeason);
        }

        void UpdateMonthlyRollover(uint32 diff)
        {
            if (_monthlyCheckTimerMs <= diff)
            {
                _monthlyCheckTimerMs =
                    MONTH_CHECK_INTERVAL_MS;

                CheckMonthlyRollover();
            }
            else
            {
                _monthlyCheckTimerMs -= diff;
            }
        }

        // ---------------------------------------------------------------------
        // Messages
        // ---------------------------------------------------------------------

        void SendMessage(
            Player* player,
            std::string const& message) const
        {
            if (!player ||
                !player->GetSession())
            {
                return;
            }

            ChatHandler(
                player->GetSession()).
                SendSysMessage(
                    message.c_str());
        }

        void Broadcast(
            uint64 sessionId,
            std::string const& message) const
        {
            auto sessionIt =
                _sessions.find(sessionId);

            if (sessionIt ==
                _sessions.end())
            {
                return;
            }

            for (ObjectGuid const& guid :
                 sessionIt->second.players)
            {
                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        guid);

                if (player)
                    SendMessage(player, message);
            }
        }

        // ---------------------------------------------------------------------
        // Récompenses
        // ---------------------------------------------------------------------

        void GiveItemReward(
            Player* player,
            uint32 itemId,
            uint64 amount,
            char const* rewardName) const
        {
            if (!player ||
                amount == 0)
            {
                return;
            }

            while (amount > 0)
            {
                uint32 chunk =
                    amount > 1000000000ULL
                        ? 1000000000U
                        : static_cast<uint32>(
                            amount);

                if (!player->AddItem(
                        itemId,
                        chunk))
                {
                    player->SendItemRetrievalMail(
                        itemId,
                        chunk);
                }

                amount -= chunk;
            }

            if (rewardName)
            {
                std::ostringstream message;

                message
                    << "|cff00ff00[Blood Arena]|r Recompense recue : "
                    << rewardName
                    << ".";

                SendMessage(
                    player,
                    message.str());
            }
        }

        uint64 GetDoubledBossReward(
            uint64 baseAmount,
            uint32 bossNumber) const
        {
            if (bossNumber <= 1)
                return baseAmount;

            uint64 value = baseAmount;

            for (uint32 i = 1;
                 i < bossNumber;
                 ++i)
            {
                if (value >=
                    MAX_SINGLE_ITEM_REWARD / 2)
                {
                    return
                        MAX_SINGLE_ITEM_REWARD;
                }

                value *= 2;
            }

            if (value >
                MAX_SINGLE_ITEM_REWARD)
            {
                value =
                    MAX_SINGLE_ITEM_REWARD;
            }

            return value;
        }

        void RewardInfiniteWave(
            Session& session,
            uint32 wave)
        {
            if (session.mode !=
                    MODE_INFINITE ||
                wave == 0)
            {
                return;
            }

            bool bossWave =
                (wave %
                 INFINITE_BOSS_EVERY) == 0;

            if (wave > session.bestClearedWave)
            {
                session.bestClearedWave = wave;
                SaveInfiniteScore(session);
            }

            uint64 crystalReward =
                INFINITE_NORMAL_WAVE_CRYSTALS;

            uint64 shardReward = 0;

            if (bossWave)
            {
                uint32 bossNumber =
                    wave /
                    INFINITE_BOSS_EVERY;

                crystalReward =
                    GetDoubledBossReward(
                        FIRST_BOSS_CRYSTALS,
                        bossNumber);

                shardReward =
                    GetDoubledBossReward(
                        FIRST_BOSS_SHARDS,
                        bossNumber);
            }

            for (ObjectGuid const& guid :
                 session.players)
            {
                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        guid);

                if (!player)
                    continue;

                GiveItemReward(
                    player,
                    ITEM_INFUSION_CRYSTAL,
                    crystalReward,
                    "Cristaux d'infusion");

                if (shardReward > 0)
                {
                    GiveItemReward(
                        player,
                        ITEM_STONE_KEEPER_SHARD,
                        shardReward,
                        "Eclats du gardien des pierres");
                }

                std::ostringstream message;

                if (bossWave)
                {
                    message
                        << "|cffff8000[Blood Arena]|r Boss de la vague "
                        << wave
                        << " vaincu : +"
                        << crystalReward
                        << " Cristaux d'infusion et +"
                        << shardReward
                        << " Eclats du gardien des pierres.";
                }
                else
                {
                    message
                        << "|cff00ff00[Blood Arena]|r Vague "
                        << wave
                        << " terminee : +"
                        << crystalReward
                        << " Cristaux d'infusion.";
                }

                SendMessage(
                    player,
                    message.str());
            }
        }

        // ---------------------------------------------------------------------
        // Sortie volontaire par les portes
        // ---------------------------------------------------------------------

        bool IsPlayerAtExitDoor(Player* player) const
        {
            if (!player)
                return false;

            for (uint32 i = 0; i < ARENA_DOOR_COUNT; ++i)
            {
                ArenaDoor const& door = ARENA_DOORS[i];

                float dx = door.x - ARENA_CENTER_X;
                float dy = door.y - ARENA_CENTER_Y;
                float length = std::sqrt(dx * dx + dy * dy);

                if (length <= 0.001f)
                    continue;

                float nx = dx / length;
                float ny = dy / length;

                // Point légèrement à l'intérieur de la porte.
                float triggerX =
                    door.x - nx * EXIT_TRIGGER_INNER_OFFSET;

                float triggerY =
                    door.y - ny * EXIT_TRIGGER_INNER_OFFSET;

                if (player->GetDistance2d(triggerX, triggerY) <=
                    EXIT_TRIGGER_RADIUS)
                {
                    return true;
                }
            }

            return false;
        }

        void ExitGroupThroughDoor(uint64 sessionId)
        {
            auto sessionIt =
                _sessions.find(sessionId);

            if (sessionIt == _sessions.end())
                return;

            Session& session =
                sessionIt->second;

            session.completed = true;

            // Une sortie volontaire ne donne aucune récompense de fin.
            CleanupSummons(session);

            uint32 index = 0;
            uint32 total =
                static_cast<uint32>(session.snapshots.size());

            for (PlayerSnapshot const& snapshot : session.snapshots)
            {
                _playerToSession.erase(
                    snapshot.guid.GetCounter());

                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        snapshot.guid);

                if (!player)
                {
                    ++index;
                    continue;
                }

                uint32 restorePhase =
                    snapshot.oldPhaseMask != 0
                        ? snapshot.oldPhaseMask
                        : 1;

                player->SetPhaseMask(
                    restorePhase,
                    true);

                // Petite dispersion en demi-cercle devant le PNJ
                // pour éviter que tout le groupe arrive exactement empilé.
                float angleOffset = 0.0f;

                if (total > 1)
                {
                    float centeredIndex =
                        static_cast<float>(index) -
                        (static_cast<float>(total - 1) / 2.0f);

                    angleOffset =
                        centeredIndex * 0.30f;
                }

                float distance =
                    RETURN_IN_FRONT_OF_NPC_DISTANCE +
                    (index % 2) * 0.7f;

                float finalOrientation =
                    session.returnO + angleOffset;

                float x =
                    session.returnX +
                    std::cos(finalOrientation) *
                    (distance - RETURN_IN_FRONT_OF_NPC_DISTANCE);

                float y =
                    session.returnY +
                    std::sin(finalOrientation) *
                    (distance - RETURN_IN_FRONT_OF_NPC_DISTANCE);

                player->TeleportTo(
                    session.returnMapId,
                    x,
                    y,
                    session.returnZ,
                    session.returnO);

                SendMessage(
                    player,
                    "|cffff8000[Blood Arena]|r Le groupe a quitte l'evenement et a ete renvoye devant le Maitre de l'Arene.");

                ++index;
            }

            _sessions.erase(sessionIt);
        }

        // ---------------------------------------------------------------------
        // Participants
        // ---------------------------------------------------------------------

        bool CollectEligiblePlayers(
            Player* starter,
            std::vector<Player*>& members,
            std::string& error)
        {
            Group* group =
                starter->GetGroup();

            if (!group)
            {
                members.push_back(starter);
                return true;
            }

            if (group->GetLeaderGUID() !=
                starter->GetGUID())
            {
                error =
                    "Seul le chef du groupe peut lancer l'evenement.";

                return false;
            }

            for (GroupReference* reference =
                     group->GetFirstMember();
                 reference;
                 reference = reference->next())
            {
                Player* member =
                    reference->GetSource();

                if (!member)
                    continue;

                if (!member->IsInWorld())
                {
                    error =
                        "Tous les membres du groupe doivent etre connectes.";

                    return false;
                }

                // Pour lancer depuis Hurlevent / Orgrimmar / etc.,
                // tout le groupe doit simplement être avec le chef.
                if (member->GetMapId() !=
                    starter->GetMapId())
                {
                    error =
                        "Tous les membres doivent etre sur la meme carte que le chef.";

                    return false;
                }

                if (!member->IsWithinDistInMap(
                        starter,
                        REQUIRED_GROUP_RANGE))
                {
                    error =
                        "Tous les membres doivent etre proches du Maitre de l'Arene.";

                    return false;
                }

                if (IsPlayerInArena(member))
                {
                    error =
                        "Un membre du groupe participe deja a une arene.";

                    return false;
                }

                members.push_back(member);
            }

            if (members.empty())
            {
                error =
                    "Aucun participant valide.";

                return false;
            }

            return true;
        }

        uint32 GetFreePhaseMask() const
        {
            for (uint32 phaseIndex = 0;
                 phaseIndex < PHASE_POOL_COUNT;
                 ++phaseIndex)
            {
                uint32 phase =
                    PHASE_POOL[phaseIndex];

                bool used = false;

                for (auto const& pair :
                     _sessions)
                {
                    Session const& session =
                        pair.second;

                    if (!session.completed &&
                        session.phaseMask ==
                            phase)
                    {
                        used = true;
                        break;
                    }
                }

                if (!used)
                    return phase;
            }

            return 0;
        }

        // ---------------------------------------------------------------------
        // Positions
        // ---------------------------------------------------------------------

        Position GetPlayerStartPosition(
            uint32 index,
            uint32 total) const
        {
            float divisor =
                total > 0
                    ? static_cast<float>(total)
                    : 1.0f;

            float angle =
                (static_cast<float>(index) /
                 divisor) *
                6.28318530718f;

            Position position;

            position.Relocate(
                ARENA_CENTER_X +
                    std::cos(angle) *
                    PLAYER_START_RADIUS,
                ARENA_CENTER_Y +
                    std::sin(angle) *
                    PLAYER_START_RADIUS,
                ARENA_CENTER_Z,
                angle +
                    3.14159265359f);

            return position;
        }

        Position GetMobSpawnPosition(
            uint32 index,
            uint32 wave) const
        {
            uint32 doorIndex =
                ARENA_DOOR_COUNT > 0
                    ? (wave + index) %
                        ARENA_DOOR_COUNT
                    : 0;

            ArenaDoor const& door =
                ARENA_DOORS[doorIndex];

            // Dispersion latérale.
            int32 lane =
                static_cast<int32>(
                    (index /
                     ARENA_DOOR_COUNT) %
                    5) -
                2;

            float lateralOffset =
                static_cast<float>(lane) *
                0.70f;

            float sideAngle =
                door.o +
                1.57079632679f;

            Position position;

            position.Relocate(
                door.x +
                    std::cos(sideAngle) *
                    lateralOffset,
                door.y +
                    std::sin(sideAngle) *
                    lateralOffset,
                door.z,
                door.o);

            return position;
        }

        // ---------------------------------------------------------------------
        // État des joueurs
        // ---------------------------------------------------------------------

        Player* GetAnyOnlinePlayer(
            Session const& session) const
        {
            for (ObjectGuid const& guid :
                 session.players)
            {
                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        guid);

                if (player &&
                    player->IsInWorld())
                {
                    return player;
                }
            }

            return nullptr;
        }

        bool HasAtLeastOneOnlinePlayer(
            Session const& session) const
        {
            return
                GetAnyOnlinePlayer(session) !=
                nullptr;
        }

        bool HasAtLeastOneAlivePlayerInsideArena(
            Session const& session) const
        {
            for (ObjectGuid const& guid :
                 session.players)
            {
                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        guid);

                if (!player ||
                    !player->IsInWorld())
                {
                    continue;
                }

                if (player->GetMapId() !=
                    ARENA_MAP_ID)
                {
                    continue;
                }

                if (player->GetPhaseMask() !=
                    session.phaseMask)
                {
                    continue;
                }

                if (player->IsAlive())
                    return true;
            }

            return false;
        }

        Player* GetNearestAliveParticipant(
            Session const& session,
            Creature* creature) const
        {
            if (!creature)
                return nullptr;

            Player* nearest = nullptr;
            float nearestDistance =
                999999.0f;

            for (ObjectGuid const& guid :
                 session.players)
            {
                Player* player =
                    ObjectAccessor::FindConnectedPlayer(
                        guid);

                if (!player ||
                    !player->IsInWorld() ||
                    !player->IsAlive())
                {
                    continue;
                }

                if (player->GetMapId() !=
                    ARENA_MAP_ID)
                {
                    continue;
                }

                if (player->GetPhaseMask() !=
                    session.phaseMask)
                {
                    continue;
                }

                float distance =
                    creature->GetDistance(player);

                if (!nearest ||
                    distance <
                        nearestDistance)
                {
                    nearest = player;
                    nearestDistance =
                        distance;
                }
            }

            return nearest;
        }

        // ---------------------------------------------------------------------
        // Vagues
        // ---------------------------------------------------------------------

        bool IsBossWave(
            Session const& session) const
        {
            if (session.wave == 0)
                return false;

            if (session.mode ==
                MODE_INFINITE)
            {
                return
                    (session.wave %
                     INFINITE_BOSS_EVERY) ==
                    0;
            }

            return
                (session.wave %
                 TIMER_BOSS_EVERY) ==
                0;
        }

        bool HasReachedFinalWave(
            Session const& session) const
        {
            if (session.mode ==
                MODE_TIMER)
            {
                return
                    session.wave >=
                    TIMER_TOTAL_WAVES;
            }

            return
                session.wave >=
                INFINITE_MAX_WAVES;
        }

        uint32 GetInfiniteTrashCount(
            Session const& session) const
        {
            // V1.2 :
            // vague 1 = 2 mobs par joueur.
            // Puis environ +25 % par vague de trash.
            // Les vagues boss ne font pas augmenter la quantité.
            // Plafond absolu = 30 mobs actifs.

            uint64 count =
                static_cast<uint64>(2) *
                session.playerCount;

            if (count == 0)
                count = 2;

            if (count >=
                MAX_ACTIVE_TRASH_INFINITE)
            {
                return
                    MAX_ACTIVE_TRASH_INFINITE;
            }

            for (uint32 wave = 2;
                 wave <= session.wave;
                 ++wave)
            {
                if ((wave %
                     INFINITE_BOSS_EVERY) ==
                    0)
                {
                    continue;
                }

                // floor(count * 1.25)
                uint64 nextCount =
                    (count * 5) / 4;

                // Évite que l'arrondi bloque la progression.
                if (nextCount <= count)
                    ++nextCount;

                count = nextCount;

                if (count >=
                    MAX_ACTIVE_TRASH_INFINITE)
                {
                    return
                        MAX_ACTIVE_TRASH_INFINITE;
                }
            }

            return
                static_cast<uint32>(count);
        }

        uint32 GetTimerTrashCount(
            Session const& session) const
        {
            uint32 count =
                3 + session.wave;

            if (session.playerCount > 1)
            {
                count +=
                    (session.playerCount - 1) *
                    2;
            }

            if (count >
                MAX_ACTIVE_TRASH_TIMER)
            {
                count =
                    MAX_ACTIVE_TRASH_TIMER;
            }

            return count;
        }

        uint32 GetDesiredSpawnCount(
            Session const& session) const
        {
            if (IsBossWave(session))
                return 1;

            if (session.mode ==
                MODE_INFINITE)
            {
                return
                    GetInfiniteTrashCount(
                        session);
            }

            return
                GetTimerTrashCount(
                    session);
        }

        uint32 GetTrashEntry(
            Session const& session,
            uint32 index) const
        {
            return SelectCreatureFromPool(
                _trashPool,
                session.mode,
                session.wave,
                session.wave + index);
        }

        uint32 GetBossEntry(
            Session const& session) const
        {
            uint32 bossNumber = 1;

            if (session.mode == MODE_INFINITE)
            {
                bossNumber =
                    session.wave /
                    INFINITE_BOSS_EVERY;
            }
            else
            {
                bossNumber =
                    session.wave /
                    TIMER_BOSS_EVERY;
            }

            if (bossNumber == 0)
                bossNumber = 1;

            return SelectCreatureFromPool(
                _bossPool,
                session.mode,
                session.wave,
                bossNumber - 1);
        }

        // ---------------------------------------------------------------------
        // Scaling
        // ---------------------------------------------------------------------

        void ApplyScaling(
            Creature* creature,
            Session const& session,
            bool boss) const
        {
            if (!creature)
                return;

            uint32 level =
                session.highestLevel > 0
                    ? session.highestLevel
                    : 1;

            if (level > 255)
                level = 255;

            creature->SetLevel(
                static_cast<uint8>(level));

            creature->
                UpdateLevelDependantStats();

            float healthScale = 1.0f;
            float damageScale = 1.0f;
            float armorScale = 1.0f;

            if (session.playerCount > 1)
            {
                uint32 extraPlayers =
                    session.playerCount - 1;

                healthScale +=
                    static_cast<float>(
                        extraPlayers) *
                    GROUP_HEALTH_PER_EXTRA_PLAYER;

                damageScale +=
                    static_cast<float>(
                        extraPlayers) *
                    GROUP_DAMAGE_PER_EXTRA_PLAYER;

                armorScale +=
                    static_cast<float>(
                        extraPlayers) *
                    GROUP_ARMOR_PER_EXTRA_PLAYER;
            }

            uint32 completedWaves =
                session.wave > 0
                    ? session.wave - 1
                    : 0;

            if (session.mode ==
                MODE_INFINITE)
            {
                healthScale +=
                    static_cast<float>(
                        completedWaves) *
                    INFINITE_HEALTH_PER_WAVE;

                damageScale +=
                    static_cast<float>(
                        completedWaves) *
                    INFINITE_DAMAGE_PER_WAVE;

                armorScale +=
                    static_cast<float>(
                        completedWaves) *
                    INFINITE_ARMOR_PER_WAVE;
            }
            else
            {
                healthScale +=
                    static_cast<float>(
                        completedWaves) *
                    TIMER_HEALTH_PER_WAVE;

                damageScale +=
                    static_cast<float>(
                        completedWaves) *
                    TIMER_DAMAGE_PER_WAVE;

                armorScale +=
                    static_cast<float>(
                        completedWaves) *
                    TIMER_ARMOR_PER_WAVE;
            }

            if (boss)
            {
                healthScale *=
                    BOSS_HEALTH_MULTIPLIER;

                damageScale *=
                    BOSS_DAMAGE_MULTIPLIER;

                armorScale *=
                    BOSS_ARMOR_MULTIPLIER;
            }

            uint64 baseHealth =
                creature->GetMaxHealth();

            uint64 scaledHealth =
                static_cast<uint64>(
                    static_cast<double>(
                        baseHealth) *
                    static_cast<double>(
                        healthScale));

            if (scaledHealth < 1)
                scaledHealth = 1;

            creature->SetMaxHealth(
                scaledHealth);

            creature->SetHealth(
                scaledHealth);

            float damageBonusPct =
                (damageScale - 1.0f) *
                100.0f;

            float armorBonusPct =
                (armorScale - 1.0f) *
                100.0f;

            if (damageBonusPct !=
                0.0f)
            {
                creature->
                    ApplyStatPctModifier(
                        UNIT_MOD_DAMAGE_MAINHAND,
                        TOTAL_PCT,
                        damageBonusPct);

                creature->
                    ApplyStatPctModifier(
                        UNIT_MOD_DAMAGE_OFFHAND,
                        TOTAL_PCT,
                        damageBonusPct);

                creature->
                    ApplyStatPctModifier(
                        UNIT_MOD_DAMAGE_RANGED,
                        TOTAL_PCT,
                        damageBonusPct);
            }

            if (armorBonusPct !=
                0.0f)
            {
                creature->
                    ApplyStatPctModifier(
                        UNIT_MOD_ARMOR,
                        TOTAL_PCT,
                        armorBonusPct);
            }
        }

        // ---------------------------------------------------------------------
        // Spawn / progression
        // ---------------------------------------------------------------------

        void SpawnNextWave(
            Session& session)
        {
            if (HasReachedFinalWave(
                    session))
            {
                return;
            }

            ++session.wave;

            session.waitingNextWave =
                false;

            session.nextWaveDelayMs = 0;
            session.fixedWaveTimerMs = 0;

            CompactSummonList(session);

            Player* summoner =
                GetAnyOnlinePlayer(session);

            if (!summoner)
                return;

            bool bossWave =
                IsBossWave(session);

            uint32 desiredCount =
                GetDesiredSpawnCount(
                    session);

            uint32 currentlyAlive =
                CountAliveSummons(
                    session);

            uint32 spawnCount =
                desiredCount;

            uint32 activeCap =
                session.mode ==
                    MODE_INFINITE
                    ? MAX_ACTIVE_TRASH_INFINITE
                    : MAX_ACTIVE_TRASH_TIMER;

            // Pour les vagues toutes les 2 minutes,
            // des mobs précédents peuvent encore vivre.
            if (!bossWave &&
                session.progression ==
                    PROGRESSION_FIXED_2_MINUTES)
            {
                if (currentlyAlive >=
                    activeCap)
                {
                    spawnCount = 0;
                }
                else
                {
                    uint32 availableSlots =
                        activeCap -
                        currentlyAlive;

                    if (spawnCount >
                        availableSlots)
                    {
                        spawnCount =
                            availableSlots;
                    }
                }
            }

            std::ostringstream waveMessage;

            waveMessage
                << "|cffffff00[Blood Arena]|r Vague "
                << session.wave;

            if (bossWave)
                waveMessage << " - BOSS";

            Broadcast(
                session.id,
                waveMessage.str());

            uint32 successfulSpawns = 0;

            for (uint32 i = 0;
                 i < spawnCount;
                 ++i)
            {
                uint32 entry =
                    bossWave
                        ? GetBossEntry(session)
                        : GetTrashEntry(
                            session,
                            i);

                if (entry == 0)
                    continue;

                Position spawnPosition =
                    GetMobSpawnPosition(
                        i,
                        session.wave);

                TempSummon* summon =
                    summoner->SummonCreature(
                        entry,
                        spawnPosition,
                        TEMPSUMMON_MANUAL_DESPAWN,
                        Milliseconds(
                            SUMMON_DESPAWN_MS));

                if (!summon)
                    continue;

                summon->SetPhaseMask(
                    session.phaseMask,
                    true);

                summon->SetReactState(
                    REACT_AGGRESSIVE);

                ApplyScaling(
                    summon,
                    session,
                    bossWave);

                session.summons.push_back(
                    summon->GetGUID());

                session.summonWave[
                    summon->
                        GetGUID().
                        GetCounter()] =
                    session.wave;

                ++successfulSpawns;

                Player* target =
                    GetNearestAliveParticipant(
                        session,
                        summon);

                if (summon->AI())
                {
                    if (target)
                    {
                        summon->AI()->
                            AttackStart(target);
                    }
                    else
                    {
                        summon->AI()->
                            DoZoneInCombat();
                    }
                }
            }

            session.waveSpawnCount[
                session.wave] =
                successfulSpawns;

            if (spawnCount > 0 &&
                successfulSpawns == 0)
            {
                session.configurationError = true;

                Broadcast(
                    session.id,
                    "|cffff0000[Blood Arena]|r ERREUR : aucun creature_entry valide dans le pool SQL pour cette vague.");

                return;
            }

            if (spawnCount == 0)
            {
                Broadcast(
                    session.id,
                    "|cffff8000[Blood Arena]|r Limite de creatures actives atteinte.");
            }

            if (session.progression ==
                PROGRESSION_FIXED_2_MINUTES)
            {
                session.fixedWaveTimerMs =
                    FIXED_WAVE_INTERVAL_MS;
            }

            if (session.mode ==
                MODE_TIMER)
            {
                uint32 seconds =
                    session.remainingMs /
                    1000;

                uint32 minutes =
                    seconds / 60;

                uint32 remainingSeconds =
                    seconds % 60;

                std::ostringstream message;

                message
                    << "|cff00ffff[Blood Arena]|r Temps restant : "
                    << minutes
                    << "m "
                    << remainingSeconds
                    << "s.";

                Broadcast(
                    session.id,
                    message.str());
            }
        }

        void QueueNextWave(
            Session& session)
        {
            if (HasReachedFinalWave(
                    session))
            {
                return;
            }

            session.waitingNextWave =
                true;

            session.nextWaveDelayMs =
                INTERWAVE_DELAY_MS;

            Broadcast(
                session.id,
                "|cffffff00[Blood Arena]|r Prochaine vague dans 5 secondes.");
        }

        // ---------------------------------------------------------------------
        // Suivi des invocations
        // ---------------------------------------------------------------------

        uint32 CountAliveSummons(
            Session& session)
        {
            Player* referencePlayer =
                GetAnyOnlinePlayer(
                    session);

            if (!referencePlayer)
                return 0;

            uint32 alive = 0;

            std::vector<ObjectGuid>
                validGuids;

            for (ObjectGuid const& guid :
                 session.summons)
            {
                Creature* creature =
                    ObjectAccessor::GetCreature(
                        *referencePlayer,
                        guid);

                if (!creature)
                    continue;

                validGuids.push_back(guid);

                if (creature->IsAlive())
                    ++alive;
            }

            session.summons.swap(
                validGuids);

            return alive;
        }

        void CompactSummonList(
            Session& session)
        {
            Player* referencePlayer =
                GetAnyOnlinePlayer(
                    session);

            if (!referencePlayer)
            {
                session.summons.clear();
                return;
            }

            std::vector<ObjectGuid>
                validGuids;

            for (ObjectGuid const& guid :
                 session.summons)
            {
                Creature* creature =
                    ObjectAccessor::GetCreature(
                        *referencePlayer,
                        guid);

                if (creature)
                    validGuids.push_back(guid);
            }

            session.summons.swap(
                validGuids);
        }

        bool IsWaveCleared(
            Session& session,
            uint32 wave) const
        {
            auto countIt =
                session.waveSpawnCount.find(
                    wave);

            if (countIt ==
                    session.waveSpawnCount.end() ||
                countIt->second == 0)
            {
                return false;
            }

            Player* referencePlayer =
                GetAnyOnlinePlayer(
                    session);

            if (!referencePlayer)
                return false;

            for (ObjectGuid const& guid :
                 session.summons)
            {
                auto waveIt =
                    session.summonWave.find(
                        guid.GetCounter());

                if (waveIt ==
                        session.summonWave.end() ||
                    waveIt->second != wave)
                {
                    continue;
                }

                Creature* creature =
                    ObjectAccessor::GetCreature(
                        *referencePlayer,
                        guid);

                if (creature &&
                    creature->IsAlive())
                {
                    return false;
                }
            }

            return true;
        }

        void CleanupWaveSummons(
            Session& session,
            uint32 wave)
        {
            Player* referencePlayer =
                GetAnyOnlinePlayer(
                    session);

            std::vector<ObjectGuid> kept;

            for (ObjectGuid const& guid :
                 session.summons)
            {
                auto waveIt =
                    session.summonWave.find(
                        guid.GetCounter());

                if (waveIt ==
                        session.summonWave.end() ||
                    waveIt->second != wave)
                {
                    kept.push_back(guid);
                    continue;
                }

                if (referencePlayer)
                {
                    Creature* creature =
                        ObjectAccessor::GetCreature(
                            *referencePlayer,
                            guid);

                    if (creature)
                    {
                        creature->
                            DespawnOrUnsummon();
                    }
                }

                session.summonWave.erase(
                    guid.GetCounter());
            }

            session.summons.swap(kept);

            session.waveSpawnCount.erase(
                wave);
        }

        void ProcessInfiniteWaveRewards(
            Session& session)
        {
            if (session.mode !=
                MODE_INFINITE)
            {
                return;
            }

            std::vector<uint32>
                clearedWaves;

            for (auto const& pair :
                 session.waveSpawnCount)
            {
                uint32 wave = pair.first;

                if (IsWaveCleared(
                        session,
                        wave))
                {
                    clearedWaves.push_back(
                        wave);
                }
            }

            for (uint32 wave :
                 clearedWaves)
            {
                RewardInfiniteWave(
                    session,
                    wave);

                CleanupWaveSummons(
                    session,
                    wave);
            }
        }

        void CleanupSummons(
            Session& session)
        {
            Player* referencePlayer =
                GetAnyOnlinePlayer(
                    session);

            if (referencePlayer)
            {
                for (ObjectGuid const& guid :
                     session.summons)
                {
                    Creature* creature =
                        ObjectAccessor::GetCreature(
                            *referencePlayer,
                            guid);

                    if (creature)
                    {
                        creature->
                            DespawnOrUnsummon();
                    }
                }
            }

            session.summons.clear();
            session.summonWave.clear();
            session.waveSpawnCount.clear();
        }
    };

    // -------------------------------------------------------------------------
    // PNJ Maître de l'Arène
    //
    // Pour l'Entry 32748 :
    // creature_template.ScriptName = 'npc_blood_arena_master'
    //
    // Tous les spawns de cette Entry, peu importe la map,
    // utiliseront ce même script.
    // -------------------------------------------------------------------------

    class npc_blood_arena_master :
        public CreatureScript
    {
    public:
        npc_blood_arena_master()
            : CreatureScript(
                "npc_blood_arena_master")
        {
        }

        struct npc_blood_arena_masterAI :
            public ScriptedAI
        {
            npc_blood_arena_masterAI(
                Creature* creature)
                : ScriptedAI(creature)
            {
            }

            bool OnGossipHello(
                Player* player) override
            {
                if (!player || !me)
                    return true;

                ClearGossipMenuFor(player);

                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Infini - vague suivante au dernier ennemi",
                    GOSSIP_SENDER_MAIN,
                    ACTION_INFINITE_LAST_KILL);

                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Infini - nouvelle vague toutes les 2 minutes",
                    GOSSIP_SENDER_MAIN,
                    ACTION_INFINITE_FIXED);

                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Timer 30 min - vague suivante au dernier ennemi",
                    GOSSIP_SENDER_MAIN,
                    ACTION_TIMER_LAST_KILL);

                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Timer 30 min - nouvelle vague toutes les 2 minutes",
                    GOSSIP_SENDER_MAIN,
                    ACTION_TIMER_FIXED);

                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_CHAT,
                    "Classement mensuel de l'Arene",
                    GOSSIP_SENDER_MAIN,
                    ACTION_LEADERBOARD_MENU);

                if (player->GetSession() &&
                    player->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
                {
                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "|cffff8000Administration Blood Arena|r",
                        GOSSIP_SENDER_MAIN,
                        ACTION_ADMIN_MENU);
                }

                SendGossipMenuFor(
                    player,
                    68,
                    me->GetGUID());

                return true;
            }

            bool OnGossipSelect(
                Player* player,
                uint32 /*menuId*/,
                uint32 gossipListId) override
            {
                if (!player || !me)
                    return true;

                uint32 action =
                    player->
                    PlayerTalkClass->
                    GetGossipOptionAction(
                        gossipListId);

                ClearGossipMenuFor(player);

                std::string error;
                bool started = false;

                if (action == ACTION_BACK_MAIN)
                    return OnGossipHello(player);

                if (action == ACTION_ADMIN_MENU)
                {
                    if (!player->GetSession() ||
                        player->GetSession()->GetSecurity() < SEC_GAMEMASTER)
                    {
                        return OnGossipHello(player);
                    }

                    ClearGossipMenuFor(player);

                    std::ostringstream trashInfo;
                    trashInfo
                        << "Trash charges : "
                        << ArenaManager::Instance().
                            GetLoadedTrashCount();

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        trashInfo.str(),
                        GOSSIP_SENDER_MAIN,
                        ACTION_ADMIN_MENU);

                    std::ostringstream bossInfo;
                    bossInfo
                        << "Boss charges : "
                        << ArenaManager::Instance().
                            GetLoadedBossCount();

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        bossInfo.str(),
                        GOSSIP_SENDER_MAIN,
                        ACTION_ADMIN_MENU);

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "Recharger les monstres depuis la base SQL",
                        GOSSIP_SENDER_MAIN,
                        ACTION_ADMIN_RELOAD_CREATURES);

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "< Retour",
                        GOSSIP_SENDER_MAIN,
                        ACTION_BACK_MAIN);

                    SendGossipMenuFor(
                        player,
                        68,
                        me->GetGUID());

                    return true;
                }

                if (action == ACTION_ADMIN_RELOAD_CREATURES)
                {
                    if (!player->GetSession() ||
                        player->GetSession()->GetSecurity() < SEC_GAMEMASTER)
                    {
                        return OnGossipHello(player);
                    }

                    bool valid =
                        ArenaManager::Instance().
                            ReloadCreaturePool();

                    std::ostringstream message;

                    message
                        << "|cff00ffff[Blood Arena]|r Pool SQL recharge : "
                        << ArenaManager::Instance().
                            GetLoadedTrashCount()
                        << " trash(s), "
                        << ArenaManager::Instance().
                            GetLoadedBossCount()
                        << " boss.";

                    if (!valid)
                    {
                        message
                            << " |cffff0000ATTENTION : le pool doit contenir au moins un trash et un boss valides.|r";
                    }

                    ChatHandler(
                        player->GetSession()).
                        SendSysMessage(
                            message.str().c_str());

                    return OnGossipHello(player);
                }

                if (action == ACTION_LEADERBOARD_MENU)
                {
                    ClearGossipMenuFor(player);

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "Top 10 - Vagues infinies",
                        GOSSIP_SENDER_MAIN,
                        ACTION_LEADERBOARD_INFINITE);

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "Top 10 - Timer",
                        GOSSIP_SENDER_MAIN,
                        ACTION_LEADERBOARD_TIMER);

                    AddGossipItemFor(
                        player,
                        GOSSIP_ICON_CHAT,
                        "< Retour",
                        GOSSIP_SENDER_MAIN,
                        ACTION_BACK_MAIN);

                    SendGossipMenuFor(
                        player,
                        68,
                        me->GetGUID());

                    return true;
                }

                if (action == ACTION_LEADERBOARD_INFINITE)
                {
                    ArenaManager::Instance().
                        ShowLeaderboard(
                            player,
                            me,
                            MODE_INFINITE);

                    return true;
                }

                if (action == ACTION_LEADERBOARD_TIMER)
                {
                    ArenaManager::Instance().
                        ShowLeaderboard(
                            player,
                            me,
                            MODE_TIMER);

                    return true;
                }

                switch (action)
                {
                    case ACTION_INFINITE_LAST_KILL:
                        started =
                            ArenaManager::Instance().
                            StartSession(
                                player,
                                me,
                                MODE_INFINITE,
                                PROGRESSION_LAST_KILL,
                                error);
                        break;

                    case ACTION_INFINITE_FIXED:
                        started =
                            ArenaManager::Instance().
                            StartSession(
                                player,
                                me,
                                MODE_INFINITE,
                                PROGRESSION_FIXED_2_MINUTES,
                                error);
                        break;

                    case ACTION_TIMER_LAST_KILL:
                        started =
                            ArenaManager::Instance().
                            StartSession(
                                player,
                                me,
                                MODE_TIMER,
                                PROGRESSION_LAST_KILL,
                                error);
                        break;

                    case ACTION_TIMER_FIXED:
                        started =
                            ArenaManager::Instance().
                            StartSession(
                                player,
                                me,
                                MODE_TIMER,
                                PROGRESSION_FIXED_2_MINUTES,
                                error);
                        break;

                    default:
                        error =
                            "Choix d'arene inconnu.";
                        break;
                }

                CloseGossipMenuFor(player);

                if (!started &&
                    !error.empty())
                {
                    ChatHandler(
                        player->GetSession()).
                        SendSysMessage(
                            error.c_str());
                }

                return true;
            }
        };

        CreatureAI* GetAI(
            Creature* creature) const override
        {
            return
                new npc_blood_arena_masterAI(
                    creature);
        }
    };

    // -------------------------------------------------------------------------
    // World update
    // -------------------------------------------------------------------------

    class blood_arena_world :
        public WorldScript
    {
    public:
        blood_arena_world()
            : WorldScript(
                "blood_arena_world")
        {
        }

        void OnStartup() override
        {
            ArenaManager::Instance().
                InitializeDatabase();
        }

        void OnUpdate(
            uint32 diff) override
        {
            ArenaManager::Instance().
                Update(diff);
        }
    };

    // -------------------------------------------------------------------------
    // Hooks joueur
    // -------------------------------------------------------------------------

    class blood_arena_player :
        public PlayerScript
    {
    public:
        blood_arena_player()
            : PlayerScript(
                "blood_arena_player")
        {
        }

        void OnGiveXP(
            Player* player,
            uint32& amount,
            Unit* /*victim*/) override
        {
            if (ArenaManager::Instance().
                IsPlayerInArena(player))
            {
                amount = 0;
            }
        }

        // Si le worldserver redémarre pendant une session,
        // le gestionnaire mémoire disparaît. On évite alors de
        // laisser le joueur coincé dans une phase réservée.
        void OnLogin(
            Player* player,
            bool /*firstLogin*/)
        {
            if (!player)
                return;

            ArenaManager::Instance().
                NotifyUnreadGmNotices(player);

            if (ArenaManager::Instance().
                IsPlayerInArena(player))
            {
                return;
            }

            if (player->GetMapId() !=
                ARENA_MAP_ID)
            {
                return;
            }

            uint32 phaseMask =
                player->GetPhaseMask();

            if (ArenaManager::Instance().
                IsArenaPhase(phaseMask))
            {
                player->SetPhaseMask(
                    1,
                    true);

                ChatHandler(
                    player->GetSession()).
                    SendSysMessage(
                        "|cffff8000[Blood Arena]|r Phase d'arene orpheline detectee : retour en phase 1.");
            }
        }
    };

} // namespace BloodArena

// -----------------------------------------------------------------------------
// Loader
// -----------------------------------------------------------------------------

void AddSC_custom_blood_arena()
{
    SC_LOG_INFO(
        "server.loading",
        "[BloodArena] Chargement du script Blood Arena V1.6.");

    new BloodArena::
        npc_blood_arena_master();

    new BloodArena::
        blood_arena_world();

    new BloodArena::
        blood_arena_player();
}
