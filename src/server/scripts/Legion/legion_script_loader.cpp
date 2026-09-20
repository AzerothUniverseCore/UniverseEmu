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

// This is where scripts' loading functions should be declared:
/* ######### Dungeon ######## > */
		/* <Legion> */
// AntorusTheBurningThrone
void AddSC_instance_antorustheburningthrone();
void AddSC_npc_antorus_argus_the_unmaker();
void AddSC_npc_antorus_pantheon_guardian();
// DalaranLegion
void AddSC_dalaran_legion();
// HallsOfValor
void AddSC_instance_hallsofvalor();
// LegionShip
void AddSC_instance_legionship();
// TheNighthold
void AddSC_instance_thenighthold();
// TheBattleBrokenShore
void AddSC_legion_scenario();
void AddSC_legion_escort();
// LegionShip Intro
void AddSC_legion_ship_intro();
// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddLegionScripts()
{
/* ######### Dungeon ######## > */
		/* <Legion> */
    // AntorusTheBurningThrone
    AddSC_instance_antorustheburningthrone();
    AddSC_npc_antorus_argus_the_unmaker();
    AddSC_npc_antorus_pantheon_guardian();
    // DalaranLegion
    AddSC_dalaran_legion();
	// HallsOfValor
    AddSC_instance_hallsofvalor();
	// LegionShip
    AddSC_instance_legionship();
	// TheNighthold
    AddSC_instance_thenighthold();
	// TheBattleBrokenShore
	AddSC_legion_scenario();
	AddSC_legion_escort();
	// LegionShip Intro
	AddSC_legion_ship_intro();
}
