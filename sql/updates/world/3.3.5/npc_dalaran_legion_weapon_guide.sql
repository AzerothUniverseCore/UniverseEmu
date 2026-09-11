SET
@Entry = 90100,
@Name  = "Guide de Dalaran Legion";

INSERT INTO `creature_template` (`entry`, `modelid1`, `modelid2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES
(@Entry, 19638, 0, @Name, "Guide de voyage", NULL, 0, 80, 80, 0, 35, 1, 1, 0, 0, 2000, 0, 1, 0, 7, 138936390, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 0, 'npc_dalaran_legion_weapon_guide');

DELETE FROM `creature_text` WHERE `CreatureID` = @Entry;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `Comment`) VALUES
(@Entry, 0, 0, 'Allez, en route pour la Salle des Ombres ! C\'est là, dans les entrailles de Dalaran, que ton arme prodigieuse pourra être améliorée.', 12, 0, 100, 0, 0, 0, 0, 'Guide - depart'),
(@Entry, 1, 0, 'Nous y voila. L''amelioration de ton arme prodigieuse est juste devant nous.', 12, 0, 100, 0, 0, 0, 0, 'Guide - arrivee'),
(@Entry, 2, 0, 'Pour ameliorer tes armes prodigieuses, tu auras besoin de Cristaux d''Infusion.', 12, 0, 100, 0, 0, 0, 0, 'Guide - cristaux d''infusion'),
(@Entry, 3, 0, 'Chaque amelioration coute 14 000 Cristaux d''Infusion, autant te dire qu''il va falloir en recolter !', 12, 0, 100, 0, 0, 0, 0, 'Guide - cout de l''amelioration'),
(@Entry, 4, 0, 'Rien de plus simple pour en trouver : il y en a partout dans le monde, chaque creature en laisse 10.', 12, 0, 100, 0, 0, 0, 0, 'Guide - farm des cristaux'),
(@Entry, 5, 0, 'Dans notre capitale, un gnome organise un event d''arene de creatures : un excellent moyen d''en recolter en grande quantite.', 12, 0, 100, 0, 0, 0, 0, 'Guide - event arene du gnome');
