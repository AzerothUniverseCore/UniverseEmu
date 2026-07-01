/*
 * This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
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

/* ######### Custom ######## > */
//void AddSC_Buff_Zone();
void AddSC_SpellRegulator();
void AddStartGuildScripts();
void AddSC_syphrena_recycling();
void AddSC_syphrena_first_aid();
void AddSC_azgath_gift_mount();
void AddSC_REFORGER_NPC();
void AddSC_Transmogrification();
void AddSC_NPC_TransmogDisplayVendor();
void AddSC_GOMove_commandscript();
void AddLfgSoloScripts();
void AddSC_TemplateNPC();
void AddSC_npc_1v1arena();
void AddSolocraftScripts();
void AddSC_XpWeekend();
void AddSC_accountachievement();
void AddSC_AccountCompanions();
void AddSC_AccountMounts();
void AddSC_premium_account();
void AddSC_custom_reload_commands();
void AddSC_minelevationtrigger();
/* ######################## > */

/* ######### Quest ######## > */
// Quest Horde Start
void AddSC_quest_horde_start();
/* ######################## > */

/* ######### Spells ######## > */
void AddSC_bloodmage_spell_scripts();
void AddSC_demon_hunter_spell_scripts();
void AddSC_evoker_spell_scripts();
void AddSC_knight_spell_scripts();
void AddSC_monk_spell_scripts();
void AddSC_tamer_spell_scripts();
void AddSC_hero_spell_scripts();
/* ######################## > */

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddUniverseScripts()
{
/* ######### Custom ######## > */
	//AddSC_Buff_Zone();
	AddSC_SpellRegulator();
	AddStartGuildScripts();
	AddSC_syphrena_recycling();
	AddSC_syphrena_first_aid();
	AddSC_azgath_gift_mount();
	AddSC_REFORGER_NPC();
	AddSC_Transmogrification();
	AddSC_NPC_TransmogDisplayVendor();
	AddSC_GOMove_commandscript();
	AddLfgSoloScripts();
	AddSC_TemplateNPC();
	AddSC_npc_1v1arena();
	AddSolocraftScripts();
	AddSC_XpWeekend();
	AddSC_accountachievement();
	AddSC_AccountCompanions();
	AddSC_AccountMounts();
	AddSC_premium_account();
	AddSC_custom_reload_commands();
	AddSC_minelevationtrigger();
/* ######################## > */
	
/* ######### Quest ######## > */
    // Quest Horde Start
    AddSC_quest_horde_start();
/* ######################## > */
	
/* ######### Spells ######## > */
	AddSC_bloodmage_spell_scripts();
    AddSC_demon_hunter_spell_scripts();
	AddSC_evoker_spell_scripts();
	AddSC_knight_spell_scripts();
	AddSC_monk_spell_scripts();
	AddSC_tamer_spell_scripts();
	AddSC_hero_spell_scripts();
/* ######################## > */
}
