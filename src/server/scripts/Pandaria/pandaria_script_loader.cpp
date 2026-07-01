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
		/* <Pandaria> */
// HeartOfFear
void AddSC_instance_heartoffear();
// MogushanPalace
void AddSC_instance_mogushanpalace();
// MogushanVaults
void AddSC_instance_mogushanvaults();
// ShadoPanMonastery
void AddSC_instance_shadopanmonastery();
// SiegeOfNiuzaoTemple
void AddSC_instance_siegeofniuzaotemple();
void AddSC_siegeofniuzaotemple();
void AddSC_vizier_jinbak();
// StormstoutBrewery
void AddSC_instance_stormstoutbrewery();
void AddSC_stormstout_brewery();
// TempleOfTheJadeSerpent
void AddSC_instance_templeofthejadeserpent();
// TerraceOfEndlessSpring
void AddSC_instance_terraceofendlessspring();
// WanderingIsland
void AddSC_zone_the_wandering_isle();
// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddPandariaScripts()
{
/* ######### Dungeon ######## > */
		/* <Pandaria> */
    // HeartOfFear
    AddSC_instance_heartoffear();
    // MogushanPalace
    AddSC_instance_mogushanpalace();
	// MogushanVaults
    AddSC_instance_mogushanvaults();
	// ShadoPanMonastery
    AddSC_instance_shadopanmonastery();
	// SiegeOfNiuzaoTemple
    AddSC_instance_siegeofniuzaotemple();
	AddSC_siegeofniuzaotemple();
	AddSC_vizier_jinbak();
	// StormstoutBrewery
    AddSC_instance_stormstoutbrewery();
    AddSC_stormstout_brewery();
	// TempleOfTheJadeSerpent
    AddSC_instance_templeofthejadeserpent();
	// TerraceOfEndlessSpring
    AddSC_instance_terraceofendlessspring();
	// WanderingIsland
	AddSC_zone_the_wandering_isle();
}
