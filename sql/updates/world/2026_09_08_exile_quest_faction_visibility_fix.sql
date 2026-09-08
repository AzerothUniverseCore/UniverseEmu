-- ============================================================================
-- 2026_09_08 离岛任务阵营可见性修复 (Exile's Reach quest faction visibility fix)
-- Fix: la visibilite des points d'exclamation par faction
--
-- 现象 (Symptom / Symptome):
--   联盟 NPC 156280 (Lady Jaina Proudmoore) 与 部落 NPC 166573 (Thrall)
--   挂载的离岛镜像任务 quest_template.AllowableRaces = 0,
--   导致双方阵营玩家都能看到对方 NPC 头顶的任务感叹号。
--   Quest template AllowableRaces = 0 -> les deux factions voient les points
--   d'exclamation de chaque PNJ.
--
-- 根因 (Root cause / Cause racine):
--   GetQuestDialogStatus() -> CanSeeStartQuest() -> SatisfyQuestRace() 按
--   AllowableRaces & Player::GetRaceMask() 过滤感叹号; 全库该列为 0 时不过滤。
--   AllowableRaces = 0 signifie "toutes races", donc aucun filtre de faction.
--
-- 修复 (Fix / Correctif):
--   联盟镜像任务(156280): AllowableRaces = RACEMASK_ALLIANCE
--   部落镜像任务(166573): AllowableRaces = RACEMASK_HORDE
--   数值由 src/server/shared/SharedDefines.h 的 RACEMASK_ALLIANCE/HORDE 宏
--   按 1 << (RACE-1) 精确展开, 由脚本计算, 与 Player::GetRaceMask() 同源。
--   Valeurs calculees depuis SharedDefines.h (RACEMASK_* macros).
--
--   RACEMASK_ALLIANCE = 1431481421 (0x5552AC4D) :
--     Human, Dwarf, NightElf, Gnome, Draenei, Worgen, Pandaren(Alliance),
--     NightElf Illidari, Lightforged Draenei, VoidElf, DarkIron Dwarf,
--     HighElf, Vulpera(Alliance), Dracthyr(Alliance), Kultiran
--   RACEMASK_HORDE    =  716002226 (0x2AAD53B2) :
--     Orc, Undead, Tauren, Troll, Goblin, BloodElf, Pandaren(Horde),
--     BloodElf Illidari, Eredar, Vulpera, Nightborne, Zandalari Troll,
--     DarkIron(Horde), Highmountain Tauren, Dracthyr(Horde), Mag'har Orc
-- ============================================================================

-- 联盟镜像任务 (Alliance mirror quests, NPC 156280 Jaina Proudmoore)
UPDATE `quest_template` SET `AllowableRaces` = 1431481421
WHERE `ID` IN (55122, 55174, 55988, 55992, 56344, 56775, 58208, 58209);

-- 部落镜像任务 (Horde mirror quests, NPC 166573 Thrall)
UPDATE `quest_template` SET `AllowableRaces` = 716002226
WHERE `ID` IN (59926, 59927, 59928, 59929, 59932, 59975, 59979, 59984);
