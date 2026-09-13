-- creature_template
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(624190, 0, 0, 0, 0, 0, 41667, 0, 0, 0, 'Aysa Cloudsinger', 'Monk Trainer', NULL, 0, 80, 80, 0, 35, 1, 1.0e0, 1.0e0, 1.0e0, 0, 0, 2000, 0, 1.0e0, 1.0e0, 1, 0, 0, 0, 0, 7, 138936390, 0, 0, 0, 0, 0, 0, 0, '', 0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 1.0e0, 0, 0, 1, 0, 0, 0, 'npc_aysa_cloudsinger_nerozias_guide', NULL, NULL);

-- creature_text : (enUS)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(624190, 0, 0, 'Hello, adventurer! I am Aysa Cloudsinger, monk trainer. Follow me.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - depart vaisseau'),
(624190, 1, 0, 'Prodigious weapons are weapons reclaimed from the Legion: these are weapons you\'ll need to keep with you throughout your progression.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - vaisseau explication armes'),
(624190, 2, 0, 'Here we are. Stay close to me, I\'m teleporting you to Nerozias!', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - teleportation vers Nerozias'),
(624190, 3, 0, 'There are merchants here, feel free to take a look around.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - vendeurs'),
(624190, 4, 0, 'Prodigious weapons have several tiers: from A0 to A8, then AM+0 to AM+8, and finally the last tier, AM+FULL. Safe to say there\'s a long way to go!', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - paliers des armes'),
(624190, 5, 0, 'Pick up your quest here.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - quete'),
(624190, 6, 0, 'Follow me, I\'m teleporting you to the Emerald Dream Way!', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - teleportation vers Chemin du Reve d\'Emeraude'),
(624190, 7, 0, 'Here we are, we\'ve arrived at the Emerald Dream Way.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - arrivee Chemin du Reve d\'Emeraude'),
(624190, 8, 0, 'We\'re now heading to the Monastery of the Pandashan dungeon.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - direction donjon'),
(624190, 9, 0, 'We\'re almost there!', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - presque arrive'),
(624190, 10, 0, 'Make sure you go all the way to the end of the dungeon, your quest awaits you there.', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - aller au bout du donjon'),
(624190, 11, 0, 'Well then, our tour is coming to an end. Good luck in the dungeon, adventurer. Farewell!', 12, 0, 100.0e0, 0, 0, 0, 0, 0, 'Guide Nerozias - au revoir/fin');

-- creature_text_locale : (frFR)
INSERT INTO `creature_text_locale` (`CreatureID`, `GroupID`, `ID`, `Locale`, `Text`) VALUES
(624190, 0, 0, 'frFR', 'Bonjour, aventurier ! Je suis Aysa Cloudsinger, formatrice moine. Suivez-moi.'),
(624190, 1, 0, 'frFR', 'Les armes prodigieuses sont des armes reprises à la Légion : ce sont des armes que vous devrez conserver tout au long de votre progression.'),
(624190, 2, 0, 'frFR', 'Nous y voici. Restez près de moi, je vous téléporte jusqu\'à Nerozias !'),
(624190, 3, 0, 'frFR', 'Il y a des marchands ici, n\'hésitez pas à faire un tour.'),
(624190, 4, 0, 'frFR', 'Les armes prodigieuses possèdent plusieurs paliers : de A0 à A8, puis AM+0 à AM+8, et enfin le dernier palier, AM+FULL. Autant dire qu\'il y a du chemin à parcourir !'),
(624190, 5, 0, 'frFR', 'Prenez votre quête ici.'),
(624190, 6, 0, 'frFR', 'Suivez-moi, je vous téléporte jusqu\'au Chemin du Rêve d\'Émeraude !'),
(624190, 7, 0, 'frFR', 'Nous voici arrivés au Chemin du Rêve d\'Émeraude.'),
(624190, 8, 0, 'frFR', 'Nous nous rendons à présent au donjon du Monastère des Pandashan.'),
(624190, 9, 0, 'frFR', 'On y est presque !'),
(624190, 10, 0, 'frFR', 'Allez bien jusqu\'au bout du donjon, votre quête vous y attend.'),
(624190, 11, 0, 'frFR', 'Voilà, notre visite touche à sa fin. Bonne chance dans le donjon, aventurier. Au revoir !');
