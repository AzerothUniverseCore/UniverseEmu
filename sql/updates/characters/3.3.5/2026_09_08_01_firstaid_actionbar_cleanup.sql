-- 2026_09_08 急救动作条图标清理
-- First Aid action-bar icon cleanup / Nettoyage des icônes de Premiers soins
-- By leewheel 2026-09-08
--
-- 问题: 新建角色首次登录时,核心沿急救法术链(3273→…→45542)递归学习,
--       客户端收到学习封包后自动把多个急救图标摆进动作条空位,并回报入库,
--       导致 character_action 堆积大量急救图标。
-- 本更新: 把既有角色的动作条规整为"急救图标至多一个(45542),位于第一技能条末格(button=11)"。
-- 与 FirstAid.cpp(2026-09-08) 登录规整逻辑保持一致; 角色下次登录时脚本也会自动再次规整。

-- 1) 删除所有非末格(button<>11)的急救图标(覆盖任意 spec / 任意技能条)
DELETE FROM character_action
WHERE action IN (3273, 3274, 7924, 10846, 27028, 45542)
  AND button <> 11;

-- 2) 末格(button=11)若为急救低级等级法术,且角色已掌握急救(45542) → 统一替换为 45542
UPDATE character_action ca
INNER JOIN character_spell cs ON cs.guid = ca.guid AND cs.spell = 45542 AND cs.active = 1
SET ca.action = 45542
WHERE ca.button = 11
  AND ca.type = 0
  AND ca.action IN (3273, 3274, 7924, 10846, 27028);
