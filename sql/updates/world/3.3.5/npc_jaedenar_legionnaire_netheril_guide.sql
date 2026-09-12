DELETE FROM `creature_template` WHERE `entry` = 2000512;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(2000512, 0, 0, 0, 0, 0, 9129, 0, 0, 0, 'Jaedenar Legionnaire', 'Assauts de la Légion', NULL, 0, 80, 80, 0, 35, 1, 1.0e0, 1.0e0, 1.0e0, 0, 0, 2000, 0, 1.0e0, 1.0e0, 1, 0, 0, 0, 0, 7, 138936390, 0, 0, 0, 0, 0, 0, 0, '', 0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 0, 0, 1, 0, 0, 0, 'npc_jaedenar_legionnaire_netheril_guide', NULL, NULL);

DELETE FROM `creature_text` WHERE `CreatureID` = 2000512;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(2000512, 0, 0, 'Bienvenue, aventurier ! Je suis le Légionnaire de Jaedenar. Suivez-moi, je vais vous guider jusqu\'au camp de Netheril.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - depart vaisseau'),
(2000512, 1, 0, 'Ce n\'est plus très loin, continuez de me suivre.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - vaisseau 1'),
(2000512, 2, 0, 'Cette zone est prévue pour vous équiper avec du stuff d\'ilvl 245.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - vaisseau 2'),
(2000512, 3, 0, 'Elle vous permettra de vous préparer avant d\'affronter les donjons personnalisés d\'Azeroth Universe.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - vaisseau 3'),
(2000512, 4, 0, 'Vous devrez accomplir le haut fait de Netheril pour accéder au donjon personnalisé d\'Azeroth Universe.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - vaisseau 4'),
(2000512, 5, 0, 'Nous y voici. Restez près de moi, je vous téléporte au Camp 1 de Netheril !', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - teleportation vers Netheril'),
(2000512, 6, 0, 'Nous voici arrivés à Netheril.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - arrivee camp'),
(2000512, 7, 0, 'Suivez-moi, je vais vous faire visiter les lieux.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - intro visite'),
(2000512, 8, 0, 'Prenez vos quêtes ici, puis nous continuons notre chemin.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - quetes 1'),
(2000512, 9, 0, 'C\'est ici que se trouvent les créatures à vaincre pour obtenir des bons d\'équipement, à convertir ensuite pour vous équiper avant les donjons personnalisés d\'Azeroth Universe.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - zone creatures/bons d\'equipement'),
(2000512, 10, 0, 'Vous pouvez également récupérer des bons d\'équipement par ici.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - bons d\'equipement bis'),
(2000512, 11, 0, 'Pour accéder au premier donjon des Terres de Fyra, palier S0 d\'Azeroth Universe, vous devrez obtenir le haut fait de Netheril.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - haut fait'),
(2000512, 12, 0, 'En vainquant les créatures que je vous ai montrées, vous obtiendrez des bons d\'équipement. Ici, vous pourrez les convertir en bons d\'équipement supérieurs.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - conversion des bons'),
(2000512, 13, 0, 'Ici, vous pourrez acheter vos recettes.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - vente de recettes'),
(2000512, 14, 0, 'Et voici, côté Alliance, de quoi vous équiper également grâce à vos bons d\'équipement.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - equipement Alliance'),
(2000512, 15, 0, 'Prenez vos quêtes, puis continuons la visite.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - quetes 2'),
(2000512, 16, 0, 'En participant aux champs de bataille avec les bots, vous gagnerez des points d\'honneur, échangeables contre des montures ou de l\'équipement PvP.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - PvP/points d\'honneur'),
(2000512, 17, 0, 'De nouvelles quêtes vous attendent ici, puis nous continuons.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - quetes 3'),
(2000512, 18, 0, 'Récupérez vos dernières quêtes ici. Notre visite de Netheril touche à sa fin. Bonne chance dans votre aventure, aventurier. Au revoir !', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Netheril - au revoir/fin');
