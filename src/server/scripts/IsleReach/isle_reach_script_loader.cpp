/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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
 // EarthTemple
void AddSC_instance_earth_temple();
 // MysticForest
void AddSC_instance_mystic_forest();
 // TimeTemple
void AddSC_instance_time_temple();
 // Forgotten Reach
void AddSC_instance_forgotten_reach();
/* ######################## > */

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddIsleReachScripts()
{
/* ######### Dungeon ######## > */
    // EarthTemple
    AddSC_instance_earth_temple();
	// MysticForest
	AddSC_instance_mystic_forest();
	// TimeTemple
	AddSC_instance_time_temple();
	// Forgotten Reach
	AddSC_instance_forgotten_reach();
/* ######################## > */
}
