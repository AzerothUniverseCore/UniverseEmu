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

#include "AntorusArgus.h"
#include "Creature.h"
#include "Unit.h"
#include "SpellDefines.h"
#include "Log.h"

#include <array>
#include <unordered_map>

namespace AntorusArgus
{
    namespace
    {
        struct EncounterState
        {
            std::array<Creature*, PARTICIPANT_COUNT> participants { };
            std::array<bool, PARTICIPANT_COUNT> alive { };
            std::array<float, PARTICIPANT_COUNT> swapThresholdPct { };
            std::array<uint32, PARTICIPANT_COUNT> frozenHealth { };
            uint32 activeIndex = 0;
            bool warnedSwapFailure = false;

            EncounterState()
            {
                alive.fill(true);
            }
        };

        std::unordered_map<uint32, EncounterState> g_state;

        void ApplyPhysicalState(Creature* creature, bool active, Position const& pos)
        {
            if (!creature)
                return;

            creature->GetMotionMaster()->Clear();
            creature->ClearUnitState(UNIT_STATE_EVADE);
            creature->SetCannotReachTarget(false);

            creature->NearTeleportTo(pos);
            creature->SetRegenerateHealth(false);

            if (active)
            {
                creature->SetStandState(UNIT_STAND_STATE_STAND);
                creature->SetImmuneToPC(false, false);
                creature->SetReactState(REACT_AGGRESSIVE);
            }
            else
            {
                creature->CombatStop(true);
                creature->SetStandState(UNIT_STAND_STATE_SIT);
                creature->SetImmuneToPC(true, false);
                creature->SetReactState(REACT_PASSIVE);
            }
        }

        void ArmSwapThreshold(EncounterState& st, uint32 idx, Creature* creature)
        {
            if (!creature)
                return;

            float const current = creature->GetHealthPct();
            st.swapThresholdPct[idx] = current > SWAP_HEALTH_LOSS_PCT ? current - SWAP_HEALTH_LOSS_PCT : 0.0f;
        }

        void RepositionArgusIfSeated(EncounterState& st)
        {
            Creature* argus = st.participants[0];
            if (!argus || st.activeIndex == 0)
                return;

            Creature* activeNow = st.participants[st.activeIndex];
            if (!activeNow)
                return;

            ApplyPhysicalState(argus, false, activeNow->GetHomePosition());
        }

        int32 FindNextLiving(EncounterState const& st, uint32 fromIdx)
        {
            for (uint32 step = 1; step <= PARTICIPANT_COUNT; ++step)
            {
                uint32 const cand = (fromIdx + step) % PARTICIPANT_COUNT;
                if (cand != fromIdx && st.alive[cand])
                    return int32(cand);
            }
            return -1;
        }

        bool IsUtilityAbility(uint32 spellId)
        {
            switch (spellId)
            {
                case 9853:  // Entangling Roots (Eonar)
                case 12826: // Polymorph (Norgannon)
                case 30449: // Spellsteal (Norgannon)
                case 15122: // Counterspell (Norgannon)
                    return true;
                default:
                    return false;
            }
        }
    }

    int32 IndexOfEntry(uint32 entry)
    {
        for (uint32 i = 0; i < PARTICIPANT_COUNT; ++i)
            if (ROTATION_ORDER[i] == entry)
                return int32(i);
        return -1;
    }

    void CastRotationAbility(Creature* me, Unit* victim, uint32 slotIdx, uint32 cycleIndex)
    {
        if (!me || !victim || slotIdx >= PARTICIPANT_COUNT)
            return;

        uint32 const spellId = ROTATION_ABILITIES[slotIdx][cycleIndex % SPELLS_PER_PARTICIPANT];

        if (IsUtilityAbility(spellId))
        {
            me->CastSpell(victim, spellId, true);
            return;
        }

        int32 const damage = int32(float(me->GetMaxHealth()) * ROTATION_ABILITY_DAMAGE_PCT_OF_MAX_HEALTH);
        me->CastSpell(victim, spellId, CastSpellExtraArgs(true).AddSpellBP0(damage));
    }

    void RegisterParticipant(uint32 instanceId, Creature* creature)
    {
        if (!creature)
            return;

        int32 const idx = IndexOfEntry(creature->GetEntry());
        if (idx < 0)
        {
            SC_LOG_ERROR("server", "[Antorus] '{}' (entry {}) called RegisterParticipant but its entry isn't in ROTATION_ORDER - misconfigured spawn, ignored.",
                creature->GetName(), creature->GetEntry());
            return;
        }

        EncounterState& st = g_state[instanceId];
        st.participants[idx] = creature;
        st.alive[idx] = true;
        st.warnedSwapFailure = false;

        SC_LOG_INFO("server", "[Antorus] Registered '{}' (entry {}) as rotation slot {} for instance {}.",
            creature->GetName(), creature->GetEntry(), idx, instanceId);

        creature->GetMotionMaster()->Clear();
        creature->ClearUnitState(UNIT_STATE_EVADE);
        creature->SetCannotReachTarget(false);

        creature->SetFullHealth();
        creature->SetRegenerateHealth(false);
        st.frozenHealth[idx] = creature->GetHealth();
        ArmSwapThreshold(st, uint32(idx), creature);

        bool const isArgus = (uint32(idx) == 0);
        if (isArgus)
        {
            creature->SetStandState(UNIT_STAND_STATE_STAND);
            creature->SetImmuneToPC(false, false);
            creature->SetReactState(REACT_AGGRESSIVE);
        }
        else
        {
            creature->SetStandState(UNIT_STAND_STATE_SIT);
            creature->SetImmuneToPC(true, false);
            creature->SetReactState(REACT_PASSIVE);
        }
    }

    bool IsActiveParticipant(uint32 instanceId, uint32 entry)
    {
        auto itr = g_state.find(instanceId);
        if (itr == g_state.end())
            return false;

        int32 const idx = IndexOfEntry(entry);
        return idx >= 0 && itr->second.activeIndex == uint32(idx);
    }

    bool IsSafeForArgusToDie(uint32 instanceId)
    {
        auto itr = g_state.find(instanceId);
        if (itr == g_state.end())
            return true;

        for (uint32 i = 1; i < PARTICIPANT_COUNT; ++i)
            if (itr->second.alive[i])
                return false;

        return true;
    }

    bool CheckSwapOut(uint32 instanceId, Creature* me)
    {
        if (!me)
            return false;

        auto stateItr = g_state.find(instanceId);
        if (stateItr == g_state.end())
            return false;

        EncounterState& st = stateItr->second;
        int32 const idx = IndexOfEntry(me->GetEntry());
        if (idx < 0)
            return false;

        bool const isActiveSlot = (st.activeIndex == uint32(idx));
        float const healthPct = me->GetHealthPct();

        if (!isActiveSlot)
            return false;

        bool const argusPinnedWhileProtected = uint32(idx) == 0 && me->GetHealth() <= 1 && !IsSafeForArgusToDie(instanceId);

        if (healthPct > st.swapThresholdPct[idx] && !argusPinnedWhileProtected)
            return false;

        uint32 aliveOthers = 0;
        for (uint32 i = 0; i < PARTICIPANT_COUNT; ++i)
            if (i != uint32(idx) && st.alive[i])
                ++aliveOthers;

        if (aliveOthers == 0)
            return false;

        int32 const nextIdx = FindNextLiving(st, uint32(idx));

        Creature* incoming = nextIdx >= 0 ? st.participants[nextIdx] : nullptr;
        if (!incoming)
        {
            if (!st.warnedSwapFailure)
            {
                st.warnedSwapFailure = true;
                SC_LOG_ERROR("server", "[Antorus] Swap-out blocked for slot {} (instance {}): next slot {} has no registered creature yet.",
                    idx, instanceId, nextIdx);
            }
            return false;
        }

        Position const outgoingDestination = me->GetHomePosition();
        Position const centrePosition = st.participants[0] ? st.participants[0]->GetHomePosition() : me->GetHomePosition();

        Unit* previousVictim = me->GetVictim();

        SC_LOG_INFO("server", "[Antorus] Swap: '{}' (slot {}) retreats, '{}' (slot {}) becomes active. Instance {}.",
            me->GetName(), idx, incoming->GetName(), nextIdx, instanceId);

        me->Yell("Rest now - another shall take my place!", LANG_UNIVERSAL);

        ApplyPhysicalState(me, false, outgoingDestination);
        st.frozenHealth[idx] = me->GetHealth();

        ApplyPhysicalState(incoming, true, centrePosition);
        incoming->SetHealth(st.frozenHealth[uint32(nextIdx)]);
        ArmSwapThreshold(st, uint32(nextIdx), incoming);

        if (previousVictim)
            incoming->EngageWithTarget(previousVictim);

        st.activeIndex = uint32(nextIdx);

        RepositionArgusIfSeated(st);
        return true;
    }

    void NotifyEngaged(uint32 instanceId)
    {
        (void)instanceId;
    }

    void NotifyDeath(uint32 instanceId, uint32 entry, Unit* killer)
    {
        auto itr = g_state.find(instanceId);
        if (itr == g_state.end())
            return;

        EncounterState& st = itr->second;
        int32 const idx = IndexOfEntry(entry);
        if (idx < 0)
            return;

        st.alive[idx] = false;
        SC_LOG_INFO("server", "[Antorus] Slot {} (entry {}) died. Instance {}.", idx, entry, instanceId);

        if (st.activeIndex != uint32(idx))
            return;

        int32 const nextIdx = FindNextLiving(st, uint32(idx));
        if (nextIdx < 0)
            return;

        Creature* incoming = st.participants[nextIdx];
        if (!incoming)
        {
            SC_LOG_ERROR("server", "[Antorus] '{}' (slot {}) died while active but next slot {} has no registered creature - rotation stuck. Instance {}.",
                ROTATION_ORDER[idx], idx, nextIdx, instanceId);
            return;
        }

        Creature* argus = st.participants[0];
        Position const centrePosition = argus ? argus->GetHomePosition() : incoming->GetHomePosition();

        SC_LOG_INFO("server", "[Antorus] Slot {} died while active - '{}' (slot {}) steps up to take its place. Instance {}.",
            idx, incoming->GetName(), nextIdx, instanceId);

        ApplyPhysicalState(incoming, true, centrePosition);
        incoming->SetHealth(st.frozenHealth[uint32(nextIdx)]);
        ArmSwapThreshold(st, uint32(nextIdx), incoming);

        if (killer)
            incoming->EngageWithTarget(killer);

        st.activeIndex = uint32(nextIdx);
        RepositionArgusIfSeated(st);
    }

    void ResetEncounter(uint32 instanceId)
    {
        auto itr = g_state.find(instanceId);
        if (itr == g_state.end())
            return;

        EncounterState& st = itr->second;
        Creature* argus = st.participants[0];
        Position const centrePosition = argus ? argus->GetHomePosition() : Position();

        for (uint32 i = 0; i < PARTICIPANT_COUNT; ++i)
        {
            Creature* participant = st.participants[i];
            st.alive[i] = true;

            if (!participant)
                continue;

            bool const isArgus = (i == 0);
            ApplyPhysicalState(participant, isArgus, isArgus ? centrePosition : participant->GetHomePosition());
            participant->SetFullHealth();
            st.frozenHealth[i] = participant->GetHealth();
            ArmSwapThreshold(st, i, participant);
        }

        st.activeIndex = 0;
    }
}
