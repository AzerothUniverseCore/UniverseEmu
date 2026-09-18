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
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "WorldSession.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "GossipDef.h"

#include "LegionShipIntro.h"

#include <unordered_map>

constexpr uint32 GOSSIP_ACTION_START_INTRO = GOSSIP_ACTION_INFO_DEF + 1;

namespace
{
    std::unordered_map<uint64, ObjectGuid> g_activeEscorts;

    bool HasActiveEscort(Player* player)
    {
        if (!player)
            return false;

        auto itr = g_activeEscorts.find(player->GetGUID().GetRawValue());
        if (itr == g_activeEscorts.end())
            return false;

        return ObjectAccessor::GetCreature(*player, itr->second) != nullptr;
    }

    char const* LocalizedGossipText(Player* player, uint32 syphrenaStringEntry)
    {
        return sObjectMgr->GetSyphrenaString(syphrenaStringEntry, player->GetSession()->GetSessionDbLocaleIndex());
    }

    void KickOffGuiding(Creature* escort);

    void StartPersonalEscort(Player* player, uint32 entry)
    {
        if (!player || HasActiveEscort(player))
            return;

        LegionShipIntro::Point const& spawn = LegionShipIntro::GetSpawnPoint(entry);
        TempSummon* escort = player->SummonCreature(entry, spawn.x, spawn.y, spawn.z, spawn.o,
            TEMPSUMMON_MANUAL_DESPAWN, 0ms, true);
        if (!escort)
            return;

        g_activeEscorts[player->GetGUID().GetRawValue()] = escort->GetGUID();

        KickOffGuiding(escort);
    }

    void EndPersonalEscort(Player* player)
    {
        if (!player)
            return;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto itr = g_activeEscorts.find(pguid);
        if (itr == g_activeEscorts.end())
            return;

        if (Creature* escort = ObjectAccessor::GetCreature(*player, itr->second))
            escort->DespawnOrUnsummon();

        g_activeEscorts.erase(itr);
    }
}

class npc_legion_ship_illidan : public CreatureScript
{
public:
    npc_legion_ship_illidan() : CreatureScript("npc_legion_ship_illidan") { }

    struct npc_legion_ship_illidanAI : public ScriptedAI
    {
        npc_legion_ship_illidanAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        bool OnGossipHello(Player* player) override
        {
            if (!player->GetQuestRewardStatus(LegionShipIntro::QUEST_ID))
                Talk(LegionShipIntro::SAY_START);

            return false;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_legion_ship_illidanAI(creature);
    }
};

class npc_legion_ship_illidari : public CreatureScript
{
public:
    npc_legion_ship_illidari() : CreatureScript("npc_legion_ship_illidari") { }

    struct npc_legion_ship_illidariAI : public ScriptedAI
    {
        npc_legion_ship_illidariAI(Creature* creature) : ScriptedAI(creature),
            pathIndex(0),
            stallCheckTimer(0),
            finished(false),
            despawnTimer(0)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        Player* GetOwningPlayer() const
        {
            if (TempSummon* summon = me->ToTempSummon())
                return ObjectAccessor::GetPlayer(*me, summon->GetSummonerGUID());
            return nullptr;
        }

        bool OnGossipHello(Player* player) override
        {
            if (me->ToTempSummon())
                return false;

            ClearGossipMenuFor(player);

            if (player->GetQuestStatus(LegionShipIntro::QUEST_ID) != QUEST_STATUS_INCOMPLETE)
            {
                CloseGossipMenuFor(player);
                return true;
            }

            if (HasActiveEscort(player))
                AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                    LocalizedGossipText(player, LegionShipIntro::STR_GOSSIP_ALREADY_GUIDING),
                    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            else
                AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                    LocalizedGossipText(player, LegionShipIntro::STR_GOSSIP_GUIDE_ME),
                    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_START_INTRO);

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
        {
            CloseGossipMenuFor(player);

            if (!me->ToTempSummon() && !HasActiveEscort(player))
                StartPersonalEscort(player, me->GetEntry());

            return true;
        }

        void StartGuiding()
        {
            pathIndex = 0;
            Talk(LegionShipIntro::SAY_START);
            StepForward();
        }

        void StepForward()
        {
            LegionShipIntro::Point const* path = LegionShipIntro::GetPath(me->GetEntry());
            if (!path)
                return;

            if (pathIndex >= LegionShipIntro::PATH_SIZE)
            {
                FinishEscort();
                return;
            }

            LegionShipIntro::Point const& p = path[pathIndex];
            me->GetMotionMaster()->MovePoint(pathIndex, p.x, p.y, p.z);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            HandleReachedPoint(id);
        }

        void HandleReachedPoint(uint32 reachedIndex)
        {
            uint32 reachedPoint = reachedIndex + 1;
            pathIndex = reachedIndex + 1;

            uint8 textGroup = LegionShipIntro::GetMilestoneTextGroup(reachedPoint);
            if (textGroup != LegionShipIntro::NO_TEXT)
                Talk(textGroup);

            if (reachedPoint == LegionShipIntro::TELEPORT_AFTER_POINT)
            {
                LegionShipIntro::Point const& dest = LegionShipIntro::GetTeleportDestination(me->GetEntry());
                me->NearTeleportTo(dest.x, dest.y, dest.z, dest.o);

                if (Player* owner = GetOwningPlayer())
                    owner->NearTeleportTo(dest.x, dest.y, dest.z, dest.o);

                Talk(LegionShipIntro::SAY_POST_JUMP);
            }

            if (pathIndex >= LegionShipIntro::PATH_SIZE)
            {
                FinishEscort();
                return;
            }

            StepForward();
        }

        void FinishEscort()
        {
            Talk(LegionShipIntro::SAY_END);

            if (Player* owner = GetOwningPlayer())
                if (owner->GetQuestStatus(LegionShipIntro::QUEST_ID) == QUEST_STATUS_INCOMPLETE)
                    owner->AreaExploredOrEventHappens(LegionShipIntro::QUEST_ID);

            finished = true;
            despawnTimer = LegionShipIntro::POST_TOUR_DESPAWN_DELAY_MS;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me->ToTempSummon())
                return;

            if (finished)
            {
                if (despawnTimer > diff)
                {
                    despawnTimer -= diff;
                    return;
                }

                if (Player* owner = GetOwningPlayer())
                    EndPersonalEscort(owner);
                else
                    me->DespawnOrUnsummon();
                return;
            }

            if (pathIndex >= LegionShipIntro::PATH_SIZE)
                return;

            LegionShipIntro::Point const* path = LegionShipIntro::GetPath(me->GetEntry());
            if (!path)
                return;

            stallCheckTimer += diff;
            if (stallCheckTimer >= 1000)
            {
                stallCheckTimer = 0;

                LegionShipIntro::Point const& target = path[pathIndex];
                if (me->GetDistance(target.x, target.y, target.z) < 5.0f)
                    HandleReachedPoint(pathIndex);
            }
        }

    private:
        uint32 pathIndex;
        uint32 stallCheckTimer;
        bool finished;
        uint32 despawnTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_legion_ship_illidariAI(creature);
    }
};

namespace
{
    void KickOffGuiding(Creature* escort)
    {
        if (npc_legion_ship_illidari::npc_legion_ship_illidariAI* ai =
                ENSURE_AI(npc_legion_ship_illidari::npc_legion_ship_illidariAI, escort->AI()))
            ai->StartGuiding();
    }
}

class LegionShipIntroCleanup : public PlayerScript
{
public:
    LegionShipIntroCleanup() : PlayerScript("LegionShipIntroCleanup") { }

    void OnLogout(Player* player) override
    {
        EndPersonalEscort(player);
    }

    void OnMapChanged(Player* player) override
    {
        if (player->GetMapId() != LegionShipIntro::MAP_ID)
            EndPersonalEscort(player);
    }
};

void AddSC_legion_ship_intro()
{
    new npc_legion_ship_illidan();
    new npc_legion_ship_illidari();
    new LegionShipIntroCleanup();
}
