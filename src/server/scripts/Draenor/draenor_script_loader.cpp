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
		/* <Draenor> */
// Highmaul
void AddSC_instance_highmaul();
// IronDocks
void AddSC_instance_irondocks();
// Skyreach
void AddSC_instance_skyreach();
void AddSC_boss_darkweaver_syth_skyreach();
void AddSC_boss_talon_king_ikiss_skyreach();
void AddSC_boss_araknath();
// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddDraenorScripts()
{
	/* ######### Dungeon ######## > */
		/* <Draenor> */
    // Highmaul
    AddSC_instance_highmaul();
    // IronDocks
    AddSC_instance_irondocks();
	// Skyreach
    AddSC_instance_skyreach();
	AddSC_boss_darkweaver_syth_skyreach();
	AddSC_boss_talon_king_ikiss_skyreach();
	AddSC_boss_araknath();
}
