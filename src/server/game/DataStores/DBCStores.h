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

#ifndef SYPHRENA_DBCSTORES_H
#define SYPHRENA_DBCSTORES_H

#include "DBCStore.h"
#include "DBCStructure.h"
#include "SharedDefines.h"
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>

enum LocaleConstant : uint8;

 // temporary hack until includes are sorted out (don't want to pull in Windows.h)
#ifdef GetClassName
#undef GetClassName
#endif

typedef std::list<uint32> SimpleFactionsList;
SC_GAME_API SimpleFactionsList const* GetFactionTeamList(uint32 faction);

SC_GAME_API char const* GetPetName(uint32 petfamily, uint32 dbclang);
SC_GAME_API uint32 GetTalentSpellCost(uint32 spellId);
SC_GAME_API TalentSpellPos const* GetTalentSpellPos(uint32 spellId);

SC_GAME_API char const* GetRaceName(uint8 race, uint8 locale);
SC_GAME_API char const* GetClassName(uint8 class_, uint8 locale);

SC_GAME_API WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);

SC_GAME_API uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);

enum ContentLevels : uint8
{
    CONTENT_1_60 = 0,
    CONTENT_61_70,
    CONTENT_71_80
};
SC_GAME_API ContentLevels GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId);

SC_GAME_API bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);

SC_GAME_API void Zone2MapCoordinates(float &x, float &y, uint32 zone);
SC_GAME_API void Map2ZoneCoordinates(float &x, float &y, uint32 zone);

typedef std::map<uint32/*pair32(map, diff)*/, MapDifficulty> MapDifficultyMap;
SC_GAME_API MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
SC_GAME_API MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);

SC_GAME_API uint32 const* /*[MAX_TALENT_TABS]*/ GetTalentTabPages(uint8 cls);

SC_GAME_API uint32 GetLiquidFlags(uint32 liquidType);

SC_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level);
SC_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id);

SC_GAME_API CharacterFacialHairStylesEntry const* GetCharFacialHairEntry(uint8 race, uint8 gender, uint8 facialHairID);
SC_GAME_API CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color);
SC_GAME_API CharStartOutfitEntry const* GetCharStartOutfitEntry(uint8 race, uint8 class_, uint8 gender);

SC_GAME_API LFGDungeonEntry const* GetLFGDungeon(uint32 mapId, Difficulty difficulty);

SC_GAME_API uint32 GetDefaultMapLight(uint32 mapId);

typedef std::unordered_multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::iterator, SkillRaceClassInfoMap::iterator> SkillRaceClassInfoBounds;
SC_GAME_API std::vector<SkillLineAbilityEntry const*> const* GetSkillLineAbilitiesBySkill(uint32 skill);
SC_GAME_API SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_);

SC_GAME_API ResponseCodes ValidateName(std::wstring const& name, LocaleConstant locale);

SC_GAME_API EmotesTextSoundEntry const* FindTextSoundEmoteFor(uint32 emote, uint32 race, uint32 gender);

SC_GAME_API extern DBCStorage <AchievementEntry>             sAchievementStore;
SC_GAME_API extern DBCStorage <AchievementCriteriaEntry>     sAchievementCriteriaStore;
SC_GAME_API extern DBCStorage <AreaTableEntry>               sAreaTableStore;
SC_GAME_API extern DBCStorage <AreaGroupEntry>               sAreaGroupStore;
SC_GAME_API extern DBCStorage <AreaPOIEntry>                 sAreaPOIStore;
SC_GAME_API extern DBCStorage <AreaTriggerEntry>             sAreaTriggerStore;
SC_GAME_API extern DBCStorage <AuctionHouseEntry>            sAuctionHouseStore;
SC_GAME_API extern DBCStorage <BankBagSlotPricesEntry>       sBankBagSlotPricesStore;
SC_GAME_API extern DBCStorage <BannedAddOnsEntry>            sBannedAddOnsStore;
SC_GAME_API extern DBCStorage <BarberShopStyleEntry>         sBarberShopStyleStore;
SC_GAME_API extern DBCStorage <BattlemasterListEntry>        sBattlemasterListStore;
SC_GAME_API extern DBCStorage <ChatChannelsEntry>            sChatChannelsStore;
SC_GAME_API extern DBCStorage <CharacterFacialHairStylesEntry> sCharacterFacialHairStylesStore;
SC_GAME_API extern DBCStorage <CharSectionsEntry>            sCharSectionsStore;
SC_GAME_API extern DBCStorage <CharStartOutfitEntry>         sCharStartOutfitStore;
SC_GAME_API extern DBCStorage <CharTitlesEntry>              sCharTitlesStore;
SC_GAME_API extern DBCStorage <ChrClassesEntry>              sChrClassesStore;
SC_GAME_API extern DBCStorage <ChrRacesEntry>                sChrRacesStore;
SC_GAME_API extern DBCStorage <CinematicCameraEntry>         sCinematicCameraStore;
SC_GAME_API extern DBCStorage <CinematicSequencesEntry>      sCinematicSequencesStore;
SC_GAME_API extern DBCStorage <CreatureDisplayInfoEntry>     sCreatureDisplayInfoStore;
SC_GAME_API extern DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
SC_GAME_API extern DBCStorage <CreatureFamilyEntry>          sCreatureFamilyStore;
SC_GAME_API extern DBCStorage <CreatureModelDataEntry>       sCreatureModelDataStore;
SC_GAME_API extern DBCStorage <CreatureSpellDataEntry>       sCreatureSpellDataStore;
SC_GAME_API extern DBCStorage <CreatureTypeEntry>            sCreatureTypeStore;
SC_GAME_API extern DBCStorage <CurrencyTypesEntry>           sCurrencyTypesStore;
SC_GAME_API extern DBCStorage <DestructibleModelDataEntry>   sDestructibleModelDataStore;
SC_GAME_API extern DBCStorage <DungeonEncounterEntry>        sDungeonEncounterStore;
SC_GAME_API extern DBCStorage <DurabilityCostsEntry>         sDurabilityCostsStore;
SC_GAME_API extern DBCStorage <DurabilityQualityEntry>       sDurabilityQualityStore;
SC_GAME_API extern DBCStorage <EmotesEntry>                  sEmotesStore;
SC_GAME_API extern DBCStorage <EmotesTextEntry>              sEmotesTextStore;
SC_GAME_API extern DBCStorage <EmotesTextSoundEntry>         sEmotesTextSoundStore;
SC_GAME_API extern DBCStorage <FactionEntry>                 sFactionStore;
SC_GAME_API extern DBCStorage <FactionTemplateEntry>         sFactionTemplateStore;
SC_GAME_API extern DBCStorage <GameObjectArtKitEntry>        sGameObjectArtKitStore;
SC_GAME_API extern DBCStorage <GameObjectDisplayInfoEntry>   sGameObjectDisplayInfoStore;
SC_GAME_API extern DBCStorage <GemPropertiesEntry>           sGemPropertiesStore;
SC_GAME_API extern DBCStorage <GlyphPropertiesEntry>         sGlyphPropertiesStore;
SC_GAME_API extern DBCStorage <GlyphSlotEntry>               sGlyphSlotStore;

SC_GAME_API extern DBCStorage <GtBarberShopCostBaseEntry>    sGtBarberShopCostBaseStore;
SC_GAME_API extern DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore;
SC_GAME_API extern DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
SC_GAME_API extern DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore;
SC_GAME_API extern DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
SC_GAME_API extern DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore;
SC_GAME_API extern DBCStorage <GtNPCManaCostScalerEntry>     sGtNPCManaCostScalerStore;
SC_GAME_API extern DBCStorage <GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
SC_GAME_API extern DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore;
//SC_GAME_API extern DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore; -- not used currently
SC_GAME_API extern DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore;
SC_GAME_API extern DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore;
SC_GAME_API extern DBCStorage <HolidaysEntry>                sHolidaysStore;
SC_GAME_API extern DBCStorage <ItemEntry>                    sItemStore;
SC_GAME_API extern DBCStorage <ItemBagFamilyEntry>           sItemBagFamilyStore;
SC_GAME_API extern DBCStorage <ItemDisplayInfoEntry>         sItemDisplayInfoStore;
SC_GAME_API extern DBCStorage <ItemExtendedCostEntry>        sItemExtendedCostStore;
SC_GAME_API extern DBCStorage <ItemLimitCategoryEntry>       sItemLimitCategoryStore;
SC_GAME_API extern DBCStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore;
SC_GAME_API extern DBCStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore;
SC_GAME_API extern DBCStorage <ItemSetEntry>                 sItemSetStore;
SC_GAME_API extern DBCStorage <LFGDungeonEntry>              sLFGDungeonStore;
SC_GAME_API extern DBCStorage <LightEntry>                   sLightStore;
SC_GAME_API extern DBCStorage <LiquidTypeEntry>              sLiquidTypeStore;
SC_GAME_API extern DBCStorage <LockEntry>                    sLockStore;
SC_GAME_API extern DBCStorage <MailTemplateEntry>            sMailTemplateStore;
SC_GAME_API extern DBCStorage <MapEntry>                     sMapStore;
//SC_GAME_API extern DBCStorage <MapDifficultyEntry>           sMapDifficultyStore; -- use GetMapDifficultyData insteed
SC_GAME_API extern MapDifficultyMap                          sMapDifficultyMap;
SC_GAME_API extern DBCStorage <MovieEntry>                   sMovieStore;
SC_GAME_API extern DBCStorage <OverrideSpellDataEntry>       sOverrideSpellDataStore;
SC_GAME_API extern DBCStorage <PowerDisplayEntry>            sPowerDisplayStore;
SC_GAME_API extern DBCStorage <QuestSortEntry>               sQuestSortStore;
SC_GAME_API extern DBCStorage <QuestXPEntry>                 sQuestXPStore;
SC_GAME_API extern DBCStorage <QuestFactionRewEntry>         sQuestFactionRewardStore;
SC_GAME_API extern DBCStorage <RandPropPointsEntry>          sRandPropPointsStore;
SC_GAME_API extern DBCStorage <ScalingStatDistributionEntry> sScalingStatDistributionStore;
SC_GAME_API extern DBCStorage <ScalingStatValuesEntry>       sScalingStatValuesStore;
SC_GAME_API extern DBCStorage <SkillLineEntry>               sSkillLineStore;
SC_GAME_API extern DBCStorage <SkillLineAbilityEntry>        sSkillLineAbilityStore;
SC_GAME_API extern DBCStorage <SkillTiersEntry>              sSkillTiersStore;
SC_GAME_API extern DBCStorage <SoundEntriesEntry>            sSoundEntriesStore;
SC_GAME_API extern DBCStorage <SpellCastTimesEntry>          sSpellCastTimesStore;
SC_GAME_API extern DBCStorage <SpellCategoryEntry>           sSpellCategoryStore;
SC_GAME_API extern DBCStorage <SpellDifficultyEntry>         sSpellDifficultyStore;
SC_GAME_API extern DBCStorage <SpellDurationEntry>           sSpellDurationStore;
SC_GAME_API extern DBCStorage <SpellFocusObjectEntry>        sSpellFocusObjectStore;
SC_GAME_API extern DBCStorage <SpellItemEnchantmentEntry>    sSpellItemEnchantmentStore;
SC_GAME_API extern DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore;
SC_GAME_API extern PetFamilySpellsStore                      sPetFamilySpellsStore;
SC_GAME_API extern std::unordered_set<uint32>                sPetTalentSpells;
SC_GAME_API extern DBCStorage <SpellRadiusEntry>             sSpellRadiusStore;
SC_GAME_API extern DBCStorage <SpellRangeEntry>              sSpellRangeStore;
SC_GAME_API extern DBCStorage <SpellRuneCostEntry>           sSpellRuneCostStore;
SC_GAME_API extern DBCStorage <SpellShapeshiftFormEntry>     sSpellShapeshiftFormStore;
SC_GAME_API extern DBCStorage <SpellEntry>                   sSpellStore;
SC_GAME_API extern DBCStorage <SpellVisualEntry>             sSpellVisualStore;
SC_GAME_API extern DBCStorage <StableSlotPricesEntry>        sStableSlotPricesStore;
SC_GAME_API extern DBCStorage <SummonPropertiesEntry>        sSummonPropertiesStore;
SC_GAME_API extern DBCStorage <TalentEntry>                  sTalentStore;
SC_GAME_API extern DBCStorage <TalentTabEntry>               sTalentTabStore;
SC_GAME_API extern DBCStorage <TaxiNodesEntry>               sTaxiNodesStore;
SC_GAME_API extern DBCStorage <TaxiPathEntry>                sTaxiPathStore;
SC_GAME_API extern TaxiMask                                  sTaxiNodesMask;
SC_GAME_API extern TaxiMask                                  sOldContinentsNodesMask;
SC_GAME_API extern TaxiMask                                  sHordeTaxiNodesMask;
SC_GAME_API extern TaxiMask                                  sAllianceTaxiNodesMask;
SC_GAME_API extern TaxiMask                                  sDeathKnightTaxiNodesMask;
SC_GAME_API extern TaxiPathSetBySource                       sTaxiPathSetBySource;
SC_GAME_API extern TaxiPathNodesByPath                       sTaxiPathNodesByPath;
SC_GAME_API extern DBCStorage <TransportAnimationEntry>      sTransportAnimationStore;
SC_GAME_API extern DBCStorage <TransportRotationEntry>       sTransportRotationStore;
SC_GAME_API extern DBCStorage <TeamContributionPointsEntry>  sTeamContributionPointsStore;
SC_GAME_API extern DBCStorage <TotemCategoryEntry>           sTotemCategoryStore;
SC_GAME_API extern DBCStorage <VehicleEntry>                 sVehicleStore;
SC_GAME_API extern DBCStorage <VehicleSeatEntry>             sVehicleSeatStore;
SC_GAME_API extern DBCStorage <WMOAreaTableEntry>            sWMOAreaTableStore;
//SC_GAME_API extern DBCStorage <WorldMapAreaEntry>           sWorldMapAreaStore; -- use Zone2MapCoordinates and Map2ZoneCoordinates
SC_GAME_API extern DBCStorage <WorldMapOverlayEntry>         sWorldMapOverlayStore;
SC_GAME_API extern DBCStorage <WorldSafeLocsEntry>           sWorldSafeLocsStore;

SC_GAME_API void LoadDBCStores(const std::string& dataPath);

#endif
