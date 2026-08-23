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
#include "Player.h"
#include "GameObject.h"

#include <bitset>
#include <unordered_map>

namespace AysaPhasing
{
    constexpr uint32 PHASE_DEFAULT = 1;
    constexpr uint32 PHASE_POOL_SIZE = 30;

    inline std::bitset<32>& Pool()
    {
        static std::bitset<32> pool;
        return pool;
    }

    inline uint32 AllocatePhase()
    {
        std::bitset<32>& pool = Pool();
        for (uint32 i = 1; i <= PHASE_POOL_SIZE; ++i)
        {
            if (!pool.test(i))
            {
                pool.set(i);
                return (1u << i);
            }
        }
        return 0;
    }

    inline void FreePhase(uint32 mask)
    {
        if (!mask)
            return;
        for (uint32 i = 1; i <= PHASE_POOL_SIZE; ++i)
        {
            if (mask == (1u << i))
            {
                Pool().reset(i);
                return;
            }
        }
    }
}

enum AysaConst
{
    QUEST_WAY_OF_THE_TUSHUI = 29414,
    NPC_AYSA                = 54567,
    NPC_AYSA_LAKE_ESCORT    = 56661,
    NPC_MASTER_LI_FEI       = 54856,
    NPC_TROUBLEMAKER        = 59637,
    AREA_CAVE_OF_MEDITATION = 5848,

    NPC_HELPER_ENTRY        = 59650,
    GO_HELPER_ENTRY         = 209363,

    SAY_INTRO        = 0,
    SAY_END          = 1,
    SAY_INTERRUPTED  = 2,
};

enum AysaEvents
{
    EVENT_START      = 1,
    EVENT_SPAWN_MOBS = 2,
    EVENT_PROGRESS   = 3,
    EVENT_END        = 4,
    EVENT_RELEASE    = 5,

    EVENT_JUMP_1     = 10,
    EVENT_JUMP_2     = 11,
    EVENT_JUMP_3     = 12,
    EVENT_WALK_4     = 13,
    EVENT_WALK_5     = 14,
    EVENT_WALK_6     = 15,
};

struct TroublemakerSpawnPoint { float x, y, z, o; };
static const TroublemakerSpawnPoint TROUBLEMAKER_SPAWNS[3] =
{
    { 1157.050f, 3438.750f, 104.973f, 3.0f },
    { 1153.300f, 3440.915f, 104.973f, 3.3f },
    { 1153.300f, 3436.585f, 104.973f, 3.6f },
};

// [ownerKey guid joueur] = { creatureGUID Aysa, phaseMask }
static std::unordered_map<ObjectGuid, std::pair<ObjectGuid, uint32>> EncounterByPlayer;

class npc_aysa_lake_escort : public CreatureScript
{
    public:
        npc_aysa_lake_escort() : CreatureScript("npc_aysa_lake_escort") { }

        enum EscortPoints
        {
            POINT_1 = 1,
            POINT_2 = 2,
            POINT_3 = 3,
        };

        struct npc_aysa_lake_escortAI : public ScriptedAI
        {
            npc_aysa_lake_escortAI(Creature* creature) : ScriptedAI(creature) { }

            void JustAppeared() override
            {
                me->SetReactState(REACT_PASSIVE);
                events.ScheduleEvent(1, 2500ms);
            }

            EventMap events;

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
                    return;

                switch (pointId)
                {
                    case POINT_1:
                        me->GetMotionMaster()->MoveJump(1192.29f, 3478.69f, 108.788f, 0.0f, 10.0f, 20.0f, POINT_2);
                        break;
                    case POINT_2:
                        me->GetMotionMaster()->MoveJump(1197.99f, 3460.63f, 103.04f, 0.0f, 10.0f, 20.0f, POINT_3);
                        break;
                    case POINT_3:
                        me->DespawnOrUnsummon(500ms);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);
                while (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == 1)
                    {
                        Talk(0);
                        me->GetMotionMaster()->MoveJump(1196.72f, 3492.85f, 90.9836f, 0.0f, 10.0f, 20.0f, POINT_1);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_aysa_lake_escortAI(creature);
        }
};

// ============================================================
// NPC 54567 - npc_aysa
// ============================================================
class npc_aysa : public CreatureScript
{
    public:
        npc_aysa() : CreatureScript("npc_aysa") { }

        struct npc_aysaAI : public ScriptedAI
        {
            npc_aysaAI(Creature* creature) : ScriptedAI(creature)
            {
                timer = 0;
                lifeiGUID.Clear();
                ownerGuid.Clear();
                phaseMask = 0;
            }

            EventMap events;

            std::vector<ObjectGuid> playersInvolved;

            ObjectGuid lifeiGUID;
            ObjectGuid ownerGuid;
            uint32 phaseMask;
            uint32 timer;

            void JustAppeared() override
            {
                me->SetReactState(REACT_PASSIVE);

                if (ownerGuid.IsEmpty())
                    me->SetPhaseMask(AysaPhasing::PHASE_DEFAULT, true);
            }

            void OnQuestAccept(Player* player, Quest const* quest) override
            {
                if (quest->GetQuestId() != QUEST_WAY_OF_THE_TUSHUI)
                    return;

                ObjectGuid playerGuid = player->GetGUID();
                auto existing = EncounterByPlayer.find(playerGuid);
                uint32 mask = (existing != EncounterByPlayer.end())
                    ? existing->second.second
                    : AysaPhasing::AllocatePhase();

                EncounterByPlayer[playerGuid] = { me->GetGUID(), mask };

                if (mask)
                {
                    player->SetPhaseMask(mask, true);
                    me->SetPhaseMask(mask, true);
                }

                events.CancelEvent(EVENT_JUMP_1);
                events.CancelEvent(EVENT_JUMP_2);
                events.CancelEvent(EVENT_JUMP_3);
                events.CancelEvent(EVENT_WALK_4);
                events.CancelEvent(EVENT_WALK_5);
                events.CancelEvent(EVENT_WALK_6);
                events.CancelEvent(EVENT_START);

                events.ScheduleEvent(EVENT_JUMP_1, 1000ms);
                events.ScheduleEvent(EVENT_JUMP_2, 3000ms);
                events.ScheduleEvent(EVENT_JUMP_3, 5500ms);
                events.ScheduleEvent(EVENT_WALK_4, 7500ms);
                events.ScheduleEvent(EVENT_WALK_5, 8500ms);
                events.ScheduleEvent(EVENT_WALK_6, 10875ms);
                events.ScheduleEvent(EVENT_START, 17500ms);
            }

            void JustDespawned()
            {
                ReleaseEncounterState();
            }

            void ReleaseEncounterState()
            {
                DespawnAllTroublemakers();

                if (!ownerGuid.IsEmpty())
                {
                    if (Player* owner = ObjectAccessor::GetPlayer(*me, ownerGuid))
                        owner->SetPhaseMask(AysaPhasing::PHASE_DEFAULT, true);
                    EncounterByPlayer.erase(ownerGuid);
                }

                SetHelpersPhase(AysaPhasing::PHASE_DEFAULT);

                if (phaseMask)
                    AysaPhasing::FreePhase(phaseMask);
                phaseMask = 0;
                ownerGuid.Clear();
            }

            void SetHelpersPhase(uint32 mask)
            {
                std::list<Creature*> helpers;
                me->GetCreatureListWithEntryInGrid(helpers, NPC_HELPER_ENTRY, 30.0f);
                for (Creature* helper : helpers)
                    helper->SetPhaseMask(mask, true);

                std::list<GameObject*> gos;
                me->GetGameObjectListWithEntryInGrid(gos, GO_HELPER_ENTRY, 30.0f);
                for (GameObject* go : gos)
                    go->SetPhaseMask(mask, true);
            }

            void DespawnAllTroublemakers()
            {
                std::list<Creature*> mobs;
                me->GetCreatureListWithEntryInGrid(mobs, NPC_TROUBLEMAKER, 80.0f);
                for (Creature* mob : mobs)
                    mob->DespawnOrUnsummon();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
            {
                if (me->IsAlive() && me->HealthBelowPctDamaged(5, damage))
                {
                    damage = 0;
                    me->SetFullHealth();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetStandState(UNIT_STAND_STATE_STAND);

                    DespawnLifei();

                    std::list<Creature*> mobs;
                    me->GetCreatureListWithEntryInGrid(mobs, NPC_TROUBLEMAKER, 50.0f);
                    for (Creature* mob : mobs)
                        if (mob->IsAlive())
                            Unit::Kill(me, mob);

                    Talk(SAY_INTERRUPTED);

                    events.CancelEvent(EVENT_SPAWN_MOBS);
                    events.CancelEvent(EVENT_PROGRESS);
                    events.CancelEvent(EVENT_END);
                    events.ScheduleEvent(EVENT_START, 20000ms);
                }
            }

            Creature* GetOrSpawnLifei()
            {
                if (!lifeiGUID.IsEmpty())
                {
                    if (Creature* lifei = ObjectAccessor::GetCreature(*me, lifeiGUID))
                        if (lifei->IsAlive())
                            return lifei;
                    lifeiGUID.Clear();
                }

                if (Creature* temp = me->SummonCreature(NPC_MASTER_LI_FEI,
                    1130.162231f, 3435.905518f, 105.496597f, 0.0f,
                    TEMPSUMMON_MANUAL_DESPAWN))
                {
                    if (phaseMask)
                        temp->SetPhaseMask(phaseMask, true);
                    temp->GetMotionMaster()->MoveRandom(5.0f);
                    lifeiGUID = temp->GetGUID();
                    return temp;
                }
                return nullptr;
            }

            void DespawnLifei()
            {
                if (!lifeiGUID.IsEmpty())
                {
                    if (Creature* lifei = ObjectAccessor::GetCreature(*me, lifeiGUID))
                        lifei->DespawnOrUnsummon();
                    lifeiGUID.Clear();
                }
            }

            void UpdatePlayerList()
            {
                playersInvolved.clear();

                std::list<Player*> playerList;
                me->GetPlayerListInGrid(playerList, 25.0f);

                for (Player* player : playerList)
                    if (!player->IsGameMaster() && player->GetQuestStatus(QUEST_WAY_OF_THE_TUSHUI) == QUEST_STATUS_INCOMPLETE)
                        playersInvolved.push_back(player->GetGUID());
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_JUMP_1:
                            me->GetMotionMaster()->MoveJump(1196.72f, 3492.85f, 90.9836f, 0.0f, 10.0f, 20.0f);
                            break;
                        case EVENT_JUMP_2:
                            me->GetMotionMaster()->MoveJump(1192.29f, 3478.69f, 108.788f, 0.0f, 10.0f, 20.0f);
                            break;
                        case EVENT_JUMP_3:
                            me->GetMotionMaster()->MoveJump(1197.99f, 3460.63f, 103.04f, 0.0f, 10.0f, 20.0f);
                            break;
                        case EVENT_WALK_4:
                            me->GetMotionMaster()->MovePoint(4, 1192.92f, 3455.66f, 103.082f);
                            break;
                        case EVENT_WALK_5:
                            me->GetMotionMaster()->MovePoint(5, 1179.78f, 3445.48f, 102.417f);
                            break;
                        case EVENT_WALK_6:
                            me->GetMotionMaster()->MovePoint(6, 1137.02f, 3432.98f, 105.536f);
                            break;
                        case EVENT_START:
                        {
                            UpdatePlayerList();
                            if (playersInvolved.empty())
                            {
                                events.ScheduleEvent(EVENT_START, 600ms);
                                break;
                            }

                            for (ObjectGuid const& guid : playersInvolved)
                            {
                                auto it = EncounterByPlayer.find(guid);
                                if (it != EncounterByPlayer.end() && it->second.first == me->GetGUID())
                                {
                                    ownerGuid = guid;
                                    phaseMask = it->second.second;
                                    break;
                                }
                            }

                            if (phaseMask)
                                SetHelpersPhase(phaseMask);

                            me->SetStandState(UNIT_STAND_STATE_SIT);
                            Talk(SAY_INTRO);
                            me->SetReactState(REACT_PASSIVE);
                            timer = 0;

                            events.ScheduleEvent(EVENT_SPAWN_MOBS, 5000ms);
                            events.ScheduleEvent(EVENT_PROGRESS, 1000ms);
                            events.ScheduleEvent(EVENT_END, 90000ms);
                            break;
                        }
                        case EVENT_SPAWN_MOBS:
                        {
                            for (auto const& sp : TROUBLEMAKER_SPAWNS)
                            {
                                if (TempSummon* mob = me->SummonCreature(NPC_TROUBLEMAKER,
                                    sp.x, sp.y, sp.z, sp.o,
                                    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000ms))
                                {
                                    if (phaseMask)
                                        mob->SetPhaseMask(phaseMask, true);

                                    if (mob->AI())
                                        mob->AI()->AttackStart(me);
                                    mob->GetThreatManager().AddThreat(me, 250.0f);
                                    mob->GetMotionMaster()->Clear();
                                    mob->GetMotionMaster()->MoveChase(me);
                                }
                            }
                            events.ScheduleEvent(EVENT_SPAWN_MOBS, 20000ms);
                            break;
                        }
                        case EVENT_PROGRESS:
                        {
                            timer++;

                            static const uint8 lifeiTalkThreshold[7] = { 25, 30, 42, 54, 66, 78, 85 };
                            for (int i = 0; i < 7; ++i)
                            {
                                if (timer == lifeiTalkThreshold[i])
                                {
                                    if (Creature* lifei = GetOrSpawnLifei())
                                    {
                                        if (lifei->AI())
                                            lifei->AI()->Talk(i);

                                        if (i == 6)
                                        {
                                            lifei->DespawnOrUnsummon(500ms);
                                            lifeiGUID.Clear();
                                        }
                                    }
                                    break;
                                }
                            }

                            events.ScheduleEvent(EVENT_PROGRESS, 1000ms);
                            break;
                        }
                        case EVENT_END:
                        {
                            DespawnLifei();

                            me->SetReactState(REACT_PASSIVE);
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            Talk(SAY_END);

                            UpdatePlayerList();
                            for (ObjectGuid const& guid : playersInvolved)
                            {
                                if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    player->KilledMonsterCredit(NPC_MASTER_LI_FEI);
                                }
                            }

                            timer = 0;
                            events.CancelEvent(EVENT_SPAWN_MOBS);
                            events.CancelEvent(EVENT_PROGRESS);

                            events.ScheduleEvent(EVENT_RELEASE, 3000ms);
                            break;
                        }
                        case EVENT_RELEASE:
                        {
                            ReleaseEncounterState();
                            me->SetRespawnDelay(5);
                            me->DespawnOrUnsummon();
                            break;
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_aysaAI(creature);
        }
};

void AddSC_npc_aysa_tushui()
{
    new npc_aysa_lake_escort();
    new npc_aysa();
}
