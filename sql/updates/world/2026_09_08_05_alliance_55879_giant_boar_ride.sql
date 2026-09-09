-- By leewheel 2026-09-08
-- ============================================================================
-- Isle of Reach (map 859): Alliance quest 55879 "Ride of the Scientifically
-- Enhanced Boar" has NO ride vehicle anywhere on the island.
--
-- Root cause (data audit on 2026-09-08):
--   * Map 859 only hosts two "Choppy Booster Mk. 5" vehicles:
--       167027 (VehicleId 6843, guid 8000022 @ 108,-2414): flying ride used by
--           the scouting quest (Alliance 55193 "The Scout-o-Matic 5000" / the
--           Horde 59940 use the same vehicle class).
--       167142 (VehicleId 123, guid 14507499 @ 119,-2424): the Horde 59942
--           "The Re-Deather" ride.
--   * The Horde kill-quest rides Vehicle 123, the classic WotLK mount-style
--     vehicle (Havenshire Stallion 28605 / Bronze Drake 57107 / Ban-lu 142225
--     all use it), and gets its "Throw Dynamite" (29579) passenger action-bar
--     spell from creature_template_spell - so kills are cast BY the rider and
--     credit the quest. The Alliance chain (55879) is supposed to ride a GIANT
--     BOAR, but no boar-like vehicle template OR spawn exists anywhere in the
--     whole database -> quest is uncompletable for Alliance.
--
-- Fix (additive, new content entry 167150 - verified free everywhere):
--   * creature_template 167150 "Giant Boar": clone of the proven 167142 ride
--     recipe (VehicleId 123, npcflag spellclick, faction 35 friendly) but with
--     the isle-native boar model (display 52652, same family as 153238 Tamed
--     Boar) + unit_flags 0x800 (IMMUNE_TO_NPC) so the Alliance ground assault
--     is not swarmed by the ~300 undead; AIName empty -> static spawn never
--     despawns (the Horde 167142 despawns on passenger-removed, which would
--     break re-rides; Alliance keeps a persistent mount).
--   * npc_spellclick_spells: 65403 "Ride Vehicle" + 29579 (mirror 167142).
--   * creature_template_spell: Index 0 -> 29579 (vehicle action-bar dynamite,
--     cast BY the player so every kill credits quest 55879 objectives).
--   * creature spawn on map 859 at Austin's checkpoint-3 ride area.
--
-- Quest data reminder:
--   55879 (Alliance) / 59942 (Horde) share the objectives
--     Zombie Servant 156532 x100, Monstrous Cadaver 157091 x6, Torgok 162817
--     x1 then "Find Wrathion" 167128 (already spawned @ 230,-2300).
--
-- 中文：离岛(859)联盟任务 55879「骑上科学强化野猪」在全岛找不到任何骑乘载具。
-- 全库审核确认：859 图只有两架 Choppy Booster——167027(侦察飞行车)与
-- 167142(部落 59942 冲锋车)。联盟本应骑一只「巨型野猪」，但整库既无野猪
-- 载具模板也无刷新点，任务对联盟不可完成。
-- 修复(叠加新增，新条目 167150，已验证全局空闲)：
--   * 克隆已验证的 167142 冲锋车配方：VehicleId 123(骑乘式载具，骑手跨坐
--     背上)、spellclick 65403 上车、faction 35 全友好；
--   * 换成离岛原生野猪模型 52652(与 153238 驯服野猪同族)，放大 3 倍成巨兽；
--   * unit_flags 0x800 以免地面冲阵被约 300 只亡灵围殴(注：该位后被
--     2026_09_08_07 更正为 0x200 IMMUNE_TO_NPC)；
--   * 动作条法术 Index 0 -> 29579 扔炸药，由玩家自己施放，击杀全部计入任务。
--
-- FR : Sur l'ile 859, la quete alliance 55879 « chevaucher le sanglier
-- renforce » ne dispose d'aucune monture. L'audit de la base montre deux
-- Choppy Booster seulement : 167027 (reconnaissance volante) et 167142
-- (charge horde 59942). L'Alliance est censee chevaucher un « sanglier
-- geant », mais aucun template ni spawn de sanglier vehicule n'existe : la
-- quete est impossible. Correctif additif (nouvelle entree 167150) : recette
-- clonee de 167142 (VehicleId 123, spellclick 65403, faction 35), modele de
-- sanglier natif 52652 en taille x3, et sort de barre d'action 29579 lance par
-- le joueur afin que chaque victime soit creditee a la quete.
-- ============================================================================

-- ---- guard: make the update re-runnable after a fresh base import ----
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 167150;
DELETE FROM `npc_spellclick_spells`     WHERE `npc_entry` = 167150;
DELETE FROM `creature`                  WHERE `id` = 167150;
DELETE FROM `creature_template`         WHERE `entry` = 167150;

-- ---- 1) ride vehicle template (clone of 167142 recipe, boar visual) ----
INSERT INTO `creature_template` VALUES
(167150,0,0,0,0,0,52652,0,0,0,'Giant Boar',NULL,'vehichlecursor',0,1,1,0,35,16777216,1,1,1,0,0,2000,2000,1,1,1,2048,0,0,0,9,525312,0,0,0,0,123,0,0,'',0,1,0.3,1,1,1,1,0,0,1,0,0,0,'',NULL,0);

-- ---- 2) spellclick: board (65403) + click behaviour (mirror 167142) ----
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(167150, 65403, 1, 0),
(167150, 29579, 1, 0);

-- ---- 3) passenger action-bar ability (dynamite, cast by the rider) ----
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(167150, 0, 29579, 0);

-- ---- 4) static spawn next to Austin's checkpoint 3 (ride area) ----
-- NOTE: guid MUST be explicit and < 0xFFFFFF (16777216): this fork's
-- ObjectMgr::GenerateCreatureSpawnId() (TCE00007) caps creature spawn ids at
-- 0xFFFFFF. The table's AUTO_INCREMENT counter is already inflated past that
-- cap (312924242), so NEVER insert a creature row without an explicit guid.
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`ScriptName`,`StringId`,`VerifiedBuild`,`size`) VALUES
(14507796,167150,859,0,0,1,1,0,0,99.500,-2422.50,90.400,0.900,30,0,0,100,0,0,0,0,0,'',NULL,0,3.0);
