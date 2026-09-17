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

#include "BattleBrokenShoreEscort.h"
#include "TheBattleBrokenShore.h"
#include "ScenarioRoster.h"
#include "IllidariAbilities.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "TemporarySummon.h"
#include "Duration.h"
#include "Log.h"
#include "Chat.h"
#include "WorldSession.h"
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define LEGION_SCENARIO_DEBUG_LOG

enum LegionEscortMisc
{
    GOSSIP_ACTION_LAUNCH_COMBAT = 1,

    SAY_ESCORT_START        = 0, // Spawn point
    SAY_ESCORT_SETOFF       = 1, // checkpoint 1
    SAY_ESCORT_TREMOR       = 2, // checkpoint 10
    SAY_ESCORT_RIDGE        = 3, // checkpoint 14
    SAY_ESCORT_COMBAT_READY = 4, // checkpoint 21
    SAY_ESCORT_VICTORY      = 5, // checkpoint 22
    SAY_ESCORT_REST         = 6, // checkpoint 30

    SAY_ESCORT_MARCH35        = 7,  // after checkpoint 35
    SAY_ESCORT_MARCH50        = 8,  // after checkpoint 50
    SAY_ESCORT_MARCH65        = 9,  // after checkpoint 65
    SAY_ESCORT_MARCH80        = 10, // after checkpoint 80
    SAY_ESCORT_MARCH95        = 11, // after checkpoint 95
    SAY_ESCORT_BOSS1_READY    = 12, // checkpoint 107
    SAY_ESCORT_BOSS1_DOWN     = 13, // after checkpoint 115
    SAY_ESCORT_MARCH125       = 14, // after checkpoint 125
    SAY_ESCORT_MARCH135       = 15, // after checkpoint 135
    SAY_ESCORT_BOSS2_APPROACH = 16, // after checkpoint 147
    SAY_ESCORT_BOSS2_TENSION  = 17, // after checkpoint 148
    SAY_ESCORT_BOSS2_READY    = 18, // checkpoint 149
    SAY_ESCORT_END            = 19  // final boss scenario complete
};

enum LegionEscortGateType
{
    GATE_NONE    = 0,
    GATE_GENERIC = 1, // checkpoint 21
    GATE_BOSS1   = 2, // checkpoint 107
    GATE_BOSS2   = 3  // checkpoint 149
};

struct EscortDialogueLine { uint32 afterPoint; uint32 textId; };
static EscortDialogueLine const ESCORT_DIALOGUE[] =
{
    { 1,   SAY_ESCORT_SETOFF        },
    { 10,  SAY_ESCORT_TREMOR        },
    { 14,  SAY_ESCORT_RIDGE         },
    { 22,  SAY_ESCORT_VICTORY       },
    { 30,  SAY_ESCORT_REST          },
    { 35,  SAY_ESCORT_MARCH35       },
    { 50,  SAY_ESCORT_MARCH50       },
    { 65,  SAY_ESCORT_MARCH65       },
    { 80,  SAY_ESCORT_MARCH80       },
    { 95,  SAY_ESCORT_MARCH95       },
    { 115, SAY_ESCORT_BOSS1_DOWN    },
    { 125, SAY_ESCORT_MARCH125      },
    { 135, SAY_ESCORT_MARCH135      },
    { 147, SAY_ESCORT_BOSS2_APPROACH },
    { 148, SAY_ESCORT_BOSS2_TENSION  },
};

namespace
{
    bool IsEscortLeaderEntry(uint32 entry)
    {
        return entry == LegionEscort::HORDE_LEADER || entry == LegionEscort::ALLIANCE_LEADER;
    }

    void DespawnParty(Player* player);
    bool IsPartyInCombat(Player* player);

    void SnapIntoMeleeRange(Creature* attacker, Unit* victim)
    {
        float x, y, z;
        victim->GetContactPoint(attacker, x, y, z);
        attacker->NearTeleportTo(x, y, z, attacker->GetAbsoluteAngle(victim));

#ifdef LEGION_SCENARIO_DEBUG_LOG
        SC_LOG_INFO("scripts.legion_scenario",
            "[escort {}] stuck out of melee range of {} for 3s+ (pathing issue on map 833?) - snapped into range",
            attacker->GetEntry(), victim->GetEntry());
#endif
    }

    Creature* EngageNearbyEnemy(Creature* attacker, float range)
    {
        std::list<Creature*> nearby;
        attacker->GetCreatureListWithEntryInGrid(nearby, 0, range);

        Creature* target = nullptr;
        float bestDist = range;

        for (Creature* other : nearby)
        {
            if (other == attacker || !other->IsAlive())
                continue;

            if (!LegionScenario::IsSameScenarioInstance(attacker, other))
                continue;

            if (LegionScenario::GetSide(other->GetEntry()) != LegionScenario::SIDE_ENEMY)
                continue;

            float dist = attacker->GetDistance(other);
            if (dist < bestDist)
            {
                bestDist = dist;
                target = other;
            }
        }

#ifdef LEGION_SCENARIO_DEBUG_LOG
        SC_LOG_INFO("scripts.legion_scenario",
            "[escort {}] EngageNearbyEnemy: {} scenario creatures in {:.0f}y, target={}",
            attacker->GetEntry(), nearby.size(), range, target ? target->GetEntry() : 0);
#endif

        if (target && attacker->AI())
        {
            attacker->AI()->AttackStart(target);
        }

        return target;
    }

    uint32 WakeNearbyAllies(Creature* leader, float range)
    {
        std::list<Creature*> nearby;
        leader->GetCreatureListWithEntryInGrid(nearby, 0, range);

        uint32 allyCount = 0;
        for (Creature* other : nearby)
        {
            if (other == leader || !other->IsAlive())
                continue;
            if (!LegionScenario::IsSameScenarioInstance(leader, other))
                continue;
            if (LegionScenario::GetSide(other->GetEntry()) != LegionScenario::SIDE_ALLIED)
                continue;
            ++allyCount;
            EngageNearbyEnemy(other, 25.0f);
        }
        return allyCount;
    }

    bool IsFrenchClient(Player* player)
    {
        return player && player->GetSession() && player->GetSession()->GetSessionDbcLocale() == LOCALE_frFR;
    }

    std::string FormatRestTimeLeft(uint32 secondsLeft, bool frenchLocale)
    {
        uint32 minutes = secondsLeft / 60;
        uint32 seconds = secondsLeft % 60;

        std::ostringstream oss;
        if (frenchLocale)
        {
            oss << "La troupe se repose encore ";
            if (minutes > 0)
            {
                oss << minutes << (minutes > 1 ? " minutes" : " minute");
                if (seconds > 0)
                    oss << " " << seconds << (seconds > 1 ? " secondes" : " seconde");
            }
            else
                oss << seconds << (seconds > 1 ? " secondes" : " seconde");
            oss << " avant de reprendre la marche.";
        }
        else
        {
            oss << "The company still needs to rest for ";
            if (minutes > 0)
            {
                oss << minutes << (minutes > 1 ? " minutes" : " minute");
                if (seconds > 0)
                    oss << " " << seconds << (seconds > 1 ? " seconds" : " second");
            }
            else
                oss << seconds << (seconds > 1 ? " seconds" : " second");
            oss << " before resuming the march.";
        }
        return oss.str();
    }

    void SendRestCountdown(Player* owner, uint32 secondsLeft)
    {
        if (!owner)
            return;

        std::string text = FormatRestTimeLeft(secondsLeft, IsFrenchClient(owner));
        ChatHandler(owner->GetSession()).PSendSysMessage("|cff1eff00[Le Rivage Brise]|r %s", text.c_str());
    }

    void SendRestOver(Player* owner)
    {
        if (!owner)
            return;

        char const* text = IsFrenchClient(owner)
            ? "La troupe est reposee, c'est repartis pour la marche !"
            : "The company is rested - back on the march!";
        ChatHandler(owner->GetSession()).PSendSysMessage("|cff1eff00[Le Rivage Brise]|r %s", text);
    }
}

class npc_legion_escort : public CreatureScript
{
public:
    npc_legion_escort() : CreatureScript("npc_legion_escort") { }

    struct npc_legion_escortAI : public ScriptedAI
    {
        npc_legion_escortAI(Creature* creature) : ScriptedAI(creature),
            isLeader(IsEscortLeaderEntry(creature->GetEntry())),
            isIllidari(LegionScenario::IsIllidariDemonHunter(creature->GetEntry())),
            pathIndex(0),
            waitingForGossip(false),
            followAngle(0.0f),
            questsFired(false),
            gateType(GATE_NONE),
            meleeStuckTimer(0),
            followRefreshTimer(0),
            followRefreshCount(0),
            resting(false),
            restTimer(0),
            restAnnounceTimer(0),
            restSecondsLeft(0),
            finalBossEngaged(false),
            finalBossSawAliveViaRescan(false),
            finalBossWatchTimer(0),
            finalBossWaitElapsedMs(0),
            ambientScanTimer(0),
            partyPaused(false),
            partyPauseWatchdog(0),
            stallCheckTimer(0)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        Player* GetOwningPlayer() const
        {
            if (TempSummon* summon = me->ToTempSummon())
                return ObjectAccessor::GetPlayer(*me, summon->GetSummonerGUID());
            return nullptr;
        }

        void SetFollowTarget(ObjectGuid guid, float angle)
        {
            leaderGuid = guid;
            followAngle = angle;
        }

        void StartGuiding()
        {
            if (!isLeader)
                return;

            pathIndex = 0;
            waitingForGossip = false;
            me->SetWalk(false);
            Talk(SAY_ESCORT_START);
            StepForward();
        }

        void StepForward()
        {
            if (pathIndex >= LegionEscort::PATH_SIZE)
            {
                if (isLeader && !questsFired)
                {
                    questsFired = true;
                    if (Player* owner = GetOwningPlayer())
                    {
                        owner->AreaExploredOrEventHappens(LegionEscort::MAIN_QUEST_ID);
                        owner->AreaExploredOrEventHappens(LegionEscort::WEEKLY_QUEST_ID);
                        DespawnParty(owner);
                    }
                }
                return;
            }

            LegionEscort::Point const& pt = LegionEscort::PATH[pathIndex];
            me->GetMotionMaster()->MovePoint(pathIndex, pt.x, pt.y, pt.z);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (!isLeader || type != POINT_MOTION_TYPE || id != pathIndex)
                return;

            HandleReachedPoint(pathIndex + 1);
        }

        void HandleReachedPoint(uint32 reachedPoint)
        {
            if (reachedPoint == LegionEscort::COMBAT_GATE_POINT)
            {
                Talk(SAY_ESCORT_COMBAT_READY);
                gateType = GATE_GENERIC;
                waitingForGossip = true;
                return;
            }

            if (reachedPoint == LegionEscort::COMBAT_GATE_BOSS1_POINT)
            {
                Talk(SAY_ESCORT_BOSS1_READY);
                gateType = GATE_BOSS1;
                waitingForGossip = true;
                return;
            }

            if (reachedPoint == LegionEscort::COMBAT_GATE_BOSS2_POINT)
            {
                Talk(SAY_ESCORT_BOSS2_READY);
                gateType = GATE_BOSS2;
                waitingForGossip = true;
                return;
            }

            for (EscortDialogueLine const& line : ESCORT_DIALOGUE)
            {
                if (line.afterPoint == reachedPoint)
                {
                    Talk(line.textId);
                    break;
                }
            }

            if (reachedPoint == LegionEscort::REST_PAUSE_POINT)
            {
                resting = true;
                restTimer = LegionEscort::REST_PAUSE_DURATION_MS;
                restAnnounceTimer = 0;
                restSecondsLeft = LegionEscort::REST_PAUSE_DURATION_MS / 1000;

                SendRestCountdown(GetOwningPlayer(), restSecondsLeft);

#ifdef LEGION_SCENARIO_DEBUG_LOG
                SC_LOG_INFO("scripts.legion_scenario",
                    "[escort {}] resting at checkpoint {} for {} ms before continuing",
                    me->GetEntry(), reachedPoint, LegionEscort::REST_PAUSE_DURATION_MS);
#endif
                return;
            }

            ++pathIndex;
            StepForward();
        }

        bool OnGossipHello(Player* player)
        {
            if (!isLeader)
            {
                if (!leaderGuid.IsEmpty())
                    if (Creature* leader = ObjectAccessor::GetCreature(*me, leaderGuid))
                        if (CreatureAI* leaderAI = leader->AI())
                            return leaderAI->OnGossipHello(player);
                return false;
            }

            if (!waitingForGossip)
                return false;

            char const* text = "Lance l'assaut contre les forces de la Legion.";
            if (gateType == GATE_BOSS1)
                text = "Lance l'assaut contre le premier Boss de la Legion.";
            else if (gateType == GATE_BOSS2)
                text = "Lance l'assaut final contre le Dernier Boss de la Legion.";

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, text,
                GOSSIP_SENDER_MAIN, GOSSIP_ACTION_LAUNCH_COMBAT);

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* /*player*/, uint32 /*menuId*/, uint32 /*gossipListId*/)
        {
            LaunchCombat();
            return true;
        }

        void LaunchCombat()
        {
            Creature* primaryTarget = EngageNearbyEnemy(me, 60.0f);
            uint32 allyCount = WakeNearbyAllies(me, 15.0f);

#ifdef LEGION_SCENARIO_DEBUG_LOG
            SC_LOG_INFO("scripts.legion_scenario",
                "[escort {}] LaunchCombat: {} allied escorts found within 15y to wake up (gate={})",
                me->GetEntry(), allyCount, static_cast<uint32>(gateType));

            if (Player* owner = GetOwningPlayer())
            {
                ChatHandler(owner->GetSession()).PSendSysMessage(
                    "|cffff6060[LegionDebug]|r LaunchCombat called (gate=%u): primaryTarget=%u, %u allies woken",
                    static_cast<uint32>(gateType), primaryTarget ? primaryTarget->GetEntry() : 0, allyCount);
            }
#endif

            waitingForGossip = false;

            if (gateType == GATE_BOSS2)
            {
                finalBossEngaged = true;
                finalBossGuid = ObjectGuid::Empty;

                std::list<Creature*> bossSearch;
                me->GetCreatureListWithEntryInGrid(bossSearch, LegionScenario::BOSS_ENTRIES[2], 150.0f);
                for (Creature* c : bossSearch)
                {
                    if (c->IsAlive() && LegionScenario::IsSameScenarioInstance(me, c))
                    {
                        finalBossGuid = c->GetGUID();
                        break;
                    }
                }

                finalBossSawAliveViaRescan = !finalBossGuid.IsEmpty();
                finalBossWatchTimer = 0;
                finalBossWaitElapsedMs = 0;

#ifdef LEGION_SCENARIO_DEBUG_LOG
                SC_LOG_INFO("scripts.legion_scenario",
                    "[escort {}] LaunchCombat: final boss engaged (captured guid empty={}), waiting for it to die",
                    me->GetEntry(), finalBossGuid.IsEmpty());
#endif
                return;
            }

            partyPaused = true;
            partyPauseWatchdog = 0;
            ++pathIndex;

#ifdef LEGION_SCENARIO_DEBUG_LOG
            SC_LOG_INFO("scripts.legion_scenario",
                "[escort {}] LaunchCombat: gate combat started (gate={}), pathIndex advanced to {} but walk stays paused until combat clears",
                me->GetEntry(), static_cast<uint32>(gateType), pathIndex);

            if (Player* owner = GetOwningPlayer())
            {
                ChatHandler(owner->GetSession()).PSendSysMessage(
                    "|cffff6060[LegionDebug]|r LaunchCombat: pathIndex advanced to %u, march paused until combat clears",
                    pathIndex);
            }
#endif
        }

        void UpdateAI(uint32 diff) override
        {
            if (Unit* victim = me->GetVictim())
            {
                if (victim->IsAlive() && me->IsAlive())
                {
                    if (!me->IsWithinMeleeRange(victim))
                    {
                        meleeStuckTimer += diff;
                        if (meleeStuckTimer >= 3000)
                        {
                            meleeStuckTimer = 0;
                            SnapIntoMeleeRange(me, victim);
                        }
                    }
                    else
                        meleeStuckTimer = 0;

                    DoMeleeAttackIfReady();

                    if (isIllidari)
                        illidariKit.Update(me, victim, diff);

                    return;
                }

                me->AttackStop();
                me->GetMotionMaster()->Clear();
            }
            meleeStuckTimer = 0;

            if (!isLeader && !leaderGuid.IsEmpty())
            {
                if (Unit* leader = ObjectAccessor::GetUnit(*me, leaderGuid))
                {
                    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                        me->GetMotionMaster()->MoveFollow(leader, 3.0f, followAngle);

                    if (me->GetDistance(leader) > 5.0f)
                    {
                        followRefreshTimer += diff;
                        if (followRefreshTimer >= 4000)
                        {
                            followRefreshTimer = 0;
                            ++followRefreshCount;

                            if (followRefreshCount >= 3)
                            {
                                followRefreshCount = 0;

                                float x, y, z;
                                leader->GetNearPoint(me, x, y, z, 3.0f, leader->GetOrientation() + followAngle);
                                me->NearTeleportTo(x, y, z, me->GetAbsoluteAngle(leader));
                                me->GetMotionMaster()->MoveFollow(leader, 3.0f, followAngle);

#ifdef LEGION_SCENARIO_DEBUG_LOG
                                SC_LOG_INFO("scripts.legion_scenario",
                                    "[escort {}] follow watchdog: still {:.0f}y from leader after 3 forced recomputes (~12s) - snapped into formation",
                                    me->GetEntry(), me->GetDistance(leader));
#endif
                            }
                            else
                            {
                                me->GetMotionMaster()->Clear();
                                me->GetMotionMaster()->MoveFollow(leader, 3.0f, followAngle);

#ifdef LEGION_SCENARIO_DEBUG_LOG
                                SC_LOG_INFO("scripts.legion_scenario",
                                    "[escort {}] follow watchdog: {:.0f}y from leader after 4s - forcing a fresh path recompute (attempt {})",
                                    me->GetEntry(), me->GetDistance(leader), followRefreshCount);
#endif
                            }
                        }
                    }
                    else
                    {
                        followRefreshTimer = 0;
                        followRefreshCount = 0;
                    }
                }
            }

            if (!isLeader)
                return;

            if (resting)
            {
                restAnnounceTimer += diff;
                if (restAnnounceTimer >= 30000 && restSecondsLeft > 30)
                {
                    restAnnounceTimer -= 30000;
                    restSecondsLeft -= 30;
                    SendRestCountdown(GetOwningPlayer(), restSecondsLeft);
                }

                if (restTimer <= diff)
                {
                    resting = false;
                    restTimer = 0;
                    restAnnounceTimer = 0;
                    restSecondsLeft = 0;
                    ++pathIndex;

                    SendRestOver(GetOwningPlayer());
                    StepForward();

#ifdef LEGION_SCENARIO_DEBUG_LOG
                    SC_LOG_INFO("scripts.legion_scenario",
                        "[escort {}] rest pause over, resuming march at pathIndex {}",
                        me->GetEntry(), pathIndex);
#endif
                }
                else
                {
                    restTimer -= diff;
                }
                return;
            }

            if (finalBossEngaged && pathIndex < LegionEscort::PATH_SIZE)
            {
                finalBossWatchTimer += diff;
                finalBossWaitElapsedMs += diff;

                if (finalBossWatchTimer >= 1000)
                {
                    finalBossWatchTimer = 0;
                    bool confirmedDead = false;

                    if (!finalBossGuid.IsEmpty())
                    {
                        Creature* boss = ObjectAccessor::GetCreature(*me, finalBossGuid);
                        if (!boss || !boss->IsAlive())
                            confirmedDead = true;
                    }

                    if (!confirmedDead)
                    {
                        std::list<Creature*> nearby;
                        me->GetCreatureListWithEntryInGrid(nearby, LegionScenario::BOSS_ENTRIES[2], 150.0f);
                        bool aliveNow = false;
                        for (Creature* c : nearby)
                        {
                            if (c->IsAlive() && LegionScenario::IsSameScenarioInstance(me, c))
                            {
                                aliveNow = true;
                                break;
                            }
                        }
                        if (aliveNow)
                            finalBossSawAliveViaRescan = true;
                        else if (finalBossSawAliveViaRescan)
                            confirmedDead = true;
                    }

                    if (!confirmedDead && finalBossWaitElapsedMs >= 20u * 60u * 1000u)
                    {
                        confirmedDead = true;
#ifdef LEGION_SCENARIO_DEBUG_LOG
                        SC_LOG_INFO("scripts.legion_scenario",
                            "[escort {}] final boss watchdog: 20-minute safety timeout reached, forcing completion",
                            me->GetEntry());
#endif
                    }

#ifdef LEGION_SCENARIO_DEBUG_LOG
                    SC_LOG_INFO("scripts.legion_scenario",
                        "[escort {}] final boss watchdog tick: confirmedDead={}, sawAliveViaRescan={}",
                        me->GetEntry(), confirmedDead, finalBossSawAliveViaRescan);
#endif

                    if (confirmedDead)
                    {
                        finalBossEngaged = false;
                        Talk(SAY_ESCORT_END);
                        pathIndex = LegionEscort::PATH_SIZE;
                        StepForward();
                        return;
                    }
                }
            }

            if (!waitingForGossip && !finalBossEngaged && pathIndex < LegionEscort::PATH_SIZE)
            {
                ambientScanTimer += diff;
                if (ambientScanTimer >= 1000)
                {
                    ambientScanTimer = 0;

                    if (EngageNearbyEnemy(me, 25.0f))
                        WakeNearbyAllies(me, 15.0f);

                    Player* owner = GetOwningPlayer();
                    bool anyCombat = owner && IsPartyInCombat(owner);

                    if (anyCombat)
                    {
                        partyPaused = true;
                        partyPauseWatchdog = 0;
                    }
                    else if (partyPaused)
                    {
                        partyPaused = false;
                        partyPauseWatchdog = 0;
#ifdef LEGION_SCENARIO_DEBUG_LOG
                        SC_LOG_INFO("scripts.legion_scenario",
                            "[escort {}] ambient scan: combat cleared, resuming march at pathIndex {}",
                            me->GetEntry(), pathIndex);
                        if (owner)
                            ChatHandler(owner->GetSession()).PSendSysMessage(
                                "|cffff6060[LegionDebug]|r ambient scan: combat cleared, resuming march at pathIndex %u",
                                pathIndex);
#endif
                        StepForward();
                    }

                    if (partyPaused)
                    {
                        partyPauseWatchdog += 1000;
                        if (partyPauseWatchdog >= 8000)
                        {
                            partyPaused = false;
                            partyPauseWatchdog = 0;
#ifdef LEGION_SCENARIO_DEBUG_LOG
                            SC_LOG_INFO("scripts.legion_scenario",
                                "[escort {}] party-pause watchdog: stuck paused 8s+ with no confirmed combat, forcing resume at pathIndex {}",
                                me->GetEntry(), pathIndex);
                            if (owner)
                                ChatHandler(owner->GetSession()).PSendSysMessage(
                                    "|cffff6060[LegionDebug]|r party-pause watchdog: forcing resume at pathIndex %u (no real combat detected after 8s)",
                                    pathIndex);
#endif
                            StepForward();
                        }
                    }
                }
            }

            if (!waitingForGossip && !finalBossEngaged && pathIndex < LegionEscort::PATH_SIZE)
            {
                stallCheckTimer += diff;
                if (stallCheckTimer >= 1000)
                {
                    stallCheckTimer = 0;

                    LegionEscort::Point const& target = LegionEscort::PATH[pathIndex];
                    if (me->GetDistance(target.x, target.y, target.z) <= 5.0f)
                    {
#ifdef LEGION_SCENARIO_DEBUG_LOG
                        SC_LOG_INFO("scripts.legion_scenario",
                            "[escort {}] stall watchdog: within 5y of checkpoint {} but MovementInform never fired - forcing arrival",
                            me->GetEntry(), pathIndex + 1);
#endif
                        HandleReachedPoint(pathIndex + 1);
                    }
                }
            }
        }

        bool isLeader;
        bool isIllidari;
        uint32 pathIndex;
        bool waitingForGossip;
        ObjectGuid leaderGuid;
        float followAngle;
        bool questsFired;
        LegionEscortGateType gateType;
        uint32 meleeStuckTimer;
        uint32 followRefreshTimer;
        uint8 followRefreshCount;
        bool resting;
        uint32 restTimer;
        uint32 restAnnounceTimer;
        uint32 restSecondsLeft;
        LegionScenario::IllidariCombatKit illidariKit;

        bool finalBossEngaged;
        ObjectGuid finalBossGuid;
        bool finalBossSawAliveViaRescan;
        uint32 finalBossWatchTimer;
        uint32 finalBossWaitElapsedMs;

        uint32 ambientScanTimer;
        bool partyPaused;
        uint32 partyPauseWatchdog;

        uint32 stallCheckTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_legion_escortAI(creature);
    }
};

namespace
{
    struct EscortParty
    {
        ObjectGuid leader;
        ObjectGuid members[4];
        ObjectGuid questGiver;
        std::vector<ObjectGuid> rosterGuids;
        uint32 scenarioPhaseMask = 0;
        uint32 previousPhaseMask = 0;
    };

    std::unordered_map<uint64, EscortParty> g_parties;

    constexpr uint32 SCENARIO_PHASE_POOL_SIZE = 30;
    bool g_phaseSlotInUse[SCENARIO_PHASE_POOL_SIZE] = {};

    uint32 AllocateScenarioPhase()
    {
        for (uint32 i = 0; i < SCENARIO_PHASE_POOL_SIZE; ++i)
        {
            if (!g_phaseSlotInUse[i])
            {
                g_phaseSlotInUse[i] = true;
                return 1u << (i + 1);
            }
        }
        return 0;
    }

    void ReleaseScenarioPhase(uint32 mask)
    {
        if (!mask)
            return;

        for (uint32 i = 0; i < SCENARIO_PHASE_POOL_SIZE; ++i)
        {
            if (mask == (1u << (i + 1)))
            {
                g_phaseSlotInUse[i] = false;
                return;
            }
        }
    }

    void DespawnParty(Player* player)
    {
        if (!player)
            return;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto itr = g_parties.find(pguid);
        if (itr == g_parties.end())
            return;

        if (Creature* leader = ObjectAccessor::GetCreature(*player, itr->second.leader))
            leader->DespawnOrUnsummon();

        for (ObjectGuid const& guid : itr->second.members)
            if (Creature* member = ObjectAccessor::GetCreature(*player, guid))
                member->DespawnOrUnsummon();

        if (Creature* questGiver = ObjectAccessor::GetCreature(*player, itr->second.questGiver))
            questGiver->DespawnOrUnsummon();

        for (ObjectGuid const& guid : itr->second.rosterGuids)
            if (Creature* roster = ObjectAccessor::GetCreature(*player, guid))
                roster->DespawnOrUnsummon();

        if (itr->second.scenarioPhaseMask)
        {
            player->SetPhaseMask(itr->second.previousPhaseMask, true);
            ReleaseScenarioPhase(itr->second.scenarioPhaseMask);

#ifdef LEGION_SCENARIO_DEBUG_LOG
            SC_LOG_INFO("scripts.legion_scenario",
                "[escort] DespawnParty: player {} back to phase {} (left private phase {})",
                player->GetName(), itr->second.previousPhaseMask, itr->second.scenarioPhaseMask);
#endif
        }

        g_parties.erase(itr);
    }

    bool IsPartyInCombat(Player* player)
    {
        if (!player)
            return false;

        uint64 pguid = player->GetGUID().GetRawValue();
        auto itr = g_parties.find(pguid);
        if (itr == g_parties.end())
            return false;

        if (Creature* leader = ObjectAccessor::GetCreature(*player, itr->second.leader))
            if (leader->GetVictim())
                return true;

        for (ObjectGuid const& guid : itr->second.members)
            if (Creature* member = ObjectAccessor::GetCreature(*player, guid))
                if (member->GetVictim())
                    return true;

        if (Creature* questGiver = ObjectAccessor::GetCreature(*player, itr->second.questGiver))
            if (questGiver->GetVictim())
                return true;

        return false;
    }

    void SpawnParty(Player* player)
    {
        uint64 pguid = player->GetGUID().GetRawValue();
        if (g_parties.find(pguid) != g_parties.end())
            return;

        uint32 previousPhaseMask = player->GetPhaseMask();
        uint32 scenarioPhaseMask = AllocateScenarioPhase();
        if (scenarioPhaseMask)
            player->SetPhaseMask(scenarioPhaseMask, true);
#ifdef LEGION_SCENARIO_DEBUG_LOG
        else
            SC_LOG_INFO("scripts.legion_scenario",
                "[escort] SpawnParty: private phase pool exhausted for player {} - they will stay visible to other players",
                player->GetName());
#endif

        bool isHorde = (player->GetTeamId() == TEAM_HORDE);
        uint32 leaderEntry = isHorde ? LegionEscort::HORDE_LEADER : LegionEscort::ALLIANCE_LEADER;
        uint32 const* memberEntries = isHorde ? LegionEscort::HORDE_MEMBERS : LegionEscort::ALLIANCE_MEMBERS;
        uint32 questGiverEntry = isHorde ? LegionEscort::HORDE_QUESTGIVER : LegionEscort::ALLIANCE_QUESTGIVER;

        LegionEscort::Point const& leaderPoint = LegionEscort::SPAWN_POINTS[0];
        TempSummon* leader = player->SummonCreature(leaderEntry,
            leaderPoint.x, leaderPoint.y, leaderPoint.z, leaderPoint.o,
            TEMPSUMMON_MANUAL_DESPAWN, 0ms, true);
        if (!leader)
        {
            if (scenarioPhaseMask)
            {
                player->SetPhaseMask(previousPhaseMask, true);
                ReleaseScenarioPhase(scenarioPhaseMask);
            }
            return;
        }

        EscortParty party;
        party.leader = leader->GetGUID();
        party.previousPhaseMask = previousPhaseMask;
        party.scenarioPhaseMask = scenarioPhaseMask;

        static float const followAngles[4] = { 0.0f, 1.57f, 3.14f, 4.71f };

        for (uint8 i = 0; i < LegionEscort::MEMBER_COUNT; ++i)
        {
            LegionEscort::Point const& sp = LegionEscort::SPAWN_POINTS[i + 1];
            if (TempSummon* member = player->SummonCreature(memberEntries[i],
                    sp.x, sp.y, sp.z, sp.o, TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
            {
                member->GetMotionMaster()->MoveFollow(leader, 3.0f, followAngles[i]);
                if (npc_legion_escort::npc_legion_escortAI* memberAI =
                        ENSURE_AI(npc_legion_escort::npc_legion_escortAI, member->AI()))
                    memberAI->SetFollowTarget(leader->GetGUID(), followAngles[i]);
                party.members[i] = member->GetGUID();
            }
        }

        LegionEscort::Point const& qp = LegionEscort::QUESTGIVER_SPAWN_POINT;
        if (TempSummon* questGiver = player->SummonCreature(questGiverEntry,
                qp.x, qp.y, qp.z, qp.o, TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
        {
            questGiver->GetMotionMaster()->MoveFollow(leader, 4.0f, 2.356f);
            if (npc_legion_escort::npc_legion_escortAI* questGiverAI =
                    ENSURE_AI(npc_legion_escort::npc_legion_escortAI, questGiver->AI()))
                questGiverAI->SetFollowTarget(leader->GetGUID(), 2.356f);
            party.questGiver = questGiver->GetGUID();
        }

        party.rosterGuids.reserve(LegionScenario::ENEMY_ROSTER_SPAWNS_COUNT + LegionScenario::ALLIED_AMBIENT_SPAWNS_COUNT);

        for (uint32 i = 0; i < LegionScenario::ENEMY_ROSTER_SPAWNS_COUNT; ++i)
        {
            LegionScenario::RosterSpawn const& s = LegionScenario::ENEMY_ROSTER_SPAWNS[i];
            if (TempSummon* summon = player->SummonCreature(s.entry, s.x, s.y, s.z, s.o,
                    TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
                party.rosterGuids.push_back(summon->GetGUID());
        }

        for (uint32 i = 0; i < LegionScenario::ALLIED_AMBIENT_SPAWNS_COUNT; ++i)
        {
            LegionScenario::RosterSpawn const& s = LegionScenario::ALLIED_AMBIENT_SPAWNS[i];
            if (TempSummon* summon = player->SummonCreature(s.entry, s.x, s.y, s.z, s.o,
                    TEMPSUMMON_MANUAL_DESPAWN, 0ms, true))
                party.rosterGuids.push_back(summon->GetGUID());
        }

#ifdef LEGION_SCENARIO_DEBUG_LOG
        SC_LOG_INFO("scripts.legion_scenario",
            "[escort] SpawnParty: full-phase roster spawned {}/{} creatures for player {} in private phase {}",
            party.rosterGuids.size(),
            LegionScenario::ENEMY_ROSTER_SPAWNS_COUNT + LegionScenario::ALLIED_AMBIENT_SPAWNS_COUNT,
            player->GetName(), party.scenarioPhaseMask);
#endif

        g_parties[pguid] = party;

        if (npc_legion_escort::npc_legion_escortAI* ai =
                ENSURE_AI(npc_legion_escort::npc_legion_escortAI, leader->AI()))
            ai->StartGuiding();
    }
}

class legion_escort_world : public WorldScript
{
public:
    legion_escort_world() : WorldScript("legion_escort_world"), _ticker(0) { }

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

            if (player->GetMapId() != LegionEscort::MAP_ID)
            {
                DespawnParty(player);
                continue;
            }

            uint64 pguid = player->GetGUID().GetRawValue();
            if (g_parties.find(pguid) != g_parties.end())
                continue;

            if (player->GetDistance2d(LegionEscort::START_POINT.x, LegionEscort::START_POINT.y)
                    <= LegionEscort::START_TRIGGER_RADIUS)
                SpawnParty(player);
        }
    }

private:
    uint32 _ticker;
};

class legion_escort_player : public PlayerScript
{
public:
    legion_escort_player() : PlayerScript("legion_escort_player") { }

    void OnLogout(Player* player) override
    {
        DespawnParty(player);
    }
};

void AddSC_legion_escort()
{
    new npc_legion_escort();
    new legion_escort_world();
    new legion_escort_player();
}
