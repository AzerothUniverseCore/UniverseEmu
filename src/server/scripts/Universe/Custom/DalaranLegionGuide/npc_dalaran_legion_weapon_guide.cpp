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
#include "Duration.h"
#include <unordered_map>

static std::unordered_map<ObjectGuid, uint32> PendingDalaranGuideSummons;

enum DalaranWeaponGuideMisc
{
    MAP_DALARAN_LEGION     = 781,
    GOSSIP_ACTION_GUIDE    = 1,

    SAY_START              = 0,
    SAY_ARRIVED            = 1,
    SAY_INFO_CRYSTALS      = 2,
    SAY_INFO_COST          = 3,
    SAY_INFO_FARM          = 4,
    SAY_INFO_ARENA         = 5
};

struct GuideDialogueLine { uint32 afterPoint; uint32 textId; };
static GuideDialogueLine const GuideDialogue[] =
{
    { 8,  SAY_INFO_CRYSTALS },
    { 20, SAY_INFO_COST     },
    { 32, SAY_INFO_FARM     },
    { 42, SAY_INFO_ARENA    },
};

static float const DalaranArrival[4] = { -11908.8f, 2961.1f, 1857.4f, 5.04f };

struct GuidePoint { float x, y, z; };
static GuidePoint const WeaponUpgradePath[] =
{
    { -11906.6f, 2950.21f, 1857.41f }, // 1
    { -11911.2f, 2934.62f, 1857.43f }, // 2
    { -11913.7f, 2923.23f, 1856.9f  }, // 3
    { -11913.1f, 2908.37f, 1856.66f }, // 4
    { -11911.5f, 2895.16f, 1856.02f }, // 5
    { -11906.2f, 2885.11f, 1856.4f  }, // 6
    { -11909.9f, 2871.94f, 1856.4f  }, // 7
    { -11911.0f, 2863.52f, 1856.31f }, // 8
    { -11908.4f, 2859.74f, 1856.33f }, // 9
    { -11895.0f, 2855.06f, 1849.79f }, // 10
    { -11886.8f, 2854.51f, 1849.8f  }, // 11
    { -11878.2f, 2841.92f, 1848.88f }, // 12
    { -11871.0f, 2843.37f, 1848.85f }, // 13
    { -11857.5f, 2838.71f, 1848.69f }, // 14
    { -11822.5f, 2823.45f, 1848.39f }, // 15
    { -11797.2f, 2813.51f, 1847.01f }, // 16
    { -11776.0f, 2801.09f, 1845.36f }, // 17
    { -11760.0f, 2805.07f, 1844.89f }, // 18
    { -11745.3f, 2814.03f, 1845.56f }, // 19
    { -11741.0f, 2818.27f, 1846.84f }, // 20
    { -11736.5f, 2823.09f, 1846.89f }, // 21
    { -11736.5f, 2827.54f, 1846.89f }, // 22
    { -11740.3f, 2831.2f,  1846.89f }, // 23
    { -11743.1f, 2836.08f, 1846.89f }, // 24
    { -11745.3f, 2839.24f, 1846.89f }, // 25
    { -11747.0f, 2840.96f, 1846.89f }, // 26
    { -11738.4f, 2849.66f, 1839.3f  }, // 27
    { -11730.2f, 2858.27f, 1832.39f }, // 28
    { -11742.5f, 2870.26f, 1832.39f }, // 29
    { -11754.6f, 2856.26f, 1821.01f }, // 30
    { -11761.2f, 2848.88f, 1817.46f }, // 31
    { -11766.5f, 2838.78f, 1817.52f }, // 32
    { -11772.2f, 2828.56f, 1817.52f }, // 33
    { -11776.5f, 2818.82f, 1817.26f }, // 34
    { -11785.9f, 2797.13f, 1817.26f }, // 35
    { -11790.0f, 2782.3f,  1817.26f }, // 36
    { -11795.0f, 2767.24f, 1817.26f }, // 37
    { -11805.2f, 2751.38f, 1817.26f }, // 38
    { -11816.4f, 2732.68f, 1817.26f }, // 39
    { -11819.1f, 2722.95f, 1817.26f }, // 40
    { -11801.7f, 2715.09f, 1817.43f }, // 41
    { -11788.2f, 2708.83f, 1817.22f }, // 42
    { -11777.0f, 2704.13f, 1817.2f  }, // 43
    { -11768.4f, 2700.2f,  1816.12f }, // 44
    { -11758.5f, 2694.4f,  1816.12f }, // 45
    { -11750.9f, 2687.98f, 1816.12f }, // 46
    { -11747.8f, 2680.56f, 1816.12f }, // 47
    { -11746.0f, 2673.65f, 1817.52f }, // 48
    { -11744.4f, 2671.69f, 1817.52f }, // 49
};
static uint32 const WeaponUpgradePathSize = sizeof(WeaponUpgradePath) / sizeof(GuidePoint);

class npc_dalaran_legion_weapon_guide : public CreatureScript
{
public:
    npc_dalaran_legion_weapon_guide() : CreatureScript("npc_dalaran_legion_weapon_guide") { }

    struct npc_dalaran_legion_weapon_guideAI : public ScriptedAI
    {
        npc_dalaran_legion_weapon_guideAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 pathIndex = 0;
        bool guiding = false;
        bool isTemporaryGuide = false;

        void InitializeAI() override
        {
            isTemporaryGuide = me->IsSummon();
        }

        bool OnGossipHello(Player* player)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Ramene-moi a Dalaran Legion et guide-moi vers l'amelioration des armes prodigieuses.",
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
            PendingDalaranGuideSummons[player->GetGUID()] = me->GetEntry();

            player->TeleportTo(MAP_DALARAN_LEGION, DalaranArrival[0], DalaranArrival[1],
                DalaranArrival[2], DalaranArrival[3]);
        }

        void StartGuiding()
        {
            pathIndex = 0;
            guiding = true;
            me->SetWalk(false);
            Talk(SAY_START);
            StepForward();
        }

        void StepForward()
        {
            if (pathIndex >= WeaponUpgradePathSize)
            {
                guiding = false;
                Talk(SAY_ARRIVED);

                if (isTemporaryGuide)
                    me->DespawnOrUnsummon(Milliseconds(8000));

                return;
            }

            GuidePoint const& pt = WeaponUpgradePath[pathIndex];
            me->GetMotionMaster()->MovePoint(pathIndex, pt.x, pt.y, pt.z);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE || !guiding)
                return;

            if (id == pathIndex)
            {
                uint32 const reachedPoint = pathIndex + 1;
                for (GuideDialogueLine const& line : GuideDialogue)
                {
                    if (line.afterPoint == reachedPoint)
                    {
                        Talk(line.textId);
                        break;
                    }
                }

                ++pathIndex;
                StepForward();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_dalaran_legion_weapon_guideAI(creature);
    }
};

class npc_dalaran_legion_weapon_guide_player : public PlayerScript
{
public:
    npc_dalaran_legion_weapon_guide_player() : PlayerScript("npc_dalaran_legion_weapon_guide_player") { }

    void OnMapChanged(Player* player) override
    {
        if (player->GetMapId() != MAP_DALARAN_LEGION)
            return;

        auto it = PendingDalaranGuideSummons.find(player->GetGUID());
        if (it == PendingDalaranGuideSummons.end())
            return;

        uint32 const guideEntry = it->second;
        PendingDalaranGuideSummons.erase(it);

        if (Creature* guide = player->SummonCreature(guideEntry,
                DalaranArrival[0] + 1.5f, DalaranArrival[1], DalaranArrival[2], DalaranArrival[3],
                TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0)))
        {
            ENSURE_AI(npc_dalaran_legion_weapon_guide::npc_dalaran_legion_weapon_guideAI, guide->AI())->StartGuiding();
        }
    }
};

void AddSC_npc_dalaran_legion_weapon_guide()
{
    new npc_dalaran_legion_weapon_guide();
    new npc_dalaran_legion_weapon_guide_player();
}
