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

#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "CreatureAI.h"
#include "TemporarySummon.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Duration.h"
#include <unordered_map>

// Guide Aysa Cloudsinger

namespace
{
    enum AysaGuidePhase : uint8
    {
        PHASE_SHIP      = 0,
        PHASE_NEROZIAS  = 1,
        PHASE_DREAMWAY  = 2
    };

    struct PendingAysaGuideInfo
    {
        AysaGuidePhase phase;
        uint32 entry;
        bool allianceRoute;
    };

    std::unordered_map<ObjectGuid, PendingAysaGuideInfo> PendingAysaGuideSummons;

    enum AysaGuideMisc
    {
        MAP_LEGION_SHIP     = 781,
        MAP_NEROZIAS        = 726,
        MAP_DREAM_WAY       = 792,
        GOSSIP_ACTION_GUIDE = 1,

        SAY_SHIP_START            = 0,
        SAY_SHIP_WEAPON_EXPLAIN   = 1,
        SAY_SHIP_TELEPORT         = 2,
        SAY_NEROZIAS_VENDORS      = 3,
        SAY_NEROZIAS_TIERS        = 4,
        SAY_NEROZIAS_QUEST        = 5,
        SAY_NEROZIAS_TELEPORT     = 6,
        SAY_DREAMWAY_ARRIVE       = 7,
        SAY_DREAMWAY_DUNGEON      = 8,
        SAY_DREAMWAY_ALMOST       = 9,
        SAY_DREAMWAY_GOAL         = 10,
        SAY_DREAMWAY_FAREWELL     = 11
    };

    // Points de depart
    static float const ShipStart[4]            = { -11786.2f, 2967.84f, 2745.97f, 1.98233f };
    static float const NeroziasAllianceStart[4] = { -15733.9f, -13426.6f, 94.0575f, 4.94998f };
    static float const NeroziasHordeStart[4]    = { -15705.7f, -14232.7f, 78.4658f, 1.54127f };
    static float const DreamWayStart[4]         = { 1658.18f, 1573.7f, 5.84094f, 2.67129f };

    struct GuidePoint { float x, y, z; };

    // pauseMs
    struct GuideDialogueLine { uint32 afterPoint; uint32 textId; uint32 pauseMs; };

    // Pause
    static uint32 const SHIP_START_PAUSE_MS     = 4000;
    static uint32 const DREAMWAY_ARRIVE_PAUSE_MS = 2500;

    // --- Vaisseau de la Legion (map 781) -----------------------------------
    static GuidePoint const ShipPath[] =
    {
        { -11789.2f, 2972.91f, 2745.97f }, // 2 (id 2469)
        { -11794.1f, 2981.0f,  2745.97f }, // 3 (id 2470)
        { -11797.1f, 2988.24f, 2745.97f }, // 4 (id 2471)
        { -11800.3f, 2995.11f, 2745.54f }, // 5 (id 2472) - dernier arret avant teleportation vers Nerozias
    };
    static uint32 const ShipPathSize = sizeof(ShipPath) / sizeof(GuidePoint);

    static GuideDialogueLine const ShipDialogue[] =
    {
        { 1, SAY_SHIP_WEAPON_EXPLAIN, 6000 },
        { 4, SAY_SHIP_TELEPORT,       3500 },
    };
    static uint32 const ShipDialogueSize = sizeof(ShipDialogue) / sizeof(GuideDialogueLine);

    // --- Nerozias (map 726), route Alliance --------------------------------
    static GuidePoint const NeroziasAlliancePath[] =
    {
        { -15737.8f, -13424.7f, 94.0575f  }, // 7  (id 2474) - "il y a des vendeurs"
        { -15748.0f, -13426.7f, 93.156f   }, // 8  (id 2475)
        { -15747.1f, -13431.3f, 93.156f   }, // 9  (id 2476)
        { -15745.5f, -13440.6f, 94.0563f  }, // 10 (id 2477)
        { -15756.6f, -13443.2f, 94.0563f  }, // 11 (id 2478)
        { -15772.2f, -13446.3f, 91.6206f  }, // 12 (id 2479)
        { -15788.9f, -13449.6f, 90.6887f  }, // 13 (id 2480)
        { -15804.3f, -13450.5f, 88.6498f  }, // 14 (id 2481)
        { -15813.5f, -13444.5f, 87.263f   }, // 15 (id 2482) - explication des paliers d'armes
        { -15816.7f, -13437.8f, 86.0904f  }, // 16 (id 2483)
        { -15814.8f, -13426.8f, 88.0302f  }, // 17 (id 2484)
        { -15814.1f, -13418.2f, 89.6917f  }, // 18 (id 2485)
        { -15812.6f, -13412.9f, 90.3633f  }, // 19 (id 2486)
        { -15809.5f, -13412.4f, 90.372f   }, // 20 (id 2487)
        { -15803.8f, -13414.5f, 90.3663f  }, // 21 (id 2488)
        { -15802.4f, -13409.5f, 90.3954f  }, // 22 (id 2489)
        { -15802.9f, -13407.3f, 90.405f   }, // 23 (id 2490)
        { -15805.8f, -13404.7f, 90.4199f  }, // 24 (id 2491) - "prenez votre quete"
        { -15805.2f, -13402.5f, 90.4311f  }, // 25 (id 2492)
        { -15803.6f, -13396.4f, 90.4657f  }, // 26 (id 2493) - dernier arret avant teleportation vers le Chemin du Reve d'Emeraude
    };
    static uint32 const NeroziasAlliancePathSize = sizeof(NeroziasAlliancePath) / sizeof(GuidePoint);

    static GuideDialogueLine const NeroziasAllianceDialogue[] =
    {
        { 1,  SAY_NEROZIAS_VENDORS,  3000 },
        { 9,  SAY_NEROZIAS_TIERS,    7000 },
        { 18, SAY_NEROZIAS_QUEST,    2000 },
        { 20, SAY_NEROZIAS_TELEPORT, 3500 },
    };
    static uint32 const NeroziasAllianceDialogueSize = sizeof(NeroziasAllianceDialogue) / sizeof(GuideDialogueLine);

    // --- Nerozias (map 726), route Horde -----------------------------------
    static GuidePoint const NeroziasHordePath[] =
    {
        { -15705.8f, -14229.0f, 78.4658f }, // 1  (id 2514) - "il y a des vendeurs"
        { -15705.6f, -14221.2f, 80.0325f }, // 2  (id 2515)
        { -15706.3f, -14209.0f, 80.5486f }, // 3  (id 2516)
        { -15705.9f, -14198.4f, 80.5628f }, // 4  (id 2517)
        { -15706.1f, -14187.0f, 80.0747f }, // 5  (id 2518)
        { -15707.4f, -14174.0f, 77.9162f }, // 6  (id 2519)
        { -15712.6f, -14170.0f, 77.2682f }, // 7  (id 2520)
        { -15722.6f, -14172.1f, 77.5684f }, // 8  (id 2521)
        { -15730.6f, -14175.0f, 78.2471f }, // 9  (id 2522)
        { -15740.1f, -14183.6f, 78.0563f }, // 10 (id 2523)
        { -15748.1f, -14190.4f, 77.6382f }, // 11 (id 2524)
        { -15756.9f, -14195.3f, 76.7782f }, // 12 (id 2525)
        { -15762.3f, -14198.7f, 76.5981f }, // 13 (id 2526) - explication des paliers d'armes
        { -15764.7f, -14205.3f, 76.6578f }, // 14 (id 2527)
        { -15765.9f, -14213.8f, 77.6841f }, // 15 (id 2528)
        { -15766.3f, -14222.4f, 78.238f  }, // 16 (id 2529)
        { -15766.4f, -14229.0f, 78.238f  }, // 17 (id 2530)
        { -15766.8f, -14236.0f, 78.238f  }, // 18 (id 2531)
        { -15766.9f, -14239.9f, 78.238f  }, // 19 (id 2532) - "prenez votre quete"
        { -15765.0f, -14236.8f, 78.238f  }, // 20 (id 2533)
        { -15764.5f, -14231.1f, 78.238f  }, // 21 (id 2534)
        { -15764.2f, -14225.0f, 78.238f  }, // 22 (id 2535)
        { -15762.9f, -14222.9f, 78.238f  }, // 23 (id 2536) - dernier arret avant teleportation vers le Chemin du Reve d'Emeraude
    };
    static uint32 const NeroziasHordePathSize = sizeof(NeroziasHordePath) / sizeof(GuidePoint);

    static GuideDialogueLine const NeroziasHordeDialogue[] =
    {
        { 1,  SAY_NEROZIAS_VENDORS,  3000 },
        { 13, SAY_NEROZIAS_TIERS,    7000 },
        { 19, SAY_NEROZIAS_QUEST,    2000 },
        { 23, SAY_NEROZIAS_TELEPORT, 3500 },
    };
    static uint32 const NeroziasHordeDialogueSize = sizeof(NeroziasHordeDialogue) / sizeof(GuideDialogueLine);

    // --- Chemin du Reve d'Emeraude (map 792) -------------------------------
    static GuidePoint const DreamWayPath[] =
    {
        { 1649.6f,  1578.15f, 4.58457f  }, // 28 (id 2495)
        { 1636.36f, 1584.94f, 6.0346f   }, // 29 (id 2496)
        { 1619.01f, 1589.25f, 6.81157f  }, // 30 (id 2497) - "nous nous rendons au donjon"
        { 1611.14f, 1588.48f, 8.07623f  }, // 31 (id 2498)
        { 1601.97f, 1594.01f, 9.95188f  }, // 32 (id 2499)
        { 1594.54f, 1602.26f, 12.1492f  }, // 33 (id 2500)
        { 1589.08f, 1611.03f, 13.4126f  }, // 34 (id 2501)
        { 1581.22f, 1617.85f, 15.3402f  }, // 35 (id 2502)
        { 1572.42f, 1622.32f, 17.5988f  }, // 36 (id 2503)
        { 1561.27f, 1624.02f, 19.9039f  }, // 37 (id 2504) - "on y est presque"
        { 1550.42f, 1623.28f, 21.8766f  }, // 38 (id 2505)
        { 1539.96f, 1620.7f,  24.269f   }, // 39 (id 2506)
        { 1530.37f, 1621.18f, 26.001f   }, // 40 (id 2507)
        { 1520.25f, 1623.82f, 27.3645f  }, // 41 (id 2508)
        { 1518.31f, 1635.31f, 28.5766f  }, // 42 (id 2509)
        { 1515.2f,  1645.63f, 28.9848f  }, // 43 (id 2510)
        { 1506.46f, 1649.74f, 30.4646f  }, // 44 (id 2511) - "allez jusqu'au bout du donjon"
        { 1506.46f, 1649.74f, 30.4646f  }, // 45 (id 2512) - au revoir, puis despawn
    };
    static uint32 const DreamWayPathSize = sizeof(DreamWayPath) / sizeof(GuidePoint);

    static GuideDialogueLine const DreamWayDialogue[] =
    {
        { 3,  SAY_DREAMWAY_DUNGEON,  3500 },
        { 10, SAY_DREAMWAY_ALMOST,   2000 },
        { 17, SAY_DREAMWAY_GOAL,     3000 },
        { 18, SAY_DREAMWAY_FAREWELL, 5000 },
    };
    static uint32 const DreamWayDialogueSize = sizeof(DreamWayDialogue) / sizeof(GuideDialogueLine);

    bool HasDialogueAfter(GuideDialogueLine const* lines, uint32 count, uint32 reachedPoint)
    {
        for (uint32 i = 0; i < count; ++i)
            if (lines[i].afterPoint == reachedPoint)
                return true;
        return false;
    }
}

class npc_aysa_cloudsinger_nerozias_guide : public CreatureScript
{
public:
    npc_aysa_cloudsinger_nerozias_guide() : CreatureScript("npc_aysa_cloudsinger_nerozias_guide") { }

    struct npc_aysa_cloudsinger_nerozias_guideAI : public ScriptedAI
    {
        npc_aysa_cloudsinger_nerozias_guideAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 pathIndex = 0;
        bool guiding = false;
        AysaGuidePhase phase = PHASE_SHIP;
        bool allianceRoute = true;
        ObjectGuid followedPlayerGuid;

        // Pause
        bool paused = false;
        uint32 pauseTimer = 0;
        bool pendingIncrementAfterPause = false;

        void BeginPause(uint32 ms, bool incrementPathIndexAfter)
        {
            paused = true;
            pauseTimer = ms;
            pendingIncrementAfterPause = incrementPathIndexAfter;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!paused)
                return;

            if (diff >= pauseTimer)
            {
                paused = false;

                if (pendingIncrementAfterPause)
                {
                    pendingIncrementAfterPause = false;
                    ++pathIndex;
                }

                StepForward();
            }
            else
                pauseTimer -= diff;
        }

        bool OnGossipHello(Player* player)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Emmenez-moi visiter Nerozias et le Chemin du Reve d'Emeraude.",
                GOSSIP_SENDER_MAIN, GOSSIP_ACTION_GUIDE);

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId)
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            if (action == GOSSIP_ACTION_GUIDE)
            {
                CloseGossipMenuFor(player);
                BeginJourney(player);
            }

            return true;
        }

        // Teleport
        void BeginJourney(Player* player)
        {
            uint32 const entry = me->GetEntry();

            player->TeleportTo(MAP_LEGION_SHIP, ShipStart[0], ShipStart[1], ShipStart[2], ShipStart[3]);

            if (player->GetMapId() == MAP_LEGION_SHIP)
            {
                SummonShipGuide(player, entry);
            }
            else
            {
                PendingAysaGuideSummons[player->GetGUID()] = { PHASE_SHIP, entry, true };
            }
        }

        static void SummonShipGuide(Player* player, uint32 entry)
        {
            if (Creature* guide = player->SummonCreature(entry,
                    ShipStart[0] + 1.5f, ShipStart[1], ShipStart[2], ShipStart[3],
                    TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
            {
                ENSURE_AI(npc_aysa_cloudsinger_nerozias_guideAI, guide->AI())
                    ->StartShipGuiding(player->GetGUID());
            }
        }

        static void SummonNeroziasGuide(Player* player, uint32 entry, bool isAllianceRoute)
        {
            float const* start = isAllianceRoute ? NeroziasAllianceStart : NeroziasHordeStart;

            if (Creature* guide = player->SummonCreature(entry,
                    start[0] + 1.5f, start[1], start[2], start[3],
                    TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
            {
                ENSURE_AI(npc_aysa_cloudsinger_nerozias_guideAI, guide->AI())
                    ->StartNeroziasGuiding(player->GetGUID(), isAllianceRoute);
            }
        }

        static void SummonDreamWayGuide(Player* player, uint32 entry)
        {
            if (Creature* guide = player->SummonCreature(entry,
                    DreamWayStart[0] + 1.5f, DreamWayStart[1], DreamWayStart[2], DreamWayStart[3],
                    TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
            {
                ENSURE_AI(npc_aysa_cloudsinger_nerozias_guideAI, guide->AI())
                    ->StartDreamWayGuiding(player->GetGUID());
            }
        }

        void StartShipGuiding(ObjectGuid playerGuid)
        {
            followedPlayerGuid = playerGuid;
            phase = PHASE_SHIP;
            pathIndex = 0;
            guiding = true;
            me->SetWalk(true);
            Talk(SAY_SHIP_START);
            BeginPause(SHIP_START_PAUSE_MS, false);
        }

        void StartNeroziasGuiding(ObjectGuid playerGuid, bool isAllianceRoute)
        {
            followedPlayerGuid = playerGuid;
            allianceRoute = isAllianceRoute;
            phase = PHASE_NEROZIAS;
            pathIndex = 0;
            guiding = true;
            StepForward();
        }

        void StartDreamWayGuiding(ObjectGuid playerGuid)
        {
            followedPlayerGuid = playerGuid;
            phase = PHASE_DREAMWAY;
            pathIndex = 0;
            guiding = true;
            Talk(SAY_DREAMWAY_ARRIVE);
            BeginPause(DREAMWAY_ARRIVE_PAUSE_MS, false);
        }

        GuidePoint const* CurrentPathArray() const
        {
            switch (phase)
            {
                case PHASE_SHIP:     return ShipPath;
                case PHASE_NEROZIAS: return allianceRoute ? NeroziasAlliancePath : NeroziasHordePath;
                case PHASE_DREAMWAY:
                default:             return DreamWayPath;
            }
        }

        uint32 CurrentPathSize() const
        {
            switch (phase)
            {
                case PHASE_SHIP:     return ShipPathSize;
                case PHASE_NEROZIAS: return allianceRoute ? NeroziasAlliancePathSize : NeroziasHordePathSize;
                case PHASE_DREAMWAY:
                default:             return DreamWayPathSize;
            }
        }

        GuideDialogueLine const* CurrentDialogueLines() const
        {
            switch (phase)
            {
                case PHASE_SHIP:     return ShipDialogue;
                case PHASE_NEROZIAS: return allianceRoute ? NeroziasAllianceDialogue : NeroziasHordeDialogue;
                case PHASE_DREAMWAY:
                default:             return DreamWayDialogue;
            }
        }

        uint32 CurrentDialogueSize() const
        {
            switch (phase)
            {
                case PHASE_SHIP:     return ShipDialogueSize;
                case PHASE_NEROZIAS: return allianceRoute ? NeroziasAllianceDialogueSize : NeroziasHordeDialogueSize;
                case PHASE_DREAMWAY:
                default:             return DreamWayDialogueSize;
            }
        }

        void StepForward()
        {
            uint32 const pathSize = CurrentPathSize();

            if (pathIndex >= pathSize)
            {
                guiding = false;

                switch (phase)
                {
                    case PHASE_SHIP:
                        TeleportToNerozias();
                        break;
                    case PHASE_NEROZIAS:
                        TeleportToDreamWay();
                        break;
                    case PHASE_DREAMWAY:
                    default:
                        me->DespawnOrUnsummon(Milliseconds(8000));
                        break;
                }

                return;
            }

            GuidePoint const& pt = CurrentPathArray()[pathIndex];

            bool const hasDialogueOnArrival = (phase == PHASE_SHIP)
                ? true
                : HasDialogueAfter(CurrentDialogueLines(), CurrentDialogueSize(), pathIndex + 1);
            me->SetWalk(hasDialogueOnArrival);

            me->GetMotionMaster()->MovePoint(pathIndex, pt.x, pt.y, pt.z);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE || !guiding)
                return;

            if (id == pathIndex)
            {
                uint32 const reachedPoint = pathIndex + 1;
                GuideDialogueLine const* lines = CurrentDialogueLines();
                uint32 const count = CurrentDialogueSize();

                for (uint32 i = 0; i < count; ++i)
                {
                    if (lines[i].afterPoint == reachedPoint)
                    {
                        Talk(lines[i].textId);
						
                        BeginPause(lines[i].pauseMs, true);
                        return;
                    }
                }

                ++pathIndex;
                StepForward();
            }
        }

        // Fin de la partie "vaisseau"
        void TeleportToNerozias()
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, followedPlayerGuid))
            {
                uint32 const entry = me->GetEntry();
                bool const isAlliance = player->GetTeamId() == TEAM_ALLIANCE;
                float const* dest = isAlliance ? NeroziasAllianceStart : NeroziasHordeStart;

                player->TeleportTo(MAP_NEROZIAS, dest[0], dest[1], dest[2], dest[3]);

                if (player->GetMapId() == MAP_NEROZIAS)
                    SummonNeroziasGuide(player, entry, isAlliance);
                else
                    PendingAysaGuideSummons[player->GetGUID()] = { PHASE_NEROZIAS, entry, isAlliance };
            }

            me->DespawnOrUnsummon(Milliseconds(1000));
        }

        // Fin de la partie Nerozias
        void TeleportToDreamWay()
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, followedPlayerGuid))
            {
                uint32 const entry = me->GetEntry();

                player->TeleportTo(MAP_DREAM_WAY, DreamWayStart[0], DreamWayStart[1], DreamWayStart[2], DreamWayStart[3]);

                if (player->GetMapId() == MAP_DREAM_WAY)
                    SummonDreamWayGuide(player, entry);
                else
                    PendingAysaGuideSummons[player->GetGUID()] = { PHASE_DREAMWAY, entry, false };
            }

            me->DespawnOrUnsummon(Milliseconds(1000));
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_aysa_cloudsinger_nerozias_guideAI(creature);
    }
};

class npc_aysa_cloudsinger_nerozias_guide_player : public PlayerScript
{
public:
    npc_aysa_cloudsinger_nerozias_guide_player() : PlayerScript("npc_aysa_cloudsinger_nerozias_guide_player") { }

    void OnMapChanged(Player* player) override
    {
        auto it = PendingAysaGuideSummons.find(player->GetGUID());
        if (it == PendingAysaGuideSummons.end())
            return;

        PendingAysaGuideInfo const info = it->second;

        if (info.phase == PHASE_SHIP && player->GetMapId() != MAP_LEGION_SHIP)
            return;
        if (info.phase == PHASE_NEROZIAS && player->GetMapId() != MAP_NEROZIAS)
            return;
        if (info.phase == PHASE_DREAMWAY && player->GetMapId() != MAP_DREAM_WAY)
            return;

        PendingAysaGuideSummons.erase(it);

        switch (info.phase)
        {
            case PHASE_SHIP:
                npc_aysa_cloudsinger_nerozias_guide::npc_aysa_cloudsinger_nerozias_guideAI::SummonShipGuide(player, info.entry);
                break;
            case PHASE_NEROZIAS:
                npc_aysa_cloudsinger_nerozias_guide::npc_aysa_cloudsinger_nerozias_guideAI::SummonNeroziasGuide(player, info.entry, info.allianceRoute);
                break;
            case PHASE_DREAMWAY:
                npc_aysa_cloudsinger_nerozias_guide::npc_aysa_cloudsinger_nerozias_guideAI::SummonDreamWayGuide(player, info.entry);
                break;
        }
    }
};

void AddSC_npc_aysa_cloudsinger_nerozias_guide()
{
    new npc_aysa_cloudsinger_nerozias_guide();
    new npc_aysa_cloudsinger_nerozias_guide_player();
}
