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

#ifndef __ANTORUS_ARGUS_H
#define __ANTORUS_ARGUS_H

#include "Define.h"

class Creature;
class Unit;

namespace AntorusArgus
{
    constexpr uint32 MAP_ID = 737;

    constexpr uint32 NPC_ARGUS_THE_UNMAKER = 124828;
    constexpr uint32 NPC_AMANTHUL   = 125885;
    constexpr uint32 NPC_KHAZGOROTH = 125886;
    constexpr uint32 NPC_AGGRAMAR   = 125893;
    constexpr uint32 NPC_NORGANNON  = 126266;
    constexpr uint32 NPC_EONAR      = 126267;
    constexpr uint32 NPC_GOLGANNETH = 126268;

    constexpr uint32 PARTICIPANT_COUNT = 7;

    constexpr uint32 ROTATION_ORDER[PARTICIPANT_COUNT] =
    {
        NPC_ARGUS_THE_UNMAKER, NPC_GOLGANNETH, NPC_KHAZGOROTH, NPC_EONAR,
        NPC_AMANTHUL, NPC_AGGRAMAR, NPC_NORGANNON
    };

    constexpr uint32 SPELLS_PER_PARTICIPANT = 6;

    constexpr uint32 ROTATION_ABILITIES[PARTICIPANT_COUNT][SPELLS_PER_PARTICIPANT] =
    {
        // Slot 0 - Argus l'Annihilateur: corrupted, chaotic power.
        { 47897, 59172, 46161, 40876, 39252, 37945 }, // Shadowflame, Chaos Bolt, Void Blast, Doom Bolt, Void Bolt, Fel Fireball
        // Slot 1 - Golganneth, l'Orageux: storms and lightning.
        { 15117, 15588, 6041, 57784, 28299, 61628 }, // Chain Lightning, Thunderclap, Lightning Bolt, Thunderstorm, Ball Lightning, Storm Bolt
        // Slot 2 - Khaz'goroth, le Forgeron: forge, fire and earth.
        { 35565, 15095, 40117, 49656, 26093, 76202 }, // Earthquake, Molten Blast, Volcanic Eruption, Fissure, Quake, Seismic Crash
        // Slot 3 - Eonar, la Bienfaitrice: nature and life.
        { 5177, 17402, 24977, 9853, 8926, 63570 }, // Wrath, Hurricane, Insect Swarm, Entangling Roots, Moonfire, Nature's Fury
        // Slot 4 - Aman'Thul, le Grand Père: holy order.
        { 27139, 15265, 27173, 25054, 17149, 38631 }, // Holy Wrath, Holy Fire, Consecration, Holy Smite, Exorcism, Avenger's Shield
        // Slot 5 - Aggramar, le Champion: valorous fire and steel.
        { 44190, 31661, 11113, 46968, 15576, 11608 }, // Flame Strike, Dragon's Breath, Blast Wave, Shockwave, Whirlwind, Cleave
        // Slot 6 - Norgannon, le Sage: arcane and order.
        { 22893, 8439, 50273, 12826, 30449, 15122 }, // Arcane Blast, Arcane Explosion, Arcane Barrage, Polymorph, Spellsteal, Counterspell
    };

    constexpr float ROTATION_ABILITY_DAMAGE_PCT_OF_MAX_HEALTH = 0.02f;

    constexpr uint32 ROTATION_ABILITY_COOLDOWN_MS = 5000;

    constexpr float SWAP_HEALTH_LOSS_PCT = 30.0f;

    constexpr float LEASH_RADIUS = 60.0f;

    constexpr float PANTHEON_SCALE_SEATED = 1.4f;
    constexpr float PANTHEON_SCALE_ACTIVE = 1.0f;

    int32 IndexOfEntry(uint32 entry);

    void CastRotationAbility(Creature* me, Unit* victim, uint32 slotIdx, uint32 cycleIndex);

    void RegisterParticipant(uint32 instanceId, Creature* creature);

    bool IsActiveParticipant(uint32 instanceId, uint32 entry);

    bool IsSafeForArgusToDie(uint32 instanceId);

    bool CheckSwapOut(uint32 instanceId, Creature* me);

    void NotifyEngaged(uint32 instanceId);

    void NotifyDeath(uint32 instanceId, uint32 entry, Unit* killer);

    void ResetEncounter(uint32 instanceId);
}

#endif
