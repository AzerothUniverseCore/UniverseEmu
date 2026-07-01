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

// This is where scripts' loading functions should be declared:
/* ######### Dungeon ######## > */
		/* <Cataclysm> */
// Firelands
void AddSC_instance_firelands();
// Fyralands
void AddSC_instance_fyralands();
// TheVortexPinnacle
void AddSC_vortex_pinnacle();
void AddSC_instance_vortex_pinnacle();
void AddSC_boss_grand_vizier_ertan();
void AddSC_boss_altairus();
void AddSC_boss_asaad();
// WellOfEternity
void AddSC_instance_wellofeternity();
// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddCataclysmScripts()
{
/* ######### Dungeon ######## > */
		/* <Cataclysm> */
    // Firelands
    AddSC_instance_firelands();
    // Fyralands
    AddSC_instance_fyralands();
	// TheVortexPinnacle
    AddSC_vortex_pinnacle();
    AddSC_instance_vortex_pinnacle();
    AddSC_boss_grand_vizier_ertan();
    AddSC_boss_altairus();
    AddSC_boss_asaad();
	// WellOfEternity
    AddSC_instance_wellofeternity();
}
