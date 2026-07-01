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
#include "CellImpl.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Log.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include "World.h"

 // class npc_injured_recruit_horde : public CreatureScript
 // {
 // public:
 //     npc_injured_recruit_horde() : CreatureScript("npc_injured_recruit_horde") { }
 // 
 //     struct npc_injured_recruit_hordeAI : public ScriptedAI
 //     {
 //         npc_injured_recruit_hordeAI(Creature* creature) : ScriptedAI(creature)
 //         {
 //             Initialize();
 //         }
 // 
 //         void Initialize()
 //         {
 //             DoctorGUID.Clear();
 //             Coord = nullptr;
 //         }
 // 
 //         ObjectGuid DoctorGUID;
 //         Position const* Coord;
 // 
 //         void Reset() override
 //         {
 //             Initialize();
 // 
 //             //no select
 //             me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
 // 
 //             //no regen health
 //             me->SetUnitFlag(UNIT_FLAG_IN_COMBAT);
 // 
 //             //to make them lay with face down
 //             me->SetStandState(UNIT_STAND_STATE_DEAD);
 // 
 //             uint32 mobId = me->GetEntry();
 // 
 //             switch (mobId)
 //             {                                                   //lower max health
 //                 case 9940532:
 //                 case 9940533:                                     //Injured Soldier
 //                     me->SetHealth(me->CountPctFromCurHealth(56));
 //                     break;
 //                 case 9940534:
 //                 case 9940535:                                     //Badly injured Soldier
 //                     me->SetHealth(me->CountPctFromCurHealth(53));
 //                     break;
 //                 case 9940536:
 //                 case 9940537:                                     //Critically injured Soldier
 //                     me->SetHealth(me->CountPctFromCurHealth(51));
 //                     break;
 //             }
 //         }
 // 
 //         void JustEngagedWith(Unit* /*who*/) override { }
 // 
 //         void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
 //         {
 //             Player* player = caster->ToPlayer();
 //             if (!player || !me->IsAlive() || spellInfo->Id != 1502036)
 //                 return;
 // 
 //             //make uninteractible
 //             me->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
 // 
 //             //regen health
 //             me->RemoveUnitFlag(UNIT_FLAG_IN_COMBAT);
 // 
 //             //stand up
 //             me->SetStandState(UNIT_STAND_STATE_STAND);
 // 
 //             uint32 mobId = me->GetEntry();
 //             me->SetWalk(false);
 //         }
 // 
 //         void UpdateAI(uint32 /*diff*/) override
 //         {
 //             //lower HP on every world tick makes it a useful counter, not officlone though
 //             if (me->IsAlive() && me->GetHealth() > 16)
 //                 me->ModifyHealth(0);
 // 
 //             if (me->IsAlive() && me->GetHealth() <= 16)
 //             {
 //                 me->RemoveUnitFlag(UNIT_FLAG_IN_COMBAT);
 //                 me->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
 //                 me->setDeathState(JUST_DIED);
 //                 me->SetDynamicFlag(32);
 //             }
 //         }
 //     };
 // 
 //     CreatureAI* GetAI(Creature* creature) const override
 //     {
 //         return new npc_injured_recruit_hordeAI(creature);
 //     }
 // };
 // 
void AddSC_quest_horde_start()
{
	//     new npc_injured_recruit_horde();
}
