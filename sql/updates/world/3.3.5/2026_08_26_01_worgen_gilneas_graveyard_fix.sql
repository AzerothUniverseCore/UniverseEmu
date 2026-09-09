-- ============================================================
-- 修复: 狼人死亡后释放灵魂传送到西部荒野的问题
-- Fix: Worgen spirit release teleporting to Westfall instead of Gilneas
-- Correction: Les Worgens sont téléportés à Westfall au lieu de Gilneas lors de la libération de l'esprit
-- ============================================================
--
-- 作者 / Author / Auteur: leewheel
-- 日期 / Date: 2026-08-26
--
-- 问题描述 / Problem Description / Description du Problème:
--   狼人角色在吉尔尼斯城(map=789, zone=50668)死亡后释放灵魂，
--   灵魂被传送到西部荒野(Westfall, WorldSafeLocs ID=4)而不是留在吉尔尼斯。
--
--   Worgen characters dying in Gilneas (map=789, zone=50668) release spirit
--   to Westfall (WorldSafeLocs ID=4) instead of staying in Gilneas.
--
--   Les personnages Worgens mourant à Gilneas (map=789, zone=50668) libèrent
--   leur esprit à Westfall (WorldSafeLocs ID=4) au lieu de rester à Gilneas.
--
-- 根本原因 / Root Cause / Cause Racine:
--   1. graveyard_zone 表中缺少 GhostZone=50668 (Gilneas) 的记录
--   2. 当 GetClosestGraveyard 找不到 zone 关联的墓地时，
--      fallback 调用 GetDefaultGraveyard(ALLIANCE)
--   3. GetDefaultGraveyard 对联盟返回 Westfall (ID=4)
--
--   The graveyard_zone table has no record for GhostZone=50668 (Gilneas).
--   When GetClosestGraveyard finds no graveyard linked to the zone,
--   it falls back to GetDefaultGraveyard(ALLIANCE),
--   which returns Westfall (ID=4) for Alliance players.
--
--   La table graveyard_zone ne contient aucun enregistrement pour
--   GhostZone=50668 (Gilneas). Lorsque GetClosestGraveyard ne trouve pas
--   de cimetière lié à la zone, il utilise GetDefaultGraveyard(ALLIANCE),
--   qui renvoie Westfall (ID=4) pour les joueurs de l'Alliance.
--
-- 修复方案 / Fix Solution / Solution de Correction:
--   1. 在 WorldSafeLocs 中添加吉尔尼斯墓地坐标 (ID=19000, Continent=789)
--   2. 在 graveyard_zone 中关联 zone=50668 到该墓地
--   坐标位于狼人新手出生点 (-1451.53, 1403.35, 35.5561) 附近
--
--   1. Add Gilneas graveyard location to WorldSafeLocs (ID=19000, Continent=789)
--   2. Link zone=50668 to this graveyard in graveyard_zone
--   Coordinates near Worgen starting point (-1451.53, 1403.35, 35.5561)
--
--   1. Ajouter l'emplacement du cimetière de Gilneas dans WorldSafeLocs
--      (ID=19000, Continent=789)
--   2. Lier zone=50668 à ce cimetière dans graveyard_zone
--   Coordonnées proches du point de départ des Worgens
--   (-1451.53, 1403.35, 35.5561)
-- ============================================================

-- 步骤1: 添加 WorldSafeLocs 条目 / Step 1: Add WorldSafeLocs entry / Étape 1: Ajouter une entrée WorldSafeLocs
-- 坐标位于狼人新手出生点附近 / Coordinates near Worgen starting point / Coordonnées proches du point de départ
INSERT INTO dbcfiles.db_worldsafelocs_12340
    (ID, Continent, LocX, LocY, LocZ, AreaName_Lang_enUS, AreaName_Lang_Mask)
VALUES
    (19000, 789, -1451.53, 1403.35, 35.5561, 'Gilneas - Worgen Start Graveyard', 1);

-- 步骤2: 添加 graveyard_zone 关联 / Step 2: Add graveyard_zone link / Étape 2: Ajouter le lien graveyard_zone
-- zone=50668 (Gilneas), Faction=469 (Alliance) / zone=50668 (Gilneas), Faction=469 (Alliance)
INSERT INTO graveyard_zone (ID, GhostZone, Faction, Comment)
VALUES (19000, 50668, 469, 'Gilneas (Map 789) - Worgen Start Graveyard - Alliance');
