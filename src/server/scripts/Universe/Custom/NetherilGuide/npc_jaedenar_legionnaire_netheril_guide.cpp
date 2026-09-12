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
#include "Duration.h"
#include <unordered_map>

// Guide Jaedenar Legionnaire

namespace
{
    enum NetherilGuidePhase : uint8
    {
        NETHERIL_GUIDE_PHASE_SHIP = 0,
        NETHERIL_GUIDE_PHASE_CAMP = 1
    };

    struct PendingGuideInfo
    {
        NetherilGuidePhase phase;
        uint32 entry;
    };

    std::unordered_map<ObjectGuid, PendingGuideInfo> PendingNetherilGuideSummons;

    enum NetherilGuideMisc
    {
        MAP_LEGION_SHIP      = 781,
        MAP_NETHERIL         = 725,
        GOSSIP_ACTION_GUIDE  = 1,

        SAY_SHIP_START         = 0,
        SAY_SHIP_1             = 1,
        SAY_SHIP_2             = 2,
        SAY_SHIP_3             = 3,
        SAY_SHIP_4             = 4,
        SAY_SHIP_5_TELEPORT    = 5,
        SAY_CAMP_ARRIVE        = 6,
        SAY_CAMP_INTRO         = 7,
        SAY_CAMP_QUEST1        = 8,
        SAY_CAMP_GEARZONE      = 9,
        SAY_CAMP_GEAR2         = 10,
        SAY_CAMP_HF            = 11,
        SAY_CAMP_CONVERT       = 12,
        SAY_CAMP_RECIPES       = 13,
        SAY_CAMP_GEAR_ALLIANCE = 14,
        SAY_CAMP_QUEST2        = 15,
        SAY_CAMP_PVP           = 16,
        SAY_CAMP_QUEST3        = 17,
        SAY_CAMP_FAREWELL      = 18
    };

    // Point de depart
    static float const ShipStart[4] = { -11800.7f, 2953.42f, 2745.98f, 1.51408f };

    // Point d'arrivee a Netheril
    static float const CampStart[4] = { -14749.9f, -13192.5f, 34.431f, 1.89685f };

    struct GuidePoint { float x, y, z; };

    // pauseMs
    struct GuideDialogueLine { uint32 afterPoint; uint32 textId; uint32 pauseMs; };

    // Pause
    static uint32 const SHIP_START_PAUSE_MS = 6000;
    static uint32 const CAMP_ARRIVE_PAUSE_MS = 2500;

    // --- Vaisseau de la Legion (map 781) -----------------------------------
    static GuidePoint const ShipPath[] =
    {
        { -11800.9f, 2958.73f, 2745.98f }, // 1
        { -11802.6f, 2967.51f, 2745.98f }, // 2
        { -11804.8f, 2974.75f, 2745.98f }, // 3
        { -11807.0f, 2981.84f, 2745.98f }, // 4
        { -11809.1f, 2988.21f, 2745.98f }, // 5 - dernier arret avant teleportation vers Netheril
    };
    static uint32 const ShipPathSize = sizeof(ShipPath) / sizeof(GuidePoint);

    static GuideDialogueLine const ShipDialogue[] =
    {
        { 1, SAY_SHIP_1,          3000 },
        { 2, SAY_SHIP_2,          3500 },
        { 3, SAY_SHIP_3,          4500 },
        { 4, SAY_SHIP_4,          4500 },
        { 5, SAY_SHIP_5_TELEPORT, 4000 },
    };
    static uint32 const ShipDialogueSize = sizeof(ShipDialogue) / sizeof(GuideDialogueLine);

    // --- Camp de Netheril (map 725) ----------------------------------------
    static GuidePoint const CampPath[] =
    {
        { -14747.2f, -13188.3f, 34.4086f  }, // 1
        { -14746.6f, -13184.1f, 34.3757f  }, // 2
        { -14751.3f, -13170.2f, 27.2209f  }, // 3
        { -14753.8f, -13166.6f, 25.9163f  }, // 4
        { -14759.5f, -13167.7f, 25.9112f  }, // 5
        { -14759.5f, -13177.4f, 25.9613f  }, // 6
        { -14759.5f, -13177.4f, 25.9613f  }, // 7 - "Prenez vos quetes"
        { -14762.0f, -13173.0f, 25.9343f  }, // 8
        { -14765.2f, -13168.4f, 25.9043f  }, // 9
        { -14768.3f, -13159.2f, 22.6657f  }, // 10
        { -14772.5f, -13144.9f, 13.4983f  }, // 11
        { -14769.5f, -13135.3f, 11.118f   }, // 12 - zone des creatures a vaincre
        { -14763.1f, -13139.1f, 11.1562f  }, // 13
        { -14759.8f, -13149.4f, 11.2175f  }, // 14
        { -14757.6f, -13163.0f, 11.2925f  }, // 15
        { -14757.6f, -13177.3f, 11.3627f  }, // 16
        { -14754.1f, -13190.6f, 11.4442f  }, // 17
        { -14749.7f, -13202.0f, 11.4847f  }, // 18
        { -14743.6f, -13205.0f, 11.6028f  }, // 19
        { -14743.6f, -13205.0f, 11.6028f  }, // 20 - bons d'equipement (bis)
        { -14745.3f, -13210.7f, 11.6304f  }, // 21
        { -14742.3f, -13212.8f, 11.6474f  }, // 22
        { -14742.3f, -13212.8f, 11.6474f  }, // 23 - haut fait Netheril
        { -14743.4f, -13220.5f, 11.6889f  }, // 24
        { -14739.2f, -13220.1f, 11.6966f  }, // 25
        { -14739.2f, -13220.1f, 11.6966f  }, // 26 - conversion des bons d'equipement
        { -14743.1f, -13228.0f, 11.7325f  }, // 27
        { -14743.1f, -13228.0f, 11.7325f  }, // 28 - vente de recettes
        { -14746.0f, -13224.1f, 11.7046f  }, // 29
        { -14746.0f, -13224.1f, 11.7046f  }, // 30 - equipement cote Alliance
        { -14736.0f, -13226.2f, 11.7331f  }, // 31
        { -14736.0f, -13226.2f, 11.7331f  }, // 32 - "Prenez vos quetes"
        { -14742.4f, -13222.0f, 11.701f   }, // 33
        { -14742.9f, -13220.8f, 11.6938f  }, // 34
        { -14743.1f, -13216.2f, 11.667f   }, // 35
        { -14745.8f, -13210.1f, 11.6304f  }, // 36
        { -14751.6f, -13212.2f, 11.6287f  }, // 37
        { -14754.4f, -13220.6f, 14.7078f  }, // 38
        { -14754.2f, -13227.9f, 15.774f   }, // 39
        { -14760.6f, -13231.4f, 16.9968f  }, // 40
        { -14761.6f, -13237.7f, 18.511f   }, // 41
        { -14765.7f, -13244.2f, 19.4688f  }, // 42
        { -14769.6f, -13246.9f, 19.4759f  }, // 43
        { -14769.6f, -13246.9f, 19.4759f  }, // 44 - points d'honneur / montures et equipement PvP
        { -14767.0f, -13246.0f, 19.4769f  }, // 45
        { -14765.1f, -13244.9f, 19.475f   }, // 46
        { -14761.7f, -13240.3f, 18.983f   }, // 47
        { -14759.7f, -13233.3f, 17.216f   }, // 48
        { -14757.2f, -13228.9f, 15.7829f  }, // 49
        { -14753.3f, -13227.4f, 15.7719f  }, // 50
        { -14754.3f, -13222.3f, 15.4689f  }, // 51
        { -14752.7f, -13215.2f, 12.1911f  }, // 52
        { -14747.7f, -13209.5f, 11.6229f  }, // 53
        { -14749.4f, -13200.7f, 11.3954f  }, // 54
        { -14753.3f, -13190.5f, 11.45f    }, // 55
        { -14753.9f, -13178.4f, 11.3863f  }, // 56
        { -14754.1f, -13166.0f, 11.3231f  }, // 57
        { -14759.8f, -13143.6f, 11.1955f  }, // 58
        { -14764.8f, -13134.0f, 11.128f   }, // 59
        { -14771.2f, -13117.2f, 7.53597f  }, // 60
        { -14760.8f, -13108.4f, 7.04117f  }, // 61
        { -14751.5f, -13105.8f, 6.54625f  }, // 62
        { -14743.0f, -13106.3f, 6.0161f   }, // 63
        { -14743.7f, -13115.9f, 5.63043f  }, // 64
        { -14743.2f, -13121.7f, 5.49873f  }, // 65
        { -14737.6f, -13127.6f, 5.34968f  }, // 66
        { -14729.5f, -13134.5f, 5.11921f  }, // 67
        { -14704.9f, -13147.9f, 5.5122f   }, // 68
        { -14681.9f, -13158.1f, 5.17059f  }, // 69
        { -14660.0f, -13160.0f, 5.48295f  }, // 70
        { -14648.4f, -13164.3f, 5.50794f  }, // 71
        { -14644.2f, -13169.5f, 5.29382f  }, // 72
        { -14651.6f, -13177.6f, 5.19232f  }, // 73
        { -14661.0f, -13188.2f, 10.729f   }, // 74
        { -14676.5f, -13202.4f, 18.4502f  }, // 75
        { -14686.6f, -13208.2f, 22.9724f  }, // 76
        { -14693.0f, -13219.9f, 28.5689f  }, // 77
        { -14700.8f, -13240.0f, 31.4854f  }, // 78
        { -14707.7f, -13253.1f, 34.9576f  }, // 79
        { -14716.9f, -13261.1f, 38.4446f  }, // 80
        { -14730.4f, -13266.1f, 42.5074f  }, // 81
        { -14742.8f, -13264.7f, 43.9805f  }, // 82
        { -14741.7f, -13258.8f, 43.9501f  }, // 83
        { -14737.3f, -13255.9f, 44.9972f  }, // 84
        { -14723.5f, -13252.6f, 50.6856f  }, // 85
        { -14712.9f, -13243.6f, 55.9641f  }, // 86
        { -14705.8f, -13228.4f, 62.1897f  }, // 87
        { -14703.9f, -13215.5f, 66.7971f  }, // 88
        { -14703.2f, -13205.8f, 68.9845f  }, // 89
        { -14708.7f, -13197.4f, 69.799f   }, // 90
        { -14711.2f, -13198.5f, 69.7805f  }, // 91
        { -14718.6f, -13206.6f, 69.8552f  }, // 92
        { -14718.6f, -13206.6f, 69.8552f  }, // 93 - "Prenez vos quetes"
        { -14716.2f, -13203.9f, 69.8473f  }, // 94
        { -14711.7f, -13199.9f, 69.7862f  }, // 95
        { -14709.4f, -13197.0f, 69.7918f  }, // 96
        { -14717.1f, -13191.4f, 70.1475f  }, // 97
        { -14728.0f, -13192.1f, 71.7904f  }, // 98
        { -14737.0f, -13194.2f, 73.8884f  }, // 99
        { -14746.8f, -13198.8f, 76.581f   }, // 100
        { -14755.3f, -13203.8f, 79.0634f  }, // 101
        { -14765.3f, -13216.3f, 82.9567f  }, // 102
        { -14771.0f, -13228.4f, 85.7955f  }, // 103
        { -14773.0f, -13235.0f, 86.0903f  }, // 104
        { -14769.5f, -13243.6f, 86.2467f  }, // 105
        { -14764.9f, -13243.6f, 86.2219f  }, // 106
        { -14759.0f, -13239.8f, 86.2908f  }, // 107
        { -14752.2f, -13235.3f, 86.2795f  }, // 108
        { -14752.2f, -13235.3f, 86.2795f  }, // 109 - dernieres quetes + au revoir, puis despawn
    };
    static uint32 const CampPathSize = sizeof(CampPath) / sizeof(GuidePoint);

    static GuideDialogueLine const CampDialogue[] =
    {
        { 1,   SAY_CAMP_INTRO,         3000 },
        { 7,   SAY_CAMP_QUEST1,        3500 },
        { 12,  SAY_CAMP_GEARZONE,      8000 },
        { 20,  SAY_CAMP_GEAR2,         3500 },
        { 23,  SAY_CAMP_HF,            6000 },
        { 26,  SAY_CAMP_CONVERT,       6500 },
        { 28,  SAY_CAMP_RECIPES,       2500 },
        { 30,  SAY_CAMP_GEAR_ALLIANCE, 4500 },
        { 32,  SAY_CAMP_QUEST2,        3000 },
        { 44,  SAY_CAMP_PVP,           6500 },
        { 93,  SAY_CAMP_QUEST3,        3500 },
        { 109, SAY_CAMP_FAREWELL,      7000 },
    };
    static uint32 const CampDialogueSize = sizeof(CampDialogue) / sizeof(GuideDialogueLine);

    bool HasDialogueAfter(GuideDialogueLine const* lines, uint32 count, uint32 reachedPoint)
    {
        for (uint32 i = 0; i < count; ++i)
            if (lines[i].afterPoint == reachedPoint)
                return true;
        return false;
    }
}

class npc_jaedenar_legionnaire_netheril_guide : public CreatureScript
{
public:
    npc_jaedenar_legionnaire_netheril_guide() : CreatureScript("npc_jaedenar_legionnaire_netheril_guide") { }

    struct npc_jaedenar_legionnaire_netheril_guideAI : public ScriptedAI
    {
        npc_jaedenar_legionnaire_netheril_guideAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 pathIndex = 0;
        bool guiding = false;
        bool isShipPhase = true;
        ObjectGuid followedPlayerGuid;

        // Pause "explication"
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
                "Emmenez-moi visiter le camp de Netheril.",
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
                PendingNetherilGuideSummons[player->GetGUID()] = { NETHERIL_GUIDE_PHASE_SHIP, entry };
            }
        }

        static void SummonShipGuide(Player* player, uint32 entry)
        {
            if (Creature* guide = player->SummonCreature(entry,
                    ShipStart[0] + 1.5f, ShipStart[1], ShipStart[2], ShipStart[3],
                    TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
            {
                ENSURE_AI(npc_jaedenar_legionnaire_netheril_guideAI, guide->AI())
                    ->StartShipGuiding(player->GetGUID());
            }
        }

        static void SummonCampGuide(Player* player, uint32 entry)
        {
            if (Creature* guide = player->SummonCreature(entry,
                    CampStart[0] + 1.5f, CampStart[1], CampStart[2], CampStart[3],
                    TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
            {
                ENSURE_AI(npc_jaedenar_legionnaire_netheril_guideAI, guide->AI())
                    ->StartCampGuiding();
            }
        }

        void StartShipGuiding(ObjectGuid playerGuid)
        {
            followedPlayerGuid = playerGuid;
            isShipPhase = true;
            pathIndex = 0;
            guiding = true;
            me->SetWalk(true);
            Talk(SAY_SHIP_START);
            BeginPause(SHIP_START_PAUSE_MS, false);
        }

        void StartCampGuiding()
        {
            isShipPhase = false;
            pathIndex = 0;
            guiding = true;
            Talk(SAY_CAMP_ARRIVE);
            BeginPause(CAMP_ARRIVE_PAUSE_MS, false);
        }

        void StepForward()
        {
            uint32 const pathSize = isShipPhase ? ShipPathSize : CampPathSize;

            if (pathIndex >= pathSize)
            {
                guiding = false;

                if (isShipPhase)
                    TeleportToNetheril();
                else
                    me->DespawnOrUnsummon(Milliseconds(8000));

                return;
            }

            GuidePoint const& pt = isShipPhase ? ShipPath[pathIndex] : CampPath[pathIndex];

            bool const hasDialogueOnArrival = isShipPhase
                ? true
                : HasDialogueAfter(CampDialogue, CampDialogueSize, pathIndex + 1);
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
                GuideDialogueLine const* lines = isShipPhase ? ShipDialogue : CampDialogue;
                uint32 const count = isShipPhase ? ShipDialogueSize : CampDialogueSize;

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

        void TeleportToNetheril()
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, followedPlayerGuid))
            {
                uint32 const entry = me->GetEntry();

                player->TeleportTo(MAP_NETHERIL, CampStart[0], CampStart[1], CampStart[2], CampStart[3]);

                if (player->GetMapId() == MAP_NETHERIL)
                    SummonCampGuide(player, entry);
                else
                    PendingNetherilGuideSummons[player->GetGUID()] = { NETHERIL_GUIDE_PHASE_CAMP, entry };
            }

            me->DespawnOrUnsummon(Milliseconds(1000));
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jaedenar_legionnaire_netheril_guideAI(creature);
    }
};

class npc_jaedenar_legionnaire_netheril_guide_player : public PlayerScript
{
public:
    npc_jaedenar_legionnaire_netheril_guide_player() : PlayerScript("npc_jaedenar_legionnaire_netheril_guide_player") { }

    void OnMapChanged(Player* player) override
    {
        auto it = PendingNetherilGuideSummons.find(player->GetGUID());
        if (it == PendingNetherilGuideSummons.end())
            return;

        PendingGuideInfo const info = it->second;

        if (info.phase == NETHERIL_GUIDE_PHASE_SHIP && player->GetMapId() != MAP_LEGION_SHIP)
            return;
        if (info.phase == NETHERIL_GUIDE_PHASE_CAMP && player->GetMapId() != MAP_NETHERIL)
            return;

        PendingNetherilGuideSummons.erase(it);

        if (info.phase == NETHERIL_GUIDE_PHASE_SHIP)
            npc_jaedenar_legionnaire_netheril_guide::npc_jaedenar_legionnaire_netheril_guideAI::SummonShipGuide(player, info.entry);
        else
            npc_jaedenar_legionnaire_netheril_guide::npc_jaedenar_legionnaire_netheril_guideAI::SummonCampGuide(player, info.entry);
    }
};

void AddSC_npc_jaedenar_legionnaire_netheril_guide()
{
    new npc_jaedenar_legionnaire_netheril_guide();
    new npc_jaedenar_legionnaire_netheril_guide_player();
}
