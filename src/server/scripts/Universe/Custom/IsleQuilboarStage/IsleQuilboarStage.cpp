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

//By leewheel 2026-09-08
//============================================================================
// Isle of Reach (map 859) - quest chain checkpoint NPC manager
//
// Problem:
//   On map 859 the same quest NPC (Austin Huxworth 157046 for the Alliance,
//   Breka Grimaxe 245248 for the Horde) used to have several STATIC spawns
//   placed along the quilboar quest chain. All of them were visible at the
//   same time, which breaks the intended design: when the player accepts
//   [Quilboar Shadow Magic] the NPC should leave the first checkpoint and
//   wait for the player at the second one.
//
//   TrinityCore 3.3.5 has no per-player creature visibility (no PhaseMgr,
//   CanSeeOrDetect never compares phase masks), so we rely on the core
//   feature "TempSummon visible by summoner only"
//   (Object::SummonCreature last boolean parameter). Each player gets his own
//   private copy of the checkpoint NPC standing at the point that matches his
//   personal quest progress. The static spawns were removed from the DB.
//
// Checkpoint layout (map 859):
//   checkpoint 1 (-247, -2492) : NPC waits here while the player has not yet
//                                accepted the shadow-magic quest
//   checkpoint 2 (-141, -2637) : NPC moved next to the quilboar den, waits
//                                for the turn-in of the accepted quest
//   checkpoint 3 (100, -2420)  : NPC waits near the "Scout-o-Matic /
//                                boosted boar" area for the rest of the chain
//
// Quest chain:
//   Alliance: 55184 Quilboar Shadow Magic -> 55186 Down with the Quilboar
//             -> 55193 The Scout-o-Matic 5000
//             -> 55879 Ride of the Scientifically Enhanced Boar
//   Horde:    59939 Forbidden Quilboar Shadow Magic
//             -> 59938 Down with the Quilboar
//             -> 59940 The Choppy Booster Mk. 5
//             -> 59942 The Re-Deather
//
// Strategy:
//   - GetStage() computes the expected checkpoint from the quest state of the
//     player (0 = no NPC needed).
//   - The private copy is summoned only while the player is close enough to
//     the expected checkpoint (SPAWN_RANGE yards), otherwise the previous
//     copy is despawned. This avoids stale TempSummons being destroyed by the
//     grid when the player walks away, and it naturally gives the "he left"
//     feeling right after a quest is accepted.
//   - Refresh triggers: WorldScript heartbeat (1 s, because the 3.3.5 core
//     never fires OnQuestStatusChange toward C++ scripts), Player OnLogin and
//     OnMapChanged.
//
// (FR) Sur la carte 859, le meme PNJ de quete (Austin Huxworth 157046 cote
//   Alliance, Breka Grimaxe 245248 cote Horde) possedait plusieurs spawns
//   statiques visibles en meme temps, ce qui casse la progression voulue :
//   quand le joueur accepte la quete d'ombre quilboar, le PNJ doit quitter le
//   premier point de controle et attendre au second. Le noyau 3.3.5 n'offre
//   aucune visibilite par joueur (pas de PhaseMgr), on s'appuie donc sur la
//   fonction "TempSummon visible par l'invocateur uniquement" : chaque joueur
//   recoit sa copie privee au point de controle correspondant a sa progression
//   personnelle. Les spawns statiques ont ete supprimes en base.
// End By leewheel

#include "ScriptMgr.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include <cmath>
#include <unordered_map>

namespace
{
    constexpr uint32 MAP_ISLE_859 = 859;
    constexpr float SPAWN_RANGE = 120.0f; // yards; summon the private NPC only when the player is close

    // Alliance chain (Austin Huxworth 157046)
    constexpr uint32 QA1 = 55184; // Quilboar Shadow Magic
    constexpr uint32 QA2 = 55186; // Down with the Quilboar
    constexpr uint32 QA3 = 55193; // The Scout-o-Matic 5000
    constexpr uint32 QA4 = 55879; // Ride of the Scientifically Enhanced Boar

    // Horde chain (Breka Grimaxe 245248)
    constexpr uint32 QH1 = 59939; // Forbidden Quilboar Shadow Magic
    constexpr uint32 QH2 = 59938; // Down with the Quilboar
    constexpr uint32 QH3 = 59940; // The Choppy Booster Mk. 5
    constexpr uint32 QH4 = 59942; // The Re-Deather

    struct CheckPoint
    {
        float x, y, z, o;
    };

    struct FactionConfig
    {
        uint32 entry;
        uint32 q1, q2, q3, q4;
        CheckPoint points[3];
    };

    FactionConfig const g_alliance =
    {
        157046, QA1, QA2, QA3, QA4,
        {
            { -247.601f, -2492.04f, 17.9996f, 3.06504f  }, // checkpoint 1
            { -141.200f, -2636.59f, 48.2745f, 2.22779f  }, // checkpoint 2
            {  100.148f, -2419.73f, 90.2718f, 0.251483f }  // checkpoint 3
        }
    };

    FactionConfig const g_horde =
    {
        245248, QH1, QH2, QH3, QH4,
        {
            { -246.732f, -2489.28f, 17.993f,  2.89274f }, // checkpoint 1
            { -144.048f, -2638.73f, 48.5814f, 2.12442f }, // checkpoint 2
            {  102.587f, -2417.51f, 90.3689f, 5.0011f  }  // checkpoint 3
        }
    };

    // registered private copies: player guid (raw) -> summoned npc guid (raw)
    std::unordered_map<uint64, uint64> g_privateNPC;

    bool IsActive(QuestStatus st)
    {
        return st == QUEST_STATUS_INCOMPLETE || st == QUEST_STATUS_COMPLETE;
    }

    bool IsDone(QuestStatus st)
    {
        return st == QUEST_STATUS_REWARDED;
    }

    // returns 0 = no NPC needed, 1..3 = checkpoint index
    uint8 GetStage(Player const* player, FactionConfig const& cfg)
    {
        QuestStatus s1 = player->GetQuestStatus(cfg.q1);
        QuestStatus s2 = player->GetQuestStatus(cfg.q2);
        QuestStatus s3 = player->GetQuestStatus(cfg.q3);
        QuestStatus s4 = player->GetQuestStatus(cfg.q4);

        if (IsDone(s4))
            return 0;
        if (IsActive(s2) || IsDone(s2) || IsActive(s3) || IsDone(s3) || IsActive(s4))
            return 3;
        if (IsActive(s1) || IsDone(s1))
            return 2;
        return 1;
    }

    FactionConfig const* GetFactionConfig(Player const* player)
    {
        return player->GetTeamId() == TEAM_ALLIANCE ? &g_alliance : &g_horde;
    }
}

class isle_quilboar_stage_npc : public PlayerScript
{
public:
    isle_quilboar_stage_npc() : PlayerScript("isle_quilboar_stage_npc") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        Refresh(player);
    }

    void OnMapChanged(Player* player) override
    {
        Refresh(player);
    }

    void OnLogout(Player* player) override
    {
        Cleanup(player);
    }

    static void Refresh(Player* player)
    {
        if (!player || !player->IsInWorld())
            return;

        uint64 pguid = player->GetGUID().GetRawValue();

        // Only the shared island map is handled here; leaving it removes the copy.
        if (player->GetMapId() != MAP_ISLE_859)
        {
            Cleanup(player);
            return;
        }

        FactionConfig const* cfg = GetFactionConfig(player);
        uint8 stage = GetStage(player, *cfg);
        if (stage == 0)
        {
            Cleanup(player);
            return;
        }

        CheckPoint const& pt = cfg->points[stage - 1];

        // Keep the private NPC alive only while the player is near the checkpoint.
        if (player->GetDistance2d(pt.x, pt.y) > SPAWN_RANGE)
        {
            Cleanup(player);
            return;
        }

        // An up to date private copy may already exist for this player.
        auto itr = g_privateNPC.find(pguid);
        if (itr != g_privateNPC.end())
        {
            Creature* existing = ObjectAccessor::GetCreature(*player, ObjectGuid(itr->second));
            if (existing && existing->GetEntry() == cfg->entry &&
                std::fabs(existing->GetPositionX() - pt.x) < 0.5f &&
                std::fabs(existing->GetPositionY() - pt.y) < 0.5f)
                return; // right NPC at the right checkpoint, nothing to do

            Cleanup(player); // stale or moved to another checkpoint: rebuild
        }

        if (TempSummon* summon = player->SummonCreature(cfg->entry, pt.x, pt.y, pt.z, pt.o,
                                                        TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
        {
            g_privateNPC[pguid] = summon->GetGUID().GetRawValue();
        }
    }

    static void Cleanup(Player* player)
    {
        if (!player)
            return;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto itr = g_privateNPC.find(pguid);
        if (itr == g_privateNPC.end())
            return;

        if (Creature* npc = ObjectAccessor::GetCreature(*player, ObjectGuid(itr->second)))
            npc->DespawnOrUnsummon();

        g_privateNPC.erase(itr);
    }
};

// 1 s heartbeat so accepting / turning in / abandoning the chain quests is
// reflected even though this 3.3.5 core never raises OnQuestStatusChange.
class isle_quilboar_stage_world : public WorldScript
{
public:
    isle_quilboar_stage_world() : WorldScript("isle_quilboar_stage_world"), _ticker(0) { }

    void OnUpdate(uint32 diff) override
    {
        _ticker += diff;
        if (_ticker < 1000)
            return;
        _ticker = 0;

        for (auto const& itr : ObjectAccessor::GetPlayers())
        {
            Player* player = itr.second;
            if (!player || !player->IsInWorld())
                continue;
            if (player->GetMapId() == MAP_ISLE_859)
                isle_quilboar_stage_npc::Refresh(player);
        }
    }

private:
    uint32 _ticker;
};

//============================================================================
//By leewheel 2026-09-08
// Whole-island mirrored checkpoint NPCs (map 859)
//
// The duplication problem fixed for the quilboar leaders above also exists
// for the other named quest NPCs of the island: Lady Jaina Proudmoore and
// Kee-La on the Alliance side, Thrall and Bo on the Horde side. Each of them
// used to own several STATIC spawns placed along the storyline, so every
// copy was visible at the same time on the shared island map.
//
// (FR) Le probleme de duplication resolu pour les chefs quilboars ci-dessus
// touche aussi les autres PNJ nommes de l'ile : Lady Jaina Proudmoore et
// Kee-La cote Alliance, Thrall et Bo cote Horde. Chacun possedait plusieurs
// spawns statiques disposes le long de la trame, tous visibles en meme temps
// sur la carte partagee de l'ile.
//
// Checkpoint layout (map 859, shared by both factions):
//   Alliance:
//     Jaina (156280)  cp1 beach near the Murlocs (-439,-2599): the player has
//                     not started the quilboar chain yet
//                     cp2 mid-chain grassland (318,-2174): the quilboar chain
//                     has started
//                     cp3 in front of Darkmaul Citadel (706,-1867): the
//                     ogre / dungeon quests have started
//     Kee-La (157043) cp1 southern grassland (283,-2338): the harpy quest is
//                     not taken yet
//                     cp2 west entrance of the Harpy Roost (374,-2445): the
//                     harpy quest has been taken
//   Horde:
//     Thrall (166573) same positions as Jaina
//     Bo     (166585) same positions as Kee-La
//
// Generic stage machine shared by every config:
//   - "boundary" quests are ordered. While no boundary quest has been started
//     the private copy stands on checkpoint 0; starting boundary i moves the
//     private copy to checkpoint i + 1.
//   - the whole arc ends (copy removed) once the "terminator" quest of the
//     config has been rewarded.
//   - as for the quilboar leaders, the core 3.3.5 feature "TempSummon visible
//     by summoner only" gives each player his own private copy, refreshed by
//     a 1 s heartbeat, OnLogin and OnMapChanged.
// (FR) Machine a etats generique partagee par chaque configuration :
//   - les quetes "frontieres" sont ordonnees. Tant qu'aucune quete frontiere
//     n'a ete demarree, la copie privee se tient au point de controle 0 ;
//     demarrer la frontiere i deplace la copie au point i + 1.
//   - l'arc se termine (copie retiree) une fois la quete "terminator" de la
//     configuration recompensee.
//
// Quest chains (Alliance / Horde):
//   Jaina  A: boundary 55184 (quilboar chain) -> boundary 55988 (ogre quests)
//             -> terminator 56344 (To Darkmaul Citadel rewarded)
//   Kee-La A: boundary 55196 -> terminator 55764
//   Thrall H: boundary 59939 -> boundary 59979 -> terminator 59975
//   Bo     H: boundary 59943 -> terminator 59945
// End By leewheel
namespace
{
    constexpr uint8 STAGE_DONE = 0xFF; // private copy must not exist

    struct IslandStageNpc
    {
        uint32 entry;        // creature template entry of the quest NPC
        TeamId team;         // which faction is served by this NPC
        uint32 terminator;   // quest rewarded -> story arc over, no copy
        uint32 bounds[3];    // boundary quests, order matters
        uint8  boundsCount;  // number of boundary quests (1..3)
        CheckPoint points[4];// one checkpoint per possible stage
    };

    // registered private copies: player guid (raw) -> entry -> npc guid (raw)
    std::unordered_map<uint64, std::unordered_map<uint32, uint64>> g_islandNPCs;

    IslandStageNpc const g_islandStages[] =
    {
        // Alliance - Lady Jaina Proudmoore: beach -> grassland -> citadel gate
        {
            156280, TEAM_ALLIANCE, 56344, { 55184, 55988 }, 2,
            {
                { -439.195f, -2599.68f, 0.628877f, 0.188965f },
                {  318.397f, -2174.48f, 105.99f,   2.83133f  },
                {  706.727f, -1867.60f, 186.882f,  4.36286f  }
            }
        },
        // Alliance - Kee-La: grassland -> west entrance of the Harpy Roost
        {
            157043, TEAM_ALLIANCE, 55764, { 55196 }, 1,
            {
                { 283.361f, -2338.67f, 84.9874f, 2.32649f },
                { 374.309f, -2445.20f, 122.433f, 0.852012f }
            }
        },
        // Horde - Thrall: same positions as Jaina
        {
            166573, TEAM_HORDE, 59975, { 59939, 59979 }, 2,
            {
                { -435.865f, -2611.58f, 0.591125f, 0.182215f },
                {  318.374f, -2177.90f, 105.697f,  2.83133f  },
                {  709.864f, -1868.96f, 186.882f,  4.30395f  }
            }
        },
        // Horde - Bo: same positions as Kee-La
        {
            166585, TEAM_HORDE, 59945, { 59943 }, 1,
            {
                { 281.109f, -2335.98f, 84.7438f, 5.42252f },
                { 371.783f, -2442.98f, 122.017f, 0.816669f }
            }
        }
    };

    constexpr uint32 ISLAND_STAGE_COUNT = sizeof(g_islandStages) / sizeof(g_islandStages[0]);

    // Returns the checkpoint index for the given player, or STAGE_DONE when
    // the whole story arc of this NPC is over for him.
    uint8 ComputeIslandStage(Player const* player, IslandStageNpc const& cfg)
    {
        if (player->GetQuestStatus(cfg.terminator) == QUEST_STATUS_REWARDED)
            return STAGE_DONE;

        uint8 stage = 0;
        for (uint8 i = 0; i < cfg.boundsCount; ++i)
            if (player->GetQuestStatus(cfg.bounds[i]) != QUEST_STATUS_NONE)
                stage = i + 1;
        return stage;
    }

    void DespawnIslandEntry(Player* player, uint32 entry)
    {
        uint64 pguid = player->GetGUID().GetRawValue();
        auto pItr = g_islandNPCs.find(pguid);
        if (pItr == g_islandNPCs.end())
            return;

        auto eItr = pItr->second.find(entry);
        if (eItr == pItr->second.end())
            return;

        if (Creature* npc = ObjectAccessor::GetCreature(*player, ObjectGuid(eItr->second)))
            npc->DespawnOrUnsummon();

        pItr->second.erase(eItr);
        if (pItr->second.empty())
            g_islandNPCs.erase(pItr);
    }
}

// Handles the per-player private copies of every whole-island checkpoint NPC.
class isle_stage_mirror_npc : public PlayerScript
{
public:
    isle_stage_mirror_npc() : PlayerScript("isle_stage_mirror_npc") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        RefreshPlayer(player);
    }

    void OnMapChanged(Player* player) override
    {
        RefreshPlayer(player);
    }

    void OnLogout(Player* player) override
    {
        CleanupPlayer(player);
    }

    static void RefreshPlayer(Player* player)
    {
        if (!player || !player->IsInWorld())
            return;

        // Only the shared island map is handled here; leaving it removes all copies.
        if (player->GetMapId() != MAP_ISLE_859)
        {
            CleanupPlayer(player);
            return;
        }

        TeamId team = player->GetTeamId();
        uint64 pguid = player->GetGUID().GetRawValue();

        for (uint32 k = 0; k < ISLAND_STAGE_COUNT; ++k)
        {
            IslandStageNpc const& cfg = g_islandStages[k];
            if (cfg.team != team)
                continue;

            uint8 stage = ComputeIslandStage(player, cfg);
            if (stage == STAGE_DONE)
            {
                DespawnIslandEntry(player, cfg.entry);
                continue;
            }

            CheckPoint const& pt = cfg.points[stage];

            // Keep the private NPC alive only while the player is near the checkpoint.
            if (player->GetDistance2d(pt.x, pt.y) > SPAWN_RANGE)
            {
                DespawnIslandEntry(player, cfg.entry);
                continue;
            }

            // An up to date private copy may already exist for this player.
            bool upToDate = false;
            auto pItr = g_islandNPCs.find(pguid);
            if (pItr != g_islandNPCs.end())
            {
                auto eItr = pItr->second.find(cfg.entry);
                if (eItr != pItr->second.end())
                {
                    Creature* existing = ObjectAccessor::GetCreature(*player, ObjectGuid(eItr->second));
                    if (existing && existing->GetEntry() == cfg.entry &&
                        std::fabs(existing->GetPositionX() - pt.x) < 0.5f &&
                        std::fabs(existing->GetPositionY() - pt.y) < 0.5f)
                        upToDate = true;
                }
            }

            if (upToDate)
                continue;

            DespawnIslandEntry(player, cfg.entry); // stale or moved: rebuild

            if (TempSummon* summon = player->SummonCreature(cfg.entry, pt.x, pt.y, pt.z, pt.o,
                                                            TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
            {
                g_islandNPCs[pguid][cfg.entry] = summon->GetGUID().GetRawValue();
            }
        }
    }

    static void CleanupPlayer(Player* player)
    {
        if (!player)
            return;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto pItr = g_islandNPCs.find(pguid);
        if (pItr == g_islandNPCs.end())
            return;

        for (auto const& eItr : pItr->second)
        {
            if (Creature* npc = ObjectAccessor::GetCreature(*player, ObjectGuid(eItr.second)))
                npc->DespawnOrUnsummon();
        }

        g_islandNPCs.erase(pItr);
    }
};

// 1 s heartbeat for the whole-island mirrored NPCs, because the 3.3.5 core
// never raises OnQuestStatusChange toward C++ scripts.
// (FR) Battement de coeur de 1 s pour les PNJ miroirs de l'ile, car le noyau
// 3.3.5 ne declenche jamais OnQuestStatusChange vers les scripts C++.
class isle_stage_mirror_world : public WorldScript
{
public:
    isle_stage_mirror_world() : WorldScript("isle_stage_mirror_world"), _ticker(0) { }

    void OnUpdate(uint32 diff) override
    {
        _ticker += diff;
        if (_ticker < 1000)
            return;
        _ticker = 0;

        for (auto const& itr : ObjectAccessor::GetPlayers())
        {
            Player* player = itr.second;
            if (!player || !player->IsInWorld())
                continue;
            if (player->GetMapId() == MAP_ISLE_859)
                isle_stage_mirror_npc::RefreshPlayer(player);
        }
    }

private:
    uint32 _ticker;
};

//============================================================================
//By leewheel 2026-09-08
// Quest ride vehicles of the quilboar chain (map 859)
//
// The shared island hosts three ride-related objects that used to be STATIC
// spawns visible to BOTH factions at the same time:
//   167027 Exploro-matic 5000 taxi - the scouting quest ride. Alliance 55193
//          "The Scout-o-Matic 5000" and Horde 59940 "The Choppy Booster
//          Mk. 5" both credit 167027; the actual completion is driven by the
//          Eluna script Delivery_Mount_IsleReachAH.lua (gossip menu ->
//          StartTaxi -> CompleteQuest). Eluna registers its hooks BY ENTRY,
//          so per-player copies of the taxi keep working.
//   167150 Giant Boar             - Alliance 55879 "Ride of the Scientifically
//          Enhanced Boar": a real VehicleId 123 mount (boar model, 3x size).
//   167142 Choppy Booster Mk. 5   - Horde 59942 "The Re-Deather": a real
//          VehicleId 123 mount. For both chargers the passenger casts 29579
//          "Throw Dynamite" from the vehicle action bar (creature_template
//          _spell) to destroy the undead army.
//
// This mirror follows the exact per-player TempSummon pattern of the
// checkpoint NPCs above, so an Alliance player never sees the Horde charger
// (nor the reverse). Each player owns at most ONE ride object:
//   - scouting quest in progress (q3 active)  -> his private taxi 167027
//   - scouting rewarded & charge quest active -> his faction charger
//   - charge quest rewarded                   -> nothing
// Static spawns of the three objects are removed by SQL (see the matching
// world update), the smart "despawn on passenger removed" of 167142 is
// removed as well (life cycle is now owned by this manager).
//
// (FR) Les objets de transport de la quete quilboar (carte 859) suivent le
// meme miroir par joueur que les PNJ de controle : taxi 167027 pendant la
// quete de reconnaissance, monture de charge 167150 (Alliance) ou 167142
// (Horde) pendant la quete de destruction. Chaque joueur ne possede qu'un
// objet a la fois, visible uniquement par lui.
// End By leewheel
namespace
{
    constexpr uint32 RIDE_TAXI     = 167027; // Exploro-matic 5000 (Eluna taxi, both factions)
    constexpr uint32 RIDE_BOAR     = 167150; // Alliance charger: Giant Boar
    constexpr uint32 RIDE_CHOPPY   = 167142; // Horde charger: Choppy Booster Mk. 5
    constexpr float  RIDE_RANGE    = 500.0f; // yards around checkpoint 3 (covers the whole undead battlefield)

    struct RideConfig
    {
        uint32    q3;        // scouting quest id
        uint32    q4;        // charge quest id
        uint32    charger;   // charge vehicle entry
        CheckPoint chargePt; // where the charger waits
    };

    RideConfig const g_rideAlliance =
    {
        QA3, QA4, RIDE_BOAR,
        { 99.500f, -2422.50f, 90.40f, 0.900f }
    };

    RideConfig const g_rideHorde =
    {
        QH3, QH4, RIDE_CHOPPY,
        { 119.142f, -2424.02f, 95.7469f, 0.965119f }
    };

    CheckPoint const RIDE_TAXI_PT = { 107.872f, -2414.18f, 95.4484f, 0.0f };

    // player guid (raw) -> summoned ride guid (raw); one object per player
    std::unordered_map<uint64, uint64> g_rideVehicle;

    // Returns the ride object this player should currently see, 0 = none.
    uint32 DesiredRide(Player const* player, RideConfig const& cfg)
    {
        QuestStatus s3 = player->GetQuestStatus(cfg.q3);
        QuestStatus s4 = player->GetQuestStatus(cfg.q4);
        bool scouting    = (s3 == QUEST_STATUS_INCOMPLETE || s3 == QUEST_STATUS_COMPLETE);
        bool chargeActive = (s4 == QUEST_STATUS_INCOMPLETE || s4 == QUEST_STATUS_COMPLETE);
        if (chargeActive)
            return cfg.charger;
        if (scouting)
            return RIDE_TAXI;
        return 0;
    }

    RideConfig const* GetRideConfig(Player const* player)
    {
        return player->GetTeamId() == TEAM_ALLIANCE ? &g_rideAlliance : &g_rideHorde;
    }
}

// Per-player private copy of the ride object matching the quest progress.
class isle_stage_mirror_vehicle : public PlayerScript
{
public:
    isle_stage_mirror_vehicle() : PlayerScript("isle_stage_mirror_vehicle") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        RefreshVehicle(player);
    }

    void OnMapChanged(Player* player) override
    {
        RefreshVehicle(player);
    }

    void OnLogout(Player* player) override
    {
        DespawnRide(player);
    }

    static void RefreshVehicle(Player* player)
    {
        if (!player || !player->IsInWorld())
            return;

        uint64 pguid = player->GetGUID().GetRawValue();

        // Only the shared island map is handled here.
        if (player->GetMapId() != MAP_ISLE_859)
        {
            DespawnRide(player);
            return;
        }

        RideConfig const* cfg = GetRideConfig(player);
        uint32 want = DesiredRide(player, *cfg);
        if (want == 0)
        {
            DespawnRide(player);
            return;
        }

        CheckPoint const& pt = (want == RIDE_TAXI) ? RIDE_TAXI_PT : cfg->chargePt;

        // Materialize the ride only while the player is around the ride area.
        if (player->GetDistance2d(pt.x, pt.y) > RIDE_RANGE)
        {
            DespawnRide(player);
            return;
        }

        // An up to date private copy may already exist for this player.
        auto itr = g_rideVehicle.find(pguid);
        if (itr != g_rideVehicle.end())
        {
            Creature* existing = ObjectAccessor::GetCreature(*player, ObjectGuid(itr->second));
            if (existing && existing->GetEntry() == want)
                return; // right ride, nothing to do

            DespawnRide(player); // stale or different ride: rebuild
        }

        if (TempSummon* summon = player->SummonCreature(want, pt.x, pt.y, pt.z, pt.o,
                                                        TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
            g_rideVehicle[pguid] = summon->GetGUID().GetRawValue();
    }

    static void DespawnRide(Player* player)
    {
        if (!player)
            return;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto itr = g_rideVehicle.find(pguid);
        if (itr == g_rideVehicle.end())
            return;

        if (Creature* ride = ObjectAccessor::GetCreature(*player, ObjectGuid(itr->second)))
            ride->DespawnOrUnsummon();

        g_rideVehicle.erase(itr);
    }
};

// 1 s heartbeat for the per-player ride objects (see the note above about
// OnQuestStatusChange never being raised toward C++ scripts on this core).
// (FR) Battement de coeur de 1 s pour les objets de transport par joueur
// (meme raison : OnQuestStatusChange n'est jamais declenche vers les scripts
// C++ sur ce noyau).
class isle_stage_mirror_vehicle_world : public WorldScript
{
public:
    isle_stage_mirror_vehicle_world() : WorldScript("isle_stage_mirror_vehicle_world"), _ticker(0) { }

    void OnUpdate(uint32 diff) override
    {
        _ticker += diff;
        if (_ticker < 1000)
            return;
        _ticker = 0;

        for (auto const& itr : ObjectAccessor::GetPlayers())
        {
            Player* player = itr.second;
            if (!player || !player->IsInWorld())
                continue;
            if (player->GetMapId() == MAP_ISLE_859)
                isle_stage_mirror_vehicle::RefreshVehicle(player);
        }
    }

private:
    uint32 _ticker;
};

//By leewheel 2026-09-08
// Registers the quilboar chain managers: the historical Austin / Breka
// checkpoint managers, the whole-island mirrored NPCs (Jaina, Kee-La, Thrall,
// Bo) and the per-player ride objects (taxi + faction chargers).
// (FR) Enregistre les gestionnaires de la chaine quilboar : les gestionnaires
// historiques de points de controle Austin / Breka, les PNJ miroirs de l'ile
// (Jaina, Kee-La, Thrall, Bo) et les objets de transport par joueur (taxi et
// montures de charge de faction).
// End By leewheel
void AddSC_isle_quilboar_stage_npc()
{
    new isle_quilboar_stage_npc();
    new isle_quilboar_stage_world();
    new isle_stage_mirror_npc();
    new isle_stage_mirror_world();
    new isle_stage_mirror_vehicle();
    new isle_stage_mirror_vehicle_world();
}
