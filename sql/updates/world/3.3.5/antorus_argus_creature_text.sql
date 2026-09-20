DELETE FROM `creature_text` WHERE `CreatureID` IN (124828, 125885, 125886, 125893, 126266, 126267, 126268);
DELETE FROM `creature_text_locale` WHERE `CreatureID` IN (124828, 125885, 125886, 125893, 126266, 126267, 126268) AND `Locale` = 'frFR';

-- enUS (base text, creature_text)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(124828, 0, 0, 'The Pantheon\'s light will not save you here!', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Argus l\'Annihilateur - Engage'),
(124828, 1, 0, 'Im...possible...', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Argus l\'Annihilateur - Death'),
(124828, 2, 0, 'Flee if you must - the Void does not forget!', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Argus l\'Annihilateur - Retreat (throne swap)'),
(125885, 0, 0, 'Rest, my children - order shall endure a while longer.', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Aman\'Thul - Retreat (throne swap)'),
(125886, 0, 0, 'Even the forge must cool before it burns anew.', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Khaz\'goroth - Retreat (throne swap)'),
(125893, 0, 0, 'A champion never truly retreats - I merely regroup!', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Aggramar - Retreat (throne swap)'),
(126266, 0, 0, 'Wisdom dictates a tactical withdrawal, nothing more.', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Norgannon - Retreat (throne swap)'),
(126267, 0, 0, 'I withdraw to heal - but the cycle of life continues.', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Eonar - Retreat (throne swap)'),
(126268, 0, 0, 'The storm will return - rest now, but not for long!', 14, 0, 100.0e0, 0, 0, 0, 0, 0, 'Golganneth - Retreat (throne swap)');

-- frFR (creature_text_locale)
INSERT INTO `creature_text_locale` (`CreatureID`, `GroupID`, `ID`, `Locale`, `Text`) VALUES
(124828, 0, 0, 'frFR', 'La lumière du Panthéon ne vous sauvera pas ici !'),
(124828, 1, 0, 'frFR', 'Im...possible...'),
(124828, 2, 0, 'frFR', 'Fuyez si vous le pouvez - le Néant n\'oublie jamais !'),
(125885, 0, 0, 'frFR', 'Reposez-vous, mes enfants - l\'ordre perdurera encore un peu.'),
(125886, 0, 0, 'frFR', 'Même la forge doit refroidir avant de brûler à nouveau.'),
(125893, 0, 0, 'frFR', 'Un champion ne recule jamais vraiment - je me regroupe, simplement !'),
(126266, 0, 0, 'frFR', 'La sagesse dicte un retrait tactique, rien de plus.'),
(126267, 0, 0, 'frFR', 'Je me retire pour guérir - mais le cycle de la vie continue.'),
(126268, 0, 0, 'frFR', 'La tempête reviendra - reposez-vous, mais pas pour longtemps !');
