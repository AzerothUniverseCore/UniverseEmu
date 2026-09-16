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

#include "TheBattleBrokenShore.h"
#include "IllidariAbilities.h"
#include "EnemyAbilities.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include <list>

#define LEGION_SCENARIO_DEBUG_LOG

namespace LegionScenario
{
    bool IsInScenarioZone(uint32 mapId, float x, float y)
    {
        if (mapId != MAP_ID)
            return false;

        return x >= ZONE_MIN_X && x <= ZONE_MAX_X
            && y >= ZONE_MIN_Y && y <= ZONE_MAX_Y;
    }

    Side GetSide(uint32 entry)
    {
        for (uint32 e : ENEMY_NAGA)
            if (e == entry) return SIDE_ENEMY;
        for (uint32 e : ENEMY_SHIVAN)
            if (e == entry) return SIDE_ENEMY;
        for (uint32 e : ENEMY_BROKEN)
            if (e == entry) return SIDE_ENEMY;
        for (uint32 e : ENEMY_GENERAL)
            if (e == entry) return SIDE_ENEMY;
        for (uint32 e : ALLIED_HORDE_ILLIDARI)
            if (e == entry) return SIDE_ALLIED;
        for (uint32 e : ALLIED_ALLIANCE_ILLIDARI)
            if (e == entry) return SIDE_ALLIED;
        return SIDE_NONE;
    }

    bool IsBossEntry(uint32 entry)
    {
        for (uint32 e : BOSS_ENTRIES)
            if (e == entry) return true;
        return false;
    }
}

namespace
{
    constexpr uint32 LEGION_PHASE_DURATION = 5 * 60 * 1000; // 5 minutes

    constexpr uint32 LEGION_SEEK_INTERVAL = 2000; // 2 s

    constexpr float LEGION_SEEK_RANGE_ADVANCE = 40.0f;
    constexpr float LEGION_SEEK_RANGE_DEFEND  = 15.0f;

    constexpr float LEGION_LEASH_RANGE = 60.0f;

    constexpr float ARTILLERY_SEEK_RANGE = 900.0f;
    constexpr float ARTILLERY_MAX_RANGE = 900.0f;
    constexpr uint32 ARTILLERY_SCAN_INTERVAL = 2000; // 2 s

    LegionScenario::Side g_advancingSide = LegionScenario::SIDE_ENEMY;

    bool IsAdvancing(LegionScenario::Side side)
    {
        return side == g_advancingSide;
    }

    void SnapIntoMeleeRange(Creature* attacker, Unit* victim)
    {
        float x, y, z;
        victim->GetContactPoint(attacker, x, y, z);
        attacker->NearTeleportTo(x, y, z, attacker->GetAbsoluteAngle(victim));

#ifdef LEGION_SCENARIO_DEBUG_LOG
        SC_LOG_INFO("scripts.legion_scenario",
            "[{}] stuck out of melee range of {} for 3s+ (pathing issue on map 833?) - snapped into range",
            attacker->GetEntry(), victim->GetEntry());
#endif
    }
}

class legion_scenario_phase_world : public WorldScript
{
public:
    legion_scenario_phase_world() : WorldScript("legion_scenario_phase_world"), _ticker(0) { }

    void OnUpdate(uint32 diff) override
    {
        _ticker += diff;
        if (_ticker < LEGION_PHASE_DURATION)
            return;
        _ticker = 0;

        g_advancingSide = (g_advancingSide == LegionScenario::SIDE_ENEMY)
            ? LegionScenario::SIDE_ALLIED
            : LegionScenario::SIDE_ENEMY;

        SC_LOG_INFO("scripts.legion_scenario", "7th Legion Scenario: {} side is now on the advance.",
            g_advancingSide == LegionScenario::SIDE_ENEMY ? "Enemy" : "Allied");
    }

private:
    uint32 _ticker;
};

struct npc_legion_scenario_combatantAI : public ScriptedAI
{
    npc_legion_scenario_combatantAI(Creature* creature) : ScriptedAI(creature),
        _side(LegionScenario::GetSide(creature->GetEntry())),
        _isBoss(LegionScenario::IsBossEntry(creature->GetEntry())),
        _isIllidari(LegionScenario::IsIllidariDemonHunter(creature->GetEntry())),
        _isRangedArtillery(LegionScenario::GetEnemyArchetype(creature->GetEntry()) == LegionScenario::EnemyArchetype::BOSS_INFERNAL),
        _seekTimer(LEGION_SEEK_INTERVAL),
        _meleeStuckTimer(0),
        _artilleryScanTimer(0)
    {
        me->SetReactState(REACT_AGGRESSIVE);
        _enemyKit.Init(creature->GetEntry());

        if (_isRangedArtillery)
            SetCombatMovement(false);
    }

    void Reset() override
    {
        _seekTimer = LEGION_SEEK_INTERVAL;
        _meleeStuckTimer = 0;
        _artilleryScanTimer = 0;
        _illidariKit.Reset();
        _enemyKit.Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        if (_isRangedArtillery)
        {
            UpdateRangedArtillery(diff);
            return;
        }

        if (Unit* victim = me->GetVictim())
        {
            if (!victim->IsAlive() || !me->IsAlive())
            {
                me->AttackStop();
                me->GetMotionMaster()->Clear();
                _meleeStuckTimer = 0;
            }
            else
            {
                if (me->GetDistance(me->GetHomePosition()) > LEGION_LEASH_RANGE)
                {
                    me->AttackStop();
                    EnterEvadeMode();
                    return;
                }

                if (!me->IsWithinMeleeRange(victim))
                {
                    _meleeStuckTimer += diff;
                    if (_meleeStuckTimer >= 3000)
                    {
                        _meleeStuckTimer = 0;
                        SnapIntoMeleeRange(me, victim);
                    }
                }
                else
                    _meleeStuckTimer = 0;

                DoMeleeAttackIfReady();

                if (_isIllidari)
                    _illidariKit.Update(me, victim, diff);
                else if (_enemyKit.IsActive())
                    _enemyKit.Update(me, victim, diff);

                return;
            }
        }

        _meleeStuckTimer = 0;

        if (_isBoss)
            return;

        if (_seekTimer <= diff)
        {
            _seekTimer = LEGION_SEEK_INTERVAL;
            TrySeekEnemy();
        }
        else
        {
            _seekTimer -= diff;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        std::list<Player*> nearbyPlayers;
        me->GetPlayerListInGrid(nearbyPlayers, 100.0f);

        for (Player* player : nearbyPlayers)
            if (player)
                player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
    }

private:
    void UpdateRangedArtillery(uint32 diff)
    {
        Unit* victim = me->GetVictim();

        _artilleryScanTimer += diff;
        if (_artilleryScanTimer >= ARTILLERY_SCAN_INTERVAL)
        {
            _artilleryScanTimer = 0;

            if (victim && (!victim->IsAlive() || me->GetDistance(victim) > ARTILLERY_MAX_RANGE
                    || !me->IsWithinLOSInMap(victim)))
            {
                me->AttackStop();
                victim = nullptr;
            }

            if (!victim)
            {
                std::list<Player*> nearbyPlayers;
                me->GetPlayerListInGrid(nearbyPlayers, ARTILLERY_SEEK_RANGE);

                Player* target = nullptr;
                float bestDist = ARTILLERY_SEEK_RANGE;

                for (Player* player : nearbyPlayers)
                {
                    if (!player || !player->IsAlive() || player->IsGameMaster())
                        continue;
                    if (!me->IsWithinLOSInMap(player))
                        continue;

                    float dist = me->GetDistance(player);
                    if (dist < bestDist)
                    {
                        bestDist = dist;
                        target = player;
                    }
                }

                if (target)
                {
                    AttackStartNoMove(target);
                    victim = target;

#ifdef LEGION_SCENARIO_DEBUG_LOG
                    SC_LOG_INFO("scripts.legion_scenario",
                        "[{}] artillery: acquired player target '{}' at {:.0f}y (stationary)",
                        me->GetEntry(), target->GetName(), bestDist);
#endif
                }
            }
        }

        if (victim && _enemyKit.IsActive())
            _enemyKit.Update(me, victim, diff);
    }

    void TrySeekEnemy()
    {
        float range = IsAdvancing(_side) ? LEGION_SEEK_RANGE_ADVANCE : LEGION_SEEK_RANGE_DEFEND;

        std::list<Creature*> nearby;
        me->GetCreatureListWithEntryInGrid(nearby, 0, range);

        Creature* target = nullptr;
        float bestDist = range;

        for (Creature* other : nearby)
        {
            if (other == me || !other->IsAlive())
                continue;

            LegionScenario::Side otherSide = LegionScenario::GetSide(other->GetEntry());
            if (otherSide == LegionScenario::SIDE_NONE || otherSide == _side)
                continue;

            float dist = me->GetDistance(other);
            if (dist < bestDist)
            {
                bestDist = dist;
                target = other;
            }
        }

#ifdef LEGION_SCENARIO_DEBUG_LOG
        SC_LOG_INFO("scripts.legion_scenario",
            "[{}] seek: {} scenario creatures in {:.0f}y, target={}",
            me->GetEntry(), nearby.size(), range, target ? target->GetEntry() : 0);
#endif

        if (target)
        {
            AttackStart(target);

#ifdef LEGION_SCENARIO_DEBUG_LOG
            SC_LOG_INFO("scripts.legion_scenario",
                "[{}] AttackStart({}) -> GetVictim()={}, IsInCombat()={}",
                me->GetEntry(), target->GetEntry(),
                me->GetVictim() ? me->GetVictim()->GetEntry() : 0, me->IsInCombat());
#endif
        }
    }

    LegionScenario::Side _side;
    bool _isBoss;
    bool _isIllidari;
    bool _isRangedArtillery;
    uint32 _seekTimer;
    uint32 _meleeStuckTimer;
    uint32 _artilleryScanTimer;
    LegionScenario::IllidariCombatKit _illidariKit;
    LegionScenario::EnemyCombatKit _enemyKit;
};

class npc_legion_scenario_combatant : public CreatureScript
{
public:
    npc_legion_scenario_combatant() : CreatureScript("npc_legion_scenario_combatant") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_legion_scenario_combatantAI(creature);
    }
};

void AddSC_legion_scenario()
{
    new legion_scenario_phase_world();
    new npc_legion_scenario_combatant();
}
