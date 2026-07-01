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

#include "Cell.h"
#include "CellImpl.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Language.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "ScriptedCreature.h"
#include "EventMap.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SmartScript.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include "MoveSplineInit.h"
#include "GameEventMgr.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "CombatAI.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "ScriptedEscortAI.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "World.h"

class npc_training_target : public CreatureScript
{
public:
    npc_training_target() : CreatureScript("npc_training_target") { }

    struct npc_training_targetAI : ScriptedAI
    {
        npc_training_targetAI(Creature* creature) : ScriptedAI(creature)
        {
            SetCombatMovement(false);
            entry = creature->GetEntry();
        }

        uint32 entry;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_training_targetAI(creature);
    }
};

enum TushuiEvents
{
    EVENT_ENTER_VEHICLE = 1
};

class npc_tushui_monk : public CreatureScript
{
public:
    npc_tushui_monk() : CreatureScript("npc_tushui_monk") {}

    struct npc_tushui_monkAI : public ScriptedAI
    {
        npc_tushui_monkAI(Creature* creature) : ScriptedAI(creature), _boarded(false) {}

        EventMap _events;
        bool _boarded;

        void Reset() override
        {
            _boarded = false;
            me->SetFaction(2357);

            _events.ScheduleEvent(EVENT_ENTER_VEHICLE, 500ms);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENTER_VEHICLE:
                    {
                        if (me->GetVehicle() || _boarded)
                            return;

                        std::list<Creature*> poleList;
                        GetCreatureListWithEntryInGrid(poleList, me, 54993, 25.0f);

                        if (poleList.empty())
                        {
                            me->DespawnOrUnsummon(1000ms);
                            return;
                        }

                        for (Creature* pole : poleList)
                        {
                            if (Vehicle* vk = pole->GetVehicleKit())
                            {
                                if (vk->GetAvailableSeatCount() > 0)
                                {
                                    me->EnterVehicle(pole);
                                    _boarded = true;
                                    break;
                                }
                            }
                        }

                        if (!_boarded)
                            me->DespawnOrUnsummon(1000ms);

                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _events.Reset();
            me->ExitVehicle();
            me->DespawnOrUnsummon(1000ms);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tushui_monkAI(creature);
    }
};

// Rock Jump - 103069 / 103070 / 103077
class spell_rock_jump: public SpellScriptLoader
{
    public:
        spell_rock_jump() : SpellScriptLoader("spell_rock_jump") { }

        class spell_rock_jump_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rock_jump_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetPositionZ() < 90.0f)
                        caster->GetMotionMaster()->MoveJump(1045.36f, 2848.47f, 91.38f, 10.0f, 10.0f, 0);
                    else if (caster->GetPositionZ() < 92.0f)
                        caster->GetMotionMaster()->MoveJump(1054.42f, 2842.65f, 92.96f, 10.0f, 10.0f, 0);
                    else if (caster->GetPositionZ() < 94.0f)
                        caster->GetMotionMaster()->MoveJump(1063.66f, 2843.49f, 95.50f, 10.0f, 10.0f, 0);
                    else
                    {
                        caster->GetMotionMaster()->MoveJump(1078.42f, 2845.07f, 95.16f, 10.0f, 10.0f, 0);

                        if (caster->ToPlayer())
                            caster->ToPlayer()->KilledMonsterCredit(57476);
                    }
                }
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_rock_jump_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rock_jump_SpellScript();
        }
};

Position rocksPos[4] =
{
    {1102.05f, 2882.11f, 94.32f, 0.11f},
    {1120.01f, 2883.20f, 96.44f, 4.17f},
    {1128.09f, 2859.44f, 97.64f, 2.51f},
    {1111.52f, 2849.84f, 94.84f, 1.94f}
};

class npc_shang_xi_air_balloon : public CreatureScript
{
public:
    npc_shang_xi_air_balloon() : CreatureScript("npc_shang_xi_air_balloon") {}

    struct npc_shang_xi_air_balloonAI : public CreatureAI
    {
        npc_shang_xi_air_balloonAI(Creature* creature) : CreatureAI(creature) {}

        uint64 playerGUID;
        uint32 eventTimer;
        uint32 phase;

        void Reset() override
        {
            playerGUID = 0;
            eventTimer = 250;
            phase = 0;

            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
        }

        void RemoveNpcPassengers()
        {
            for (auto i = 1; i != 3; ++i)
            {
                auto const passenger = me->GetVehicleKit()->GetPassenger(i);
                if (!passenger)
                    continue;

                passenger->ExitVehicle();
                passenger->ToCreature()->DespawnOrUnsummon(1000s);
            }
        }

        void WaypointReached(uint32 /*nodeId*/, uint32 /*pathId*/)
        {
            
        }

        /*
            seat 0 = player
            seat 1 = Ji Firepaw
            seat 2 = Aysa Cloudsinger
        */

        void UpdateAI(uint32 /*diff*/) override
        {
            if (playerGUID == 0)
            {
                RemoveNpcPassengers();
                me->DespawnOrUnsummon();
                return;
            }
        }

        void PassengerTalk(uint32 /*talkId*/, uint32 /*seatId*/)
        {
            
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || passenger->GetTypeId() != TYPEID_PLAYER)
                return;

            if (apply)
            {
                auto const player = passenger->ToPlayer();

                if (auto const firepaw = player->FindNearestCreature(56660, player->GetPositionX()))
                    firepaw->EnterVehicle(me, 1);

                if (auto const aysa = player->FindNearestCreature(56662, player->GetPositionX()))
                    aysa->EnterVehicle(me, 2);

                player->KilledMonsterCredit(56378);
            }
            else
                playerGUID = 0;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shang_xi_air_balloonAI(creature);
    }
};

// Grab Air Balloon - 95247
class spell_grab_air_balloon : public SpellScriptLoader
{
public:
    spell_grab_air_balloon() : SpellScriptLoader("spell_grab_air_balloon") {}

    class spell_grab_air_balloon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_grab_air_balloon_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            PreventHitAura();

            if (Unit* caster = GetCaster())
                if (Creature* balloon = caster->FindNearestCreature(55649, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    caster->EnterVehicle(balloon, 0);
                }
        }

        void Register() override
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_grab_air_balloon_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_grab_air_balloon_SpellScript();
    }
};

class mob_aisa_pre_balon_event : public CreatureScript
{
public:
    mob_aisa_pre_balon_event() : CreatureScript("mob_aisa_pre_balon_event") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 29791)
            sCreatureTextMgr->SendChat(creature, 1);

        creature->CastSpell(player, 95247, true);
        player->CastSpell(player, 95247, true);
        return true;
    }

    struct mob_aisa_pre_balon_eventAI : public ScriptedAI
    {
        mob_aisa_pre_balon_eventAI(Creature* creature) : ScriptedAI(creature) {}

        bool justSpeaking;
        EventMap _events;
        GuidSet m_player_for_event;

        enum events
        {
            EVENT_1 = 1,
            EVENT_2 = 2,
            EVENT_3 = 3,

            NPC_FRIEND = 56663,
        };

        void Reset() override
        {
            justSpeaking = false;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (justSpeaking || who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle(0))
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());

            if (itr != m_player_for_event.end())
                return;

            if (who->ToPlayer()->GetQuestStatus(29790) != QUEST_STATUS_COMPLETE)
                return;

            m_player_for_event.insert(who->GetGUID());
            justSpeaking = true;
            _events.ScheduleEvent(EVENT_1, 10000s);
            sCreatureTextMgr->SendChat(me, 0);

        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                {
                    _events.ScheduleEvent(EVENT_2, 8000s);

                    if (Creature* f = me->FindNearestCreature(NPC_FRIEND, 100.0f, true))
                    {
                        sCreatureTextMgr->SendChat(f, 0);
                        f->SetFacingToObject(me);
                    }
                    break;
                }
                case EVENT_2:
                    sCreatureTextMgr->SendChat(me, 2);
                    justSpeaking = false;
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_aisa_pre_balon_eventAI(creature);
    }
};

void AddSC_zone_the_wandering_isle()
{
    new npc_training_target();
    new npc_tushui_monk();
    new spell_rock_jump();
    new npc_shang_xi_air_balloon();
    new spell_grab_air_balloon();
    new mob_aisa_pre_balon_event();
}
