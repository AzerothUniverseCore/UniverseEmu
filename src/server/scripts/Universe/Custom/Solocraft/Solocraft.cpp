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
 *
 * Orignal Azeroth Module Pulled from https://github.com/azerothcore/mod-solocraft
 * Which is a fork of https://github.com/conan513/mod-solocraft
 * Refactored to Syphrenacore Custom 335
 * and improved upon by Single Player Project Developer MDic
 */

#include <map>
#include "Config.h"
#include "DatabaseEnv.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Pet.h"
#include "Map.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Chat.h"
#include "Log.h"
#include <math.h>
#include <unordered_map>

bool SoloCraftEnable = 1;
bool SoloCraftAnnounceModule = 1;
bool SoloCraftDebuffEnable = 1;
float SoloCraftSpellMult = 1.0;
float SoloCraftStatsMult = 100.0;
uint32 SolocraftLevelDiff = 1;
uint32 SolocraftDungeonLevel = 1;
std::unordered_map<uint32, uint32> dungeons;
std::unordered_map<uint32, float> diff_Multiplier;
std::unordered_map<uint32, float> diff_Multiplier_Heroics;
std::unordered_map<uint8, uint32> classes;
float D5 = 1.0;
float D10 = 1.0;
float D25 = 1.0;
float D40 = 1.0;
float D649H10 = 1.0;
float D649H25 = 1.0;

class SolocraftConfig : public WorldScript
{
public:
    SolocraftConfig() : WorldScript("SolocraftConfig") {}
    // Load Configuration Settings
    void OnConfigLoad(bool /*reload*/) override
    {
        SoloCraftEnable = sConfigMgr->GetBoolDefault("Solocraft.Enable", 1);
        SoloCraftAnnounceModule = sConfigMgr->GetBoolDefault("Solocraft.Announce", 1);
        //Balancing
        SoloCraftDebuffEnable = sConfigMgr->GetBoolDefault("SoloCraft.Debuff.Enable", 1);
        SoloCraftSpellMult = sConfigMgr->GetFloatDefault("SoloCraft.Spellpower.Mult", 2.5);
        SoloCraftStatsMult = sConfigMgr->GetFloatDefault("SoloCraft.Stats.Mult", 100.0);
        classes =
        {
          {1, sConfigMgr->GetIntDefault("SoloCraft.WARRIOR", 100) },
          {2, sConfigMgr->GetIntDefault("SoloCraft.PALADIN", 100) },
          {3, sConfigMgr->GetIntDefault("SoloCraft.HUNTER", 100) },
          {4, sConfigMgr->GetIntDefault("SoloCraft.ROGUE", 100) },
          {5, sConfigMgr->GetIntDefault("SoloCraft.PRIEST", 100) },
          {6, sConfigMgr->GetIntDefault("SoloCraft.DEATH_KNIGHT", 100) },
          {7, sConfigMgr->GetIntDefault("SoloCraft.SHAMAN", 100) },
          {8, sConfigMgr->GetIntDefault("SoloCraft.MAGE", 100) },
          {9, sConfigMgr->GetIntDefault("SoloCraft.WARLOCK", 100) },
		  {10, sConfigMgr->GetIntDefault("SoloCraft.BLOODMAGE", 100) },
          {11, sConfigMgr->GetIntDefault("SoloCraft.DRUID", 100) },
		  {12, sConfigMgr->GetIntDefault("SoloCraft.KNIGHT", 100) },
		  {13, sConfigMgr->GetIntDefault("SoloCraft.DEMON_HUNTER", 100) },
		  {14, sConfigMgr->GetIntDefault("SoloCraft.MONK", 100) },
		  {15, sConfigMgr->GetIntDefault("SoloCraft.TAMER", 100) },
          {16, sConfigMgr->GetIntDefault("SoloCraft.HERO", 100) },
          {17, sConfigMgr->GetIntDefault("SoloCraft.EVOKER", 100) },
          {18, sConfigMgr->GetIntDefault("SoloCraft.NECROMANCER", 100) },
          {19, sConfigMgr->GetIntDefault("SoloCraft.VENOMANCER", 100) },
          {20, sConfigMgr->GetIntDefault("SoloCraft.PYROMANCER", 100) },
          {21, sConfigMgr->GetIntDefault("SoloCraft.CHRONOMANCER", 100) },
          {22, sConfigMgr->GetIntDefault("SoloCraft.GEOMANCER", 100) },
          {23, sConfigMgr->GetIntDefault("SoloCraft.CHAOSRAVAGER", 100) },
        };
        //Level Thresholds
        SolocraftLevelDiff = sConfigMgr->GetIntDefault("Solocraft.Max.Level.Diff", 0);
        //Catch All Dungeon Level Threshold
        SolocraftDungeonLevel = sConfigMgr->GetIntDefault("Solocraft.Dungeon.Level", 80);
        // Dungeon Base Level
        dungeons =
        {
            // Wow Classic
            {33, sConfigMgr->GetIntDefault("Solocraft.ShadowfangKeep.Level", 15) },
            {34, sConfigMgr->GetIntDefault("Solocraft.Stockades.Level", 22) },
            {36, sConfigMgr->GetIntDefault("Solocraft.Deadmines.Level", 18) },
            {43, sConfigMgr->GetIntDefault("Solocraft.WailingCaverns.Level", 17) },
            {47, sConfigMgr->GetIntDefault("Solocraft.RazorfenKraulInstance.Level", 30) },
            {48, sConfigMgr->GetIntDefault("Solocraft.Blackfathom.Level", 20) },
            {70, sConfigMgr->GetIntDefault("Solocraft.Uldaman.Level", 40) },
            {90, sConfigMgr->GetIntDefault("Solocraft.GnomeragonInstance.Level", 24) },
            {109, sConfigMgr->GetIntDefault("Solocraft.SunkenTemple.Level", 50) },
            {129, sConfigMgr->GetIntDefault("Solocraft.RazorfenDowns.Level", 40) },
            {189, sConfigMgr->GetIntDefault("Solocraft.MonasteryInstances.Level", 35) },                  // Scarlet Monastery
            {209, sConfigMgr->GetIntDefault("Solocraft.TanarisInstance.Level", 44) },                     // Zul'Farrak
            {229, sConfigMgr->GetIntDefault("Solocraft.BlackRockSpire.Level", 55) },
            {230, sConfigMgr->GetIntDefault("Solocraft.BlackrockDepths.Level", 50) },
            {249, sConfigMgr->GetIntDefault("Solocraft.OnyxiaLairInstance.Level", 80) },
            {289, sConfigMgr->GetIntDefault("Solocraft.SchoolofNecromancy.Level", 55) },                  // Scholomance
            {309, sConfigMgr->GetIntDefault("Solocraft.Zul'gurub.Level", 80) },
            {329, sConfigMgr->GetIntDefault("Solocraft.Stratholme.Level", 55) },
            {349, sConfigMgr->GetIntDefault("Solocraft.Mauradon.Level", 48) },
            {389, sConfigMgr->GetIntDefault("Solocraft.OrgrimmarInstance.Level", 15) },                   // Ragefire Chasm
            {409, sConfigMgr->GetIntDefault("Solocraft.MoltenCore.Level", 80) },
            {429, sConfigMgr->GetIntDefault("Solocraft.DireMaul.Level", 48) },
            {469, sConfigMgr->GetIntDefault("Solocraft.BlackwingLair.Level", 40) },
            {509, sConfigMgr->GetIntDefault("Solocraft.AhnQiraj.Level", 80) },                            // Ruins of Ahn'Qiraj
            {531, sConfigMgr->GetIntDefault("Solocraft.AhnQirajTemple.Level", 80) },
            // BC Instances
            {269, sConfigMgr->GetIntDefault("Solocraft.CavernsOfTime.Level", 80) },                       // The Black Morass
            {532, sConfigMgr->GetIntDefault("Solocraft.Karazahn.Level", 80) },
            {534, sConfigMgr->GetIntDefault("Solocraft.HyjalPast.Level", 80) },                           // The Battle for Mount Hyjal - Hyjal Summit
            {540, sConfigMgr->GetIntDefault("Solocraft.HellfireMilitary.Level", 80) },                    // The Shattered Halls
            {542, sConfigMgr->GetIntDefault("Solocraft.HellfireDemon.Level", 80) },                       // The Blood Furnace
            {543, sConfigMgr->GetIntDefault("Solocraft.HellfireRampart.Level", 80) },
            {544, sConfigMgr->GetIntDefault("Solocraft.HellfireRaid.Level", 80) },                        // Magtheridon's Lair
            {545, sConfigMgr->GetIntDefault("Solocraft.CoilfangPumping.Level", 80) },                     // The Steamvault
            {546, sConfigMgr->GetIntDefault("Solocraft.CoilfangMarsh.Level", 80) },                       // The Underbog
            {547, sConfigMgr->GetIntDefault("Solocraft.CoilfangDraenei.Level", 80) },                     // The Slavepens
            {548, sConfigMgr->GetIntDefault("Solocraft.CoilfangRaid.Level", 80) },                        // Serpentshrine Cavern
            {550, sConfigMgr->GetIntDefault("Solocraft.TempestKeepRaid.Level", 80) },                     // The Eye
            {552, sConfigMgr->GetIntDefault("Solocraft.TempestKeepArcane.Level", 80) },                   // The Arcatraz
            {553, sConfigMgr->GetIntDefault("Solocraft.TempestKeepAtrium.Level", 80) },                   // The Botanica
            {554, sConfigMgr->GetIntDefault("Solocraft.TempestKeepFactory.Level", 80) },                  // The Mechanar
            {555, sConfigMgr->GetIntDefault("Solocraft.AuchindounShadow.Level", 80) },                    // Shadow Labyrinth
            {556, sConfigMgr->GetIntDefault("Solocraft.AuchindounDemon.Level", 80) },                     // Sethekk Halls
            {557, sConfigMgr->GetIntDefault("Solocraft.AuchindounEthereal.Level", 80) },                  // Mana-Tombs
            {558, sConfigMgr->GetIntDefault("Solocraft.AuchindounDraenei.Level", 80) },                   // Auchenai Crypts
            {560, sConfigMgr->GetIntDefault("Solocraft.HillsbradPast.Level", 80) },                       // Old Hillsbrad Foothills
            {564, sConfigMgr->GetIntDefault("Solocraft.BlackTemple.Level", 80) },
            {565, sConfigMgr->GetIntDefault("Solocraft.GruulsLair.Level", 80) },
            {568, sConfigMgr->GetIntDefault("Solocraft.ZulAman.Level", 80) },
            {580, sConfigMgr->GetIntDefault("Solocraft.SunwellPlateau.Level", 80) },
            {585, sConfigMgr->GetIntDefault("Solocraft.Sunwell5ManFix.Level", 80) },                      // Magister's Terrace
            // WOTLK Instances
            {533, sConfigMgr->GetIntDefault("Solocraft.StratholmeRaid.Level", 80) },                      // Naxxramas
            {574, sConfigMgr->GetIntDefault("Solocraft.Valgarde70.Level", 80) },                          // Utgarde Keep
            {575, sConfigMgr->GetIntDefault("Solocraft.UtgardePinnacle.Level", 80) },
            {576, sConfigMgr->GetIntDefault("Solocraft.Nexus70.Level", 80) },                             // The Nexus
            {578, sConfigMgr->GetIntDefault("Solocraft.Nexus80.Level", 80) },                             // The Occulus
            {595, sConfigMgr->GetIntDefault("Solocraft.StratholmeCOT.Level", 80) },                       // The Culling of Stratholme
            {599, sConfigMgr->GetIntDefault("Solocraft.Ulduar70.Level", 80) },                            // Halls of Stone
            {600, sConfigMgr->GetIntDefault("Solocraft.DrakTheronKeep.Level", 80) },                      // Drak'Tharon Keep
            {601, sConfigMgr->GetIntDefault("Solocraft.Azjol_Uppercity.Level", 80) },                     // Azjol-Nerub
            {602, sConfigMgr->GetIntDefault("Solocraft.Ulduar80.Level", 80) },                            // Halls of Lighting
            {603, sConfigMgr->GetIntDefault("Solocraft.UlduarRaid.Level", 80) },                          // Ulduar
            {604, sConfigMgr->GetIntDefault("Solocraft.GunDrak.Level", 80) },
            {608, sConfigMgr->GetIntDefault("Solocraft.DalaranPrison.Level", 80) },                       // Violet Hold
            {615, sConfigMgr->GetIntDefault("Solocraft.ChamberOfAspectsBlack.Level", 80) },               // The Obsidian Sanctum
            {616, sConfigMgr->GetIntDefault("Solocraft.NexusRaid.Level", 80) },                           // The Eye of Eternity
            {619, sConfigMgr->GetIntDefault("Solocraft.Azjol_LowerCity.Level", 80) },                     // Ahn'kahet: The Old Kingdom
            {631, sConfigMgr->GetIntDefault("Solocraft.IcecrownCitadel.Level", 80) },                     // Icecrown Citadel
            {632, sConfigMgr->GetIntDefault("Solocraft.IcecrownCitadel5Man.Level", 80) },                 // The Forge of Souls
            {649, sConfigMgr->GetIntDefault("Solocraft.ArgentTournamentRaid.Level", 80) },                // Trial of the Crusader
            {650, sConfigMgr->GetIntDefault("Solocraft.ArgentTournamentDungeon.Level", 80) },             // Trial of the Champion
            {658, sConfigMgr->GetIntDefault("Solocraft.QuarryOfTears.Level", 80) },                       // Pit of Saron
            {668, sConfigMgr->GetIntDefault("Solocraft.HallsOfReflection.Level", 80) },                   // Halls of Reflection
            {724, sConfigMgr->GetIntDefault("Solocraft.ChamberOfAspectsRed.Level", 80) },                 // The Ruby Sanctum
			// Instance Universe
			{732, sConfigMgr->GetIntDefault("Solocraft.Fyralands.Level", 80) },                           // Fyralands S0
            {734, sConfigMgr->GetIntDefault("Solocraft.Highmaul.Level", 80) },                            // Highmaul S3
			{735, sConfigMgr->GetIntDefault("Solocraft.LegionShip.Level", 80) },                          // Legion Ship A2
			{736, sConfigMgr->GetIntDefault("Solocraft.Firelands.Level", 80) },                           // Firelands S1
            {737, sConfigMgr->GetIntDefault("Solocraft.AntorusBurningThrone.Level", 80) },                // Antorus, the Burning Throne S3
			{750, sConfigMgr->GetIntDefault("Solocraft.MogushanPalace.Level", 80) },                      // Mogushan Palace S2
			{751, sConfigMgr->GetIntDefault("Solocraft.MogushanVaults.Level", 80) },                      // Mogu'shan Vaults S3
			{763, sConfigMgr->GetIntDefault("Solocraft.TempleJadeSerpent.Level", 80) },                   // Temple of the Jade Serpent S2
			{764, sConfigMgr->GetIntDefault("Solocraft.ShadowpanMonastery.Level", 80) },                  // Shadowpan Monastery A0
            {765, sConfigMgr->GetIntDefault("Solocraft.TerraceEndlessSpring.Level", 80) },                // Terrace of Endless Spring S5
			{775, sConfigMgr->GetIntDefault("Solocraft.StormstoutBrewery.Level", 80) },                   // StormstoutBrewery A1
            {776, sConfigMgr->GetIntDefault("Solocraft.VortexPinnacle.Level", 80) },                      // The Vortex Pinnacle S5
            {778, sConfigMgr->GetIntDefault("Solocraft.SiegeNiuzaoTemple.Level", 10) },                   // Siege of Niuzao Temple S6
            {779, sConfigMgr->GetIntDefault("Solocraft.HeartFear.Level", 80) },                           // Heart of Fear S4
            {791, sConfigMgr->GetIntDefault("Solocraft.IronDocks.Level", 80) },                           // Iron Docks S4
            {794, sConfigMgr->GetIntDefault("Solocraft.WellEternity.Level", 10) },                        // Well of Eternity S7
            {795, sConfigMgr->GetIntDefault("Solocraft.Skyreach.Level", 10) },                            // Skyreach S6
            {796, sConfigMgr->GetIntDefault("Solocraft.HallsValor.Level", 10) },                          // Halls of Valor S7
            {797, sConfigMgr->GetIntDefault("Solocraft.Nighthold.Level", 10) },                           // Nighthold S8
            {821, sConfigMgr->GetIntDefault("Solocraft.BaradinHold.Level", 80) },                         // BaradinHold A3
            {814, sConfigMgr->GetIntDefault("Solocraft.BlackRookHold.Level", 80) },                       // BlackRookHold A4
            {823, sConfigMgr->GetIntDefault("Solocraft.TimeTemple.Level", 15) },                          // TimeTemple
            {825, sConfigMgr->GetIntDefault("Solocraft.MysticForest.Level", 20) },                        // Mystic Forest
        };
        // Dungeon Difficulty
        // Catch alls
        D5 = sConfigMgr->GetFloatDefault("Solocraft.Dungeon", 5.0);
        D10 = sConfigMgr->GetFloatDefault("Solocraft.Heroic", 10.0);
        D25 = sConfigMgr->GetFloatDefault("Solocraft.Raid25", 25.0);
        D40 = sConfigMgr->GetFloatDefault("Solocraft.Raid40", 40.0);
        diff_Multiplier =
        {
            // WOW Classic Instances
            {33, sConfigMgr->GetFloatDefault("Solocraft.ShadowfangKeep", 5.0) },
            {34, sConfigMgr->GetFloatDefault("Solocraft.Stockades", 5.0) },
            {36, sConfigMgr->GetFloatDefault("Solocraft.Deadmines", 5.0) },
            {43, sConfigMgr->GetFloatDefault("Solocraft.WailingCaverns", 5.0) },
            {47, sConfigMgr->GetFloatDefault("Solocraft.RazorfenKraulInstance", 5.0) },
            {48, sConfigMgr->GetFloatDefault("Solocraft.Blackfathom", 5.0) },
            {70, sConfigMgr->GetFloatDefault("Solocraft.Uldaman", 5.0) },
            {90, sConfigMgr->GetFloatDefault("Solocraft.GnomeragonInstance", 5.0) },
            {109, sConfigMgr->GetFloatDefault("Solocraft.SunkenTemple", 5.0) },
            {129, sConfigMgr->GetFloatDefault("Solocraft.RazorfenDowns", 5.0) },
            {189, sConfigMgr->GetFloatDefault("Solocraft.MonasteryInstances", 5.0) },                     // Scarlet
            {209, sConfigMgr->GetFloatDefault("Solocraft.TanarisInstance", 5.0) },                        // Zul'Farrak
            {229, sConfigMgr->GetFloatDefault("Solocraft.BlackRockSpire", 10.0) },
            {230, sConfigMgr->GetFloatDefault("Solocraft.BlackrockDepths", 5.0) },
            {249, sConfigMgr->GetFloatDefault("Solocraft.OnyxiaLairInstance", 40.0) },
            {289, sConfigMgr->GetFloatDefault("Solocraft.SchoolofNecromancy", 5.0) },                     // Scholo
            {309, sConfigMgr->GetFloatDefault("Solocraft.Zul'gurub", 20.0) },
            {329, sConfigMgr->GetFloatDefault("Solocraft.Stratholme", 5.0) },
            {349, sConfigMgr->GetFloatDefault("Solocraft.Mauradon", 5.0) },
            {389, sConfigMgr->GetFloatDefault("Solocraft.OrgrimmarInstance", 5.0) },                      // Ragefire
            {409, sConfigMgr->GetFloatDefault("Solocraft.MoltenCore", 40.0) },
            {429, sConfigMgr->GetFloatDefault("Solocraft.DireMaul", 5.0) },
            {469, sConfigMgr->GetFloatDefault("Solocraft.BlackwingLair", 40.0) },
            {509, sConfigMgr->GetFloatDefault("Solocraft.AhnQiraj", 20.0) },
            {531, sConfigMgr->GetFloatDefault("Solocraft.AhnQirajTemple", 40.0) },
            // BC Instances
            {269, sConfigMgr->GetFloatDefault("Solocraft.CavernsOfTime", 5.0) },                          // Black Morass
            {532, sConfigMgr->GetFloatDefault("Solocraft.Karazahn", 10.0) },
            {534, sConfigMgr->GetFloatDefault("Solocraft.HyjalPast", 25.0) },                             // Mount Hyjal
            {540, sConfigMgr->GetFloatDefault("Solocraft.HellfireMilitary", 5.0) },                       // The Shattered Halls
            {542, sConfigMgr->GetFloatDefault("Solocraft.HellfireDemon", 5.0) },                          // The Blood Furnace
            {543, sConfigMgr->GetFloatDefault("Solocraft.HellfireRampart", 5.0) },
            {544, sConfigMgr->GetFloatDefault("Solocraft.HellfireRaid", 25.0) },                          // Magtheridon's Lair
            {545, sConfigMgr->GetFloatDefault("Solocraft.CoilfangPumping", 5.0) },                        // The Steamvault
            {546, sConfigMgr->GetFloatDefault("Solocraft.CoilfangMarsh", 5.0) },                          // The Underbog
            {547, sConfigMgr->GetFloatDefault("Solocraft.CoilfangDraenei", 5.0) },                        // The Slavepens
            {548, sConfigMgr->GetFloatDefault("Solocraft.CoilfangRaid", 25.0) },                          // Serpentshrine Cavern
            {550, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepRaid", 25.0) },                       // The Eye
            {552, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepArcane", 5.0) },                      // The Arcatraz
            {553, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepAtrium", 5.0) },                      // The Botanica
            {554, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepFactory", 5.0) },                     // The Mechanar
            {555, sConfigMgr->GetFloatDefault("Solocraft.AuchindounShadow", 5.0) },                       // Shadow Labyrinth
            {556, sConfigMgr->GetFloatDefault("Solocraft.AuchindounDemon", 5.0) },                        // Sethekk Halls
            {557, sConfigMgr->GetFloatDefault("Solocraft.AuchindounEthereal", 5.0) },                     // Mana-Tombs
            {558, sConfigMgr->GetFloatDefault("Solocraft.AuchindounDraenei", 5.0) },                      // Auchenai Crypts
            {560, sConfigMgr->GetFloatDefault("Solocraft.HillsbradPast", 5.0) },                          // Old Hillsbrad Foothills
            {564, sConfigMgr->GetFloatDefault("Solocraft.BlackTemple", 25.0) },
            {565, sConfigMgr->GetFloatDefault("Solocraft.GruulsLair", 25.0) },
            {568, sConfigMgr->GetFloatDefault("Solocraft.ZulAman", 5.0) },
            {580, sConfigMgr->GetFloatDefault("Solocraft.SunwellPlateau", 25.0) },
            {585, sConfigMgr->GetFloatDefault("Solocraft.Sunwell5ManFix", 5.0) },                         // Magister's Terrace
            // WOTLK Instances
            {533, sConfigMgr->GetFloatDefault("Solocraft.StratholmeRaid", 10.0) },                        //  Nax 10
            {574, sConfigMgr->GetFloatDefault("Solocraft.Valgarde70", 5.0) },                             // Utgarde Keep
            {575, sConfigMgr->GetFloatDefault("Solocraft.UtgardePinnacle", 5.0) },
            {576, sConfigMgr->GetFloatDefault("Solocraft.Nexus70", 5.0) },                                // The Nexus
            {578, sConfigMgr->GetFloatDefault("Solocraft.Nexus80", 5.0) },                                // The Occulus
            {595, sConfigMgr->GetFloatDefault("Solocraft.StratholmeCOT", 5.0) },                          // The Culling of Stratholme
            {599, sConfigMgr->GetFloatDefault("Solocraft.Ulduar70", 5.0) },                               // Halls of Stone
            {600, sConfigMgr->GetFloatDefault("Solocraft.DrakTheronKeep", 5.0) },                         // Drak'Tharon Keep
            {601, sConfigMgr->GetFloatDefault("Solocraft.Azjol_Uppercity", 5.0) },                        // Azjol-Nerub
            {602, sConfigMgr->GetFloatDefault("Solocraft.Ulduar80", 5.0) },                               // Halls of Lighting
            {603, sConfigMgr->GetFloatDefault("Solocraft.UlduarRaid", 10.0) },                            // Ulduar 10
            {604, sConfigMgr->GetFloatDefault("Solocraft.GunDrak", 5.0) },
            {608, sConfigMgr->GetFloatDefault("Solocraft.DalaranPrison", 5.0) },                          // Violet Hold
            {615, sConfigMgr->GetFloatDefault("Solocraft.ChamberOfAspectsBlack", 10.0) },                 // The Obsidian Sanctum 10
            {616, sConfigMgr->GetFloatDefault("Solocraft.NexusRaid", 10.0) },                             // The Eye of Eternity 10
            {619, sConfigMgr->GetFloatDefault("Solocraft.Azjol_LowerCity", 5.0) },                        // Ahn'kahet: The Old Kingdom
            {631, sConfigMgr->GetFloatDefault("Solocraft.IcecrownCitadel", 10.0) },                       // Icecrown Citadel 10
            {632, sConfigMgr->GetFloatDefault("Solocraft.IcecrownCitadel5Man", 5.0) },                    // The Forge of Souls
            {649, sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentRaid", 10.0) },                  // Trial of the Crusader 10
            {650, sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentDungeon", 5.0) },                // Trial of the Champion
            {658, sConfigMgr->GetFloatDefault("Solocraft.QuarryOfTears", 5.0) },                          // Pit of Saron
            {668, sConfigMgr->GetFloatDefault("Solocraft.HallsOfReflection", 5.0) },                      // Halls of Reflection
            {724, sConfigMgr->GetFloatDefault("Solocraft.ChamberOfAspectsRed", 10.0) },                   // The Ruby Sanctum 10
            // Universe Instances
			{732, sConfigMgr->GetFloatDefault("Solocraft.Fyralands", 5.0) },                              // Fyralands S0
            {734, sConfigMgr->GetFloatDefault("Solocraft.Highmaul", 5.0) },                               // Highmaul S3
			{735, sConfigMgr->GetFloatDefault("Solocraft.LegionShip", 5.0) },                             // Legion Ship A2
			{736, sConfigMgr->GetFloatDefault("Solocraft.Firelands", 5.0) },                              // Firelands S1
            {737, sConfigMgr->GetFloatDefault("Solocraft.AntorusBurningThrone", 5.0) },                   // Antorus, the Burning Throne
			{750, sConfigMgr->GetFloatDefault("Solocraft.MogushanPalace", 5.0) },                         // Mogushan Palace S2
			{751, sConfigMgr->GetFloatDefault("Solocraft.MogushanVaults", 5.0) },                         // Mogu'shan Vaults S3
			{763, sConfigMgr->GetFloatDefault("Solocraft.TempleJadeSerpent", 5.0) },                      // Temple of the Jade Serpent S2
			{764, sConfigMgr->GetFloatDefault("Solocraft.ShadowpanMonastery", 5.0) },                     // Shadowpan Monastery A0
            {765, sConfigMgr->GetFloatDefault("Solocraft.TerraceEndlessSpring", 5.0) },                   // Terrace of Endless Spring S5
			{775, sConfigMgr->GetFloatDefault("Solocraft.StormstoutBrewery", 5.0) },                      // StormstoutBrewery A1
            {776, sConfigMgr->GetFloatDefault("Solocraft.VortexPinnacle", 5.0) },                         // The Vortex Pinnacle S5
            {778, sConfigMgr->GetFloatDefault("Solocraft.SiegeNiuzaoTemple", 0.0) },                      // Siege of Niuzao Temple S6
            {779, sConfigMgr->GetFloatDefault("Solocraft.HeartFear", 5.0) },                              // Heart of Fear S4
            {791, sConfigMgr->GetFloatDefault("Solocraft.IronDocks", 5.0) },                              // Iron Docks S4
            {794, sConfigMgr->GetFloatDefault("Solocraft.WellEternity", 0.0) },                           // Well of Eternity S7
            {795, sConfigMgr->GetFloatDefault("Solocraft.Skyreach", 0.0) },                               // Skyreach S6
            {796, sConfigMgr->GetFloatDefault("Solocraft.HallsValor", 0.0) },                             // Halls of Valor S7
            {797, sConfigMgr->GetFloatDefault("Solocraft.Nighthold", 0.0) },                              // Nighthold S8
            {821, sConfigMgr->GetFloatDefault("Solocraft.BaradinHold", 5.0) },                            // BaradinHold A3
            {814, sConfigMgr->GetFloatDefault("Solocraft.BlackRookHold", 5.0) },                          // BlackRookHold A4
            {823, sConfigMgr->GetFloatDefault("Solocraft.TimeTemple", 5.0) },                             // TimeTemple
            {825, sConfigMgr->GetFloatDefault("Solocraft.MysticForest", 5.0) },                           // MysticForest
        };
        // Heroics
        diff_Multiplier_Heroics =
        {
            // BC Instances Heroics
            {269, sConfigMgr->GetFloatDefault("Solocraft.CavernsOfTimeH", 5.0) },                         // Black Morass H
            {540, sConfigMgr->GetFloatDefault("Solocraft.HellfireMilitaryH", 5.0) },                      // The Shattered Halls H
            {542, sConfigMgr->GetFloatDefault("Solocraft.HellfireDemonH", 5.0) },                         // The Blood Furnace H
            {543, sConfigMgr->GetFloatDefault("Solocraft.HellfireRampartH", 5.0) },                       // Heroic
            {545, sConfigMgr->GetFloatDefault("Solocraft.CoilfangPumpingH", 5.0) },                       // The Steamvault
            {546, sConfigMgr->GetFloatDefault("Solocraft.CoilfangMarshH", 5.0) },                         // The Underbog
            {547, sConfigMgr->GetFloatDefault("Solocraft.CoilfangDraeneiH", 5.0) },                       // The Slavepens  H
            {552, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepArcaneH", 5.0) },                     // The Arcatraz H
            {553, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepAtriumH", 5.0) },                     // The Botanica H
            {554, sConfigMgr->GetFloatDefault("Solocraft.TempestKeepFactoryH", 5.0) },                    // The Mechanar H
            {555, sConfigMgr->GetFloatDefault("Solocraft.AuchindounShadowH", 5.0) },                      // Shadow Labyrinth H
            {556, sConfigMgr->GetFloatDefault("Solocraft.AuchindounDemonH", 5.0) },                       // Sethekk Halls H
            {557, sConfigMgr->GetFloatDefault("Solocraft.AuchindounEtherealH", 5.0) },                    // Mana-Tombs H
            {558, sConfigMgr->GetFloatDefault("Solocraft.AuchindounDraeneiH", 5.0) },                     // Auchenai Crypts H
            {560, sConfigMgr->GetFloatDefault("Solocraft.HillsbradPastH", 5.0) },                         // Old Hillsbrad Foothills H
            {568, sConfigMgr->GetFloatDefault("Solocraft.ZulAmanH", 5.0) },                               // Zul'Aman H
            {585, sConfigMgr->GetFloatDefault("Solocraft.Sunwell5ManFixH", 5.0) },                        // Magister's Terrace H
            // WOTLK Instances Heroics
            {533, sConfigMgr->GetFloatDefault("Solocraft.StratholmeRaidH", 25.0) },                       // Naxxramas 25
            {574, sConfigMgr->GetFloatDefault("Solocraft.Valgarde70H", 5.0) },                            // Utgarde Keep H
            {575, sConfigMgr->GetFloatDefault("Solocraft.UtgardePinnacleH", 5.0) },                       // Utgarde Pinnacle H
            {576, sConfigMgr->GetFloatDefault("Solocraft.Nexus70H", 5.0) },                               // The Nexus H
            {578, sConfigMgr->GetFloatDefault("Solocraft.Nexus80H", 5.0) },                               // The Occulus H
            {595, sConfigMgr->GetFloatDefault("Solocraft.StratholmeCOTH", 5.0) },                         // The Culling of Stratholme H
            {599, sConfigMgr->GetFloatDefault("Solocraft.Ulduar70H", 5.0) },                              // Halls of Stone H
            {600, sConfigMgr->GetFloatDefault("Solocraft.DrakTheronKeepH", 5.0) },                        // Drak'Tharon Keep H
            {601, sConfigMgr->GetFloatDefault("Solocraft.Azjol_UppercityH", 5.0) },                       // Azjol-Nerub H
            {602, sConfigMgr->GetFloatDefault("Solocraft.Ulduar80H", 5.0) },                              // Halls of Lighting H
            {603, sConfigMgr->GetFloatDefault("Solocraft.UlduarRaidH", 25.0) },                           // Ulduar 25
            {604, sConfigMgr->GetFloatDefault("Solocraft.GunDrakH", 5.0) },                               // Gundrak H
            {608, sConfigMgr->GetFloatDefault("Solocraft.DalaranPrisonH", 5.0) },                         // Violet Hold H
            {615, sConfigMgr->GetFloatDefault("Solocraft.ChamberOfAspectsBlackH", 25.0) },                // The Obsidian Sanctum 25
            {616, sConfigMgr->GetFloatDefault("Solocraft.NexusRaidH", 25.0) },                            // The Eye of Eternity 25
            {619, sConfigMgr->GetFloatDefault("Solocraft.Azjol_LowerCityH", 5.0) },                       // Ahn'kahet: The Old Kingdom H
            {631, sConfigMgr->GetFloatDefault("Solocraft.IcecrownCitadelH", 25.0) },                      // Icecrown Citadel 25
            {632, sConfigMgr->GetFloatDefault("Solocraft.IcecrownCitadel5ManH", 5.0) },                   // The Forge of Souls
            {649, sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentRaidH", 25.0) },                 // Trial of the Crusader 25
            {650, sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentDungeonH", 5.0) },               // Trial of the Champion H
            {658, sConfigMgr->GetFloatDefault("Solocraft.QuarryOfTearsH", 5.0) },                         // Pit of Saron H
            {668, sConfigMgr->GetFloatDefault("Solocraft.HallsOfReflectionH", 5.0) },                     // Halls of Reflection H
            {724, sConfigMgr->GetFloatDefault("Solocraft.ChamberOfAspectsRedH", 25.0) },                  // The Ruby Sanctum 25
			// Universe Instances
			{736, sConfigMgr->GetFloatDefault("Solocraft.FirelandsH", 10.0) },                            // FirelandsH 10 S1
            {796, sConfigMgr->GetFloatDefault("Solocraft.HallsValorH", 0.0) },                           // Halls of Valor S7 HM 10 
            {797, sConfigMgr->GetFloatDefault("Solocraft.NightholdH", 0.0) },                            // Nighthold S8 HM 10 
        };
        //Unique Raids beyond the heroic and normal versions of themselves
        D649H10 = sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentRaidH10", 10.0);  //Trial of the Crusader 10 Heroic
        D649H25 = sConfigMgr->GetFloatDefault("Solocraft.ArgentTournamentRaidH25", 25.0);  //Trial of the Crusader 25 Heroic
    }
};
class SolocraftAnnounce : public PlayerScript
{
public:
    SolocraftAnnounce() : PlayerScript("SolocraftAnnounce") {}
    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        // Announce Module
        if (SoloCraftEnable)
        {
            if (SoloCraftAnnounceModule)
            {
                ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00SPP SoloCraft Syphrenacore Custom |rmodule.");
            }
        }
    }

    void OnLogout(Player* player) override
    {
        //Remove database entry as the player has logged out
        CharacterDatabase.PExecute("DELETE FROM custom_solocraft_character_stats WHERE GUID = {}", player->GetGUID());
    }
};
class solocraft_player_instance_handler : public PlayerScript {
public:
    solocraft_player_instance_handler() : PlayerScript("solocraft_player_instance_handler") {}
    void OnMapChanged(Player* player) override {
        if (sConfigMgr->GetBoolDefault("Solocraft.Enable", true))
        {
            Map* map = player->GetMap();
            float difficulty = CalculateDifficulty(map, player);
            int dunLevel = CalculateDungeonLevel(map, player);
            int numInGroup = GetNumInGroup(player);
            uint32 classBalance = GetClassBalance(player);
            ApplyBuffs(player, map, difficulty, dunLevel, numInGroup, classBalance);
        }
    }

private:
    std::map<uint32, float> _unitDifficulty;
    // Set the instance difficulty
    int CalculateDifficulty(Map* map, Player* /*player*/)
    {
        //float difficulty = 0.0;//changed from 1.0
        if (map)
        {
            //WOTLK 25 Man raids
            if (map->Is25ManRaid())
            {
                if (map->IsHeroic() && map->GetId() == 649) {
                    return D649H25;  //Heroic Grand Trial of the Crusader
                }
                else if (diff_Multiplier_Heroics.find(map->GetId()) == diff_Multiplier_Heroics.end())
                {
                    return D25; //map not found returns the catch all value
                }
                else
                    return diff_Multiplier_Heroics[map->GetId()]; //return the specific dungeon's level
            }
            if (map->IsHeroic())
            {
                //WOTLK 10 Man Heroic
                if (map->GetId() == 649)
                {
                    return D649H10;
                }
                else if (diff_Multiplier_Heroics.find(map->GetId()) == diff_Multiplier_Heroics.end())
                {
                    return D10; //map not found returns the catch all value
                }
                else
                    return diff_Multiplier_Heroics[map->GetId()]; //return the specific dungeon's level
            }
            if (diff_Multiplier.find(map->GetId()) == diff_Multiplier.end())
            {
                //Catch Alls  ----------------------5 Dungeons and 40 Raids
                if (map->IsDungeon())
                {
                    return D5;
                }
                else if (map->IsRaid())
                {
                    return D40;
                }
            }
            else
                return diff_Multiplier[map->GetId()]; //return the specific dungeon's level
        }
        return 0; //return 0
    }
    // Set the Dungeon Level
    int CalculateDungeonLevel(Map* map, Player* /*player*/)
    {
        if (dungeons.find(map->GetId()) == dungeons.end())
        {
            return SolocraftDungeonLevel; //map not found returns the catch all value
        }
        else
            return dungeons[map->GetId()]; //return the specific dungeon's level
    }
    // Get the group's size
    int GetNumInGroup(Player* player)
    {
        int numInGroup = 2;
        Group* group = player->GetGroup();
        if (group) {
            Group::MemberSlotList const& groupMembers = group->GetMemberSlots();
            numInGroup = groupMembers.size();
        }
        return numInGroup;
    }
    // Get the Player's class balance debuff
    uint32 GetClassBalance(Player* player)
    {
        uint32 classBalance = 100;

        if (classes.find(player->GetClass()) == classes.end())
        {
            return classBalance; //class not found returns the catch all value
        }
        else if (classes[player->GetClass()] <= 100)
        {
            return classes[player->GetClass()]; //return the specific class's Balance value
        }
        else
            return classBalance; //class balance value invalid returns the catch all value
    }
    // Apply the player buffs
    void ApplyBuffs(Player* player, Map* map, float difficulty, int dunLevel, int numInGroup, int classBalance)
    {
        int SpellPowerBonus = 0;
        //Check whether to buff the player or check to debuff back to normal
        if (difficulty != 0)
        {
            std::ostringstream ss;
            if (player->GetLevel() <= dunLevel + SolocraftLevelDiff) //If a player is too high level for dungeon don't buff but if in a group will count towards the group offset balancing.
            {
                //Get Current members total difficulty offset and if it exceeds the difficulty offset of the dungeon then debuff new group members coming in until all members leave and re-enter. This happens when a player already inside dungeon invite others to the group but the player already has the full difficulty offset.
                float GroupDifficulty = GetGroupDifficulty(player);
                //Check to either debuff or buff player entering dungeon.  Debuff must be enabled in Config
                if (GroupDifficulty >= difficulty && SoloCraftDebuffEnable == 1)
                {
                    //Current dungeon offset exceeded - Debuff player modified by ClassBalance Adjustment
                    difficulty = (-abs(difficulty)) + ((((float)classBalance / 100) * difficulty) / numInGroup);
                    difficulty = roundf(difficulty * 100) / 100; //Float variables suck

                    //sLog->outError("{}: would have this difficulty: %f", player->GetGUID(), tempDiff);
                }
                else
                {
                    //Current Dungeon offset not exceeded - Buff player
                    //Group difficulty and ClassBalance Adjustment
                    difficulty = (((float)classBalance / 100) * difficulty) / numInGroup;;
                    difficulty = roundf(difficulty * 100) / 100; //Float variables suck - two decimal rounding
                }

                //Check Database for a current dungeon entry
                QueryResult result = CharacterDatabase.PQuery("SELECT `GUID`, `Difficulty`, `GroupSize`, `SpellPower`, `Stats` FROM `custom_solocraft_character_stats` WHERE GUID = {}", player->GetGUID());

                //Modify Player Stats
                for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) //STATS defined/enum in SharedDefines.h
                {
                    if (result)
                    {
                        player->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, (*result)[1].GetFloat() * (*result)[4].GetFloat(), false);
                    }
                    // Buff the player
                    player->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, difficulty * SoloCraftStatsMult, true); //Unitmods enum UNIT_MOD_STAT_START defined in Unit.h line 391
                }
                // Set player health
                player->SetFullHealth();//defined in Unit.h line 1524
                //Spellcaster Stat modify
                if (player->GetPowerType() == POWER_MANA)
                {
                    // Buff the player's mana
                    player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

                    //Check for Dungeon to Dungeon Transfer and remove old Spellpower buff
                    if (result)
                    {
                        // remove spellpower bonus
                        player->ApplySpellPowerBonus((*result)[3].GetUInt32() * (*result)[4].GetFloat(), false);
                    }

                    //Buff Spellpower
                    if (difficulty > 0) //Debuffed characters do not get spellpower
                    {
                        SpellPowerBonus = static_cast<int>((player->GetLevel() * SoloCraftSpellMult) * difficulty);//Yes, I pulled this calc out of my butt.
                        player->ApplySpellPowerBonus(SpellPowerBonus, true);
                        //sLog->outError("%u: spellpower Bonus applied: %i", player->GetGUID(), SpellPowerBonus);
                    }
                }
                //Announcements
                if (difficulty > 0)
                {
                    // Announce to player - Buff
                    ss << "|cffFF0000[SoloCraft] |cffFF8000" << player->GetName() << " rentre dans une instance %s  - Compensation de la difficulter: %0.2f. Bonus puissance des sorts: %i. Equilibre entre les classes: %i";
                    ChatHandler(player->GetSession()).PSendSysMessage(ss.str().c_str(), map->GetMapName(), difficulty, SpellPowerBonus, classBalance);
                }
                else
                {
                    // Announce to player - Debuff
                    ss << "|cffFF0000[SoloCraft] |cffFF8000" << player->GetName() << " rentre dans une instance %s  - |cffFF0000BE CONSEILLE - Vous avez ete affaibli par une compensation de : %0.2f avec un equilibre entre les classes: %i. |cffFF8000 Un membre du groupe qui se trouve deja a l interieur beneficie de la compensation de buff complet du donjon..  Aucun buff de puissance ne sera appliquer aux lanceurs de sorts. TOUS les membres du groupe doivent quitter le donjon et le reintegrer pour recevoir une compensation equilibree.";
                    ChatHandler(player->GetSession()).PSendSysMessage(ss.str().c_str(), map->GetMapName(), difficulty, classBalance);
                }
                // Save Player Dungeon Offsets to Database
                CharacterDatabase.PExecute("REPLACE INTO custom_solocraft_character_stats (GUID, Difficulty, GroupSize, SpellPower, Stats) VALUES ({}, {}, {}, {}, {})", player->GetGUID(), difficulty, numInGroup, SpellPowerBonus, SoloCraftStatsMult);
            }
            else
            {
                // Announce to player - Over Max Level Threshold
                ss << "|cffFF0000[SoloCraft] |cffFF8000" << player->GetName() << " rentre dans une instance %s  - |cffFF0000Vous n avez pas recu de compensation de buff. |cffFF8000 Votre niveau est superieur au niveau maximum (%i) seuil pour ce donjon.";
                ChatHandler(player->GetSession()).PSendSysMessage(ss.str().c_str(), map->GetMapName(), dunLevel + SolocraftLevelDiff);
                ClearBuffs(player, map); //Check to revert player back to normal
            }

        }
        else
        {
            ClearBuffs(player, map); //Check to revert player back to normal - Moving this here fixed logout and login while in instance buff and debuff issues
        }
    }

    // Get the current group members GUIDS and return the total sum of the difficulty offset by all group members currently in the dungeon
    float GetGroupDifficulty(Player* player) {
        float GroupDifficulty = 0.0;
        Group* group = player->GetGroup();
        if (group)
        {
            Group::MemberSlotList const& groupMembers = group->GetMemberSlots();
            for (Group::member_citerator itr = groupMembers.begin(); itr != groupMembers.end(); ++itr)
            {
                //Exclude player from the tally because the player is the one entering the dungeon
                if (itr->guid != player->GetGUID())
                {
                    //Database query to find difficulty for each group member that is currently in an instance
                    QueryResult result = CharacterDatabase.PQuery("SELECT `GUID`, `Difficulty`, `GroupSize` FROM `custom_solocraft_character_stats` WHERE GUID = {}", itr->guid);
                    if (result)
                    {
                        //Test for debuffs already give to other members - They cannot be used to determine the total offset because negative numbers will skew the total difficulty offset
                        if ((*result)[1].GetFloat() > 0)
                        {
                            GroupDifficulty = GroupDifficulty + (*result)[1].GetFloat();
                            //sLog->outError("%u : Group member GUID in instance: %u", player->GetGUID(), itr->guid);
                        }
                    }
                }
            }
        }
        return GroupDifficulty;
    }

    void ClearBuffs(Player* player, Map* map)
    {
        //Database query to get offset from the last instance player exited
        QueryResult result = CharacterDatabase.PQuery("SELECT `GUID`, `Difficulty`, `GroupSize`, `SpellPower`, `Stats` FROM `custom_solocraft_character_stats` WHERE GUID = {}", player->GetGUID());
        if (result)
        {
            float difficulty = (*result)[1].GetFloat();
            int SpellPowerBonus = (*result)[3].GetUInt32();
            float StatsMultPct = (*result)[4].GetFloat();
            //sLog->outError("Map difficulty: %f", difficulty);
            // Inform the player
            std::ostringstream ss;
            ss << "|cffFF0000[SoloCraft] |cffFF8000" << player->GetName() << " Quitte une instance %s - Annulation de la compensation de la difficulter: %0.2f. Suppression du bonus de puissance des sorts: %i";
            ChatHandler(player->GetSession()).PSendSysMessage(ss.str().c_str(), map->GetMapName(), difficulty, SpellPowerBonus);
            // Clear the buffs
            for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
            {
                player->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, difficulty * StatsMultPct, false);
            }
            if (player->GetPowerType() == POWER_MANA && difficulty > 0)
            {
                // remove spellpower bonus
                player->ApplySpellPowerBonus(SpellPowerBonus, false);
            }
            //Remove database entry as the player is no longer in an instance
            CharacterDatabase.PExecute("DELETE FROM custom_solocraft_character_stats WHERE GUID = {}", player->GetGUID());
        }
    }
};

void AddSolocraftScripts()
{
    new SolocraftConfig();
    new SolocraftAnnounce();
    new solocraft_player_instance_handler();
}
