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
#include "Chat.h"
#include "ObjectAccessor.h"
#include "TemporarySummon.h"
#include "DisciplePhaseManager.h"

namespace
{
    uint32 const NPC_GRUNT_THROG = 166583;
    uint32 const QUEST_DISCIPLE_CHALLENGE_THROG = 59927;

    uint32 const FACTION_HOSTILE = 15;
    uint32 const FACTION_ID_FRIENDLY = 2104;
    uint32 const LOW_HEALTH_PERCENT = 50;
    uint32 const STABLE_HEALTH_PERCENT = 100;

    uint16 const UNIT_FIELD_FLAGS_INDEX = 33;

    uint32 const GOSSIP_MENU_DISCIPLE_CHALLENGE = 1;
    uint32 const GOSSIP_NPC_TEXT_DISCIPLE_CHALLENGE = 1;
    uint32 const GOSSIP_ACTION_START_CHALLENGE = 1;

    uint32 const SURRENDER_DESPAWN_DELAY_MS = 5000;
}

class npc_grunt_throg : public CreatureScript
{
public:
    npc_grunt_throg() : CreatureScript("npc_grunt_throg") { }

    struct npc_grunt_throgAI : public ScriptedAI
    {
        explicit npc_grunt_throgAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            if (_isDuelClone)
                return;

            me->SetFaction(FACTION_ID_FRIENDLY);
            me->SetHealth(me->GetMaxHealth());
            me->RemoveFlag(UNIT_FIELD_FLAGS_INDEX, UNIT_FLAG_NON_ATTACKABLE);
        }

        void InitDuel(ObjectGuid playerGUID, uint32 phaseMask)
        {
            _isDuelClone = true;
            _surrendered = false;
            _playerGUID = playerGUID;
            _phaseMask = phaseMask;
            _despawnTimer = 0;
        }

        bool OnGossipHello(Player* player) override
        {
            if (_isDuelClone)
                return false;

            if (!player->IsActiveQuest(QUEST_DISCIPLE_CHALLENGE_THROG))
            {
                ChatHandler(player->GetSession()).SendSysMessage("You must have the quest to start the challenge.");
                return true;
            }

            InitGossipMenuFor(player, GOSSIP_MENU_DISCIPLE_CHALLENGE);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Start the challenge", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_START_CHALLENGE);
            SendGossipMenuFor(player, GOSSIP_NPC_TEXT_DISCIPLE_CHALLENGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);

            if (action == GOSSIP_ACTION_START_CHALLENGE)
            {
                ChatHandler(player->GetSession()).SendSysMessage("The challenge begins!");
                StartChallenge(player);
            }

            return true;
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
        {
            if (!_isDuelClone || _surrendered)
            {
                damage = 0;
                return;
            }

            if (me->GetFaction() != FACTION_HOSTILE)
                return;

            uint32 const currentHealth = me->GetHealth();
            uint32 const maxHealth = me->GetMaxHealth();
            uint32 const healthAfterDamage = (damage >= currentHealth) ? 0 : (currentHealth - damage);
            uint32 const thresholdHealth = uint32(maxHealth * (float(LOW_HEALTH_PERCENT) / 100.0f));

            if (healthAfterDamage > thresholdHealth)
                return;

            damage = 0;

            uint32 const stableHealth = uint32(maxHealth * (float(STABLE_HEALTH_PERCENT) / 100.0f));
            me->SetHealth(stableHealth);

            _surrendered = true;
            me->SetFlag(UNIT_FIELD_FLAGS_INDEX, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFaction(FACTION_ID_FRIENDLY);
            me->GetThreatManager().ResetAllThreat();
            me->AttackStop();

            if (Player* player = attacker ? attacker->ToPlayer() : nullptr)
            {
                ChatHandler(player->GetSession()).SendSysMessage("Victory! Grunt Throg surrenders.");

                if (player->IsActiveQuest(QUEST_DISCIPLE_CHALLENGE_THROG))
                    player->CompleteQuest(QUEST_DISCIPLE_CHALLENGE_THROG);
            }

            _despawnTimer = SURRENDER_DESPAWN_DELAY_MS;
        }

        void EnterEvadeMode(EvadeReason why) override
        {
            if (!_isDuelClone)
            {
                ScriptedAI::EnterEvadeMode(why);
                return;
            }

            if (me->GetFaction() == FACTION_HOSTILE)
                me->SetFaction(FACTION_ID_FRIENDLY);

            if (!_surrendered)
                EndDuel();
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->SetFaction(FACTION_ID_FRIENDLY);
            if (_isDuelClone)
                EndDuel();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_isDuelClone)
                return;

            if (_surrendered)
            {
                if (_despawnTimer)
                {
                    if (_despawnTimer <= diff)
                    {
                        _despawnTimer = 0;
                        EndDuel();
                    }
                    else
                    {
                        _despawnTimer -= diff;
                    }
                }
                return;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        void StartChallenge(Player* player)
        {
            uint32 const phaseMask = DisciplePhaseManager::Instance().Acquire();
            if (!phaseMask)
            {
                ChatHandler(player->GetSession()).SendSysMessage("The challenge is currently full elsewhere on the server; please try again in a moment.");
                return;
            }

            player->SetPhaseMask(PHASEMASK_NORMAL | phaseMask, true);

            Position const pos(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            TempSummon* clone = me->SummonCreature(me->GetEntry(), pos, TEMPSUMMON_MANUAL_DESPAWN);
            if (!clone)
            {
                player->SetPhaseMask(PHASEMASK_NORMAL, true);
                DisciplePhaseManager::Instance().Release(phaseMask);
                return;
            }

            clone->SetPhaseMask(phaseMask, true);

            npc_grunt_throgAI* cloneAI = dynamic_cast<npc_grunt_throgAI*>(clone->AI());
            if (!cloneAI)
            {
                player->SetPhaseMask(PHASEMASK_NORMAL, true);
                DisciplePhaseManager::Instance().Release(phaseMask);
                clone->DespawnOrUnsummon();
                return;
            }

            cloneAI->InitDuel(player->GetGUID(), phaseMask);

            clone->SetFaction(FACTION_HOSTILE);
            clone->GetThreatManager().AddThreat(player, 100000.0f);
            cloneAI->AttackStart(player);
        }

        void EndDuel()
        {
            if (!_isDuelClone)
                return;

            if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                player->SetPhaseMask(PHASEMASK_NORMAL, true);

            DisciplePhaseManager::Instance().Release(_phaseMask);

            _isDuelClone = false;
            _phaseMask = 0;
            me->DespawnOrUnsummon();
        }

        bool _isDuelClone = false;
        bool _surrendered = false;
        ObjectGuid _playerGUID;
        uint32 _phaseMask = 0;
        uint32 _despawnTimer = 0;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_grunt_throgAI(creature);
    }
};

void AddSC_npc_grunt_throg()
{
    new npc_grunt_throg();
}
