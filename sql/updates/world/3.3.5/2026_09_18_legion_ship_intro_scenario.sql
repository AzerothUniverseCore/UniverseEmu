-- creature_template
UPDATE `creature_template` SET `npcflag` = 3, `ScriptName` = 'npc_legion_ship_illidan'   WHERE `entry` = 7007719;
UPDATE `creature_template` SET `npcflag` = 3, `ScriptName` = 'npc_legion_ship_illidari'  WHERE `entry` = 1173924;
UPDATE `creature_template` SET `npcflag` = 3, `ScriptName` = 'npc_legion_ship_illidari'  WHERE `entry` = 1173927;

-- syphrena_string
DELETE FROM `syphrena_string` WHERE `entry` IN (900021, 900022);
INSERT INTO `syphrena_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
(900021, 'Guide me through the ship.', NULL, 'Guidez-moi a travers le vaisseau.', NULL, NULL, NULL, NULL, NULL, NULL),
(900022, '(You are already exploring the ship with another guide.)', NULL, '(Vous explorez deja le vaisseau avec un autre guide.)', NULL, NULL, NULL, NULL, NULL, NULL);

-- quest_template
DELETE FROM `quest_template` WHERE `ID` = 900020;
INSERT INTO `quest_template`
(`ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`, `QuestInfoID`, `SuggestedGroupNum`, `RequiredFactionId1`, `RequiredFactionId2`, `RequiredFactionValue1`, `RequiredFactionValue2`, `RewardNextQuest`, `RewardXPDifficulty`, `RewardMoney`, `RewardBonusMoney`, `RewardDisplaySpell`, `RewardSpell`, `RewardHonor`, `RewardKillHonor`, `StartItem`, `Flags`, `RequiredPlayerKills`, `RewardItem1`, `RewardAmount1`, `RewardItem2`, `RewardAmount2`, `RewardItem3`, `RewardAmount3`, `RewardItem4`, `RewardAmount4`, `ItemDrop1`, `ItemDropQuantity1`, `ItemDrop2`, `ItemDropQuantity2`, `ItemDrop3`, `ItemDropQuantity3`, `ItemDrop4`, `ItemDropQuantity4`, `RewardChoiceItemID1`, `RewardChoiceItemQuantity1`, `RewardChoiceItemID2`, `RewardChoiceItemQuantity2`, `RewardChoiceItemID3`, `RewardChoiceItemQuantity3`, `RewardChoiceItemID4`, `RewardChoiceItemQuantity4`, `RewardChoiceItemID5`, `RewardChoiceItemQuantity5`, `RewardChoiceItemID6`, `RewardChoiceItemQuantity6`, `POIContinent`, `POIx`, `POIy`, `POIPriority`, `RewardTitle`, `RewardTalents`, `RewardArenaPoints`, `RewardFactionID1`, `RewardFactionValue1`, `RewardFactionOverride1`, `RewardFactionID2`, `RewardFactionValue2`, `RewardFactionOverride2`, `RewardFactionID3`, `RewardFactionValue3`, `RewardFactionOverride3`, `RewardFactionID4`, `RewardFactionValue4`, `RewardFactionOverride4`, `RewardFactionID5`, `RewardFactionValue5`, `RewardFactionOverride5`, `TimeAllowed`, `AllowableRaces`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`, `RequiredNpcOrGo1`, `RequiredNpcOrGo2`, `RequiredNpcOrGo3`, `RequiredNpcOrGo4`, `RequiredNpcOrGoCount1`, `RequiredNpcOrGoCount2`, `RequiredNpcOrGoCount3`, `RequiredNpcOrGoCount4`, `RequiredItemId1`, `RequiredItemId2`, `RequiredItemId3`, `RequiredItemId4`, `RequiredItemId5`, `RequiredItemId6`, `RequiredItemCount1`, `RequiredItemCount2`, `RequiredItemCount3`, `RequiredItemCount4`, `RequiredItemCount5`, `RequiredItemCount6`, `Unknown0`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`, `VerifiedBuild`)
VALUES
(900020, 2, 80, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5000000, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0,
 0, 0,
 'A Vessel Torn from the Legion',
 'Illidan has welcomed you aboard the captured Legion ship. Find Illidari Starr or Illidari Kilbride and let them guide you through the vessel.',
 'This ship once carried the Burning Legion''s own soldiers across the Twisting Nether. Now it is ours - reclaimed, and crewed by the Illidari. Before you take up arms beyond these decks, walk them first. Illidari Starr and Illidari Kilbride both know this vessel intimately; approach either one and they will show you the way.',
 '',
 '',
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0,
 'Speak with Illidari Starr or Illidari Kilbride to be guided through the ship.',
 '', '', '', 0);

-- quest_template_locale
DELETE FROM `quest_template_locale` WHERE `ID` = 900020 AND `locale` = 'frFR';
INSERT INTO `quest_template_locale` (`ID`, `locale`, `Title`, `Details`, `Objectives`, `EndText`, `CompletedText`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`, `VerifiedBuild`)
VALUES
(900020, 'frFR',
 'Un vaisseau arraché à la Légion',
 'Ce vaisseau transportait autrefois les propres soldats de la Légion Ardente à travers le Néant Déchiqueté. Il est désormais nôtre, repris et tenu par les Illidari. Avant de reprendre les armes au-delà de ces ponts, parcourez-les d''abord. Illidari Starr et Illidari Kilbride connaissent tous deux ce vaisseau par cœur ; approchez l''un ou l''autre et il vous montrera le chemin.',
 'Parlez à Illidari Starr ou Illidari Kilbride pour qu''il vous guide à travers le vaisseau.',
 'Vous avez vu ce que la Légion a construit, et ce que nous en avons fait. Allez maintenant - il reste bien plus à faire qu''une simple visite de ce vaisseau.',
 'Ce vaisseau n''a plus de secret pour vous aujourd''hui. Ce qui l''attend au-delà est une toute autre affaire.',
 'Parlez à Illidari Starr ou Illidari Kilbride pour qu''il vous guide à travers le vaisseau.',
 '', '', '', 0);

-- quest_template_addon
DELETE FROM `quest_template_addon` WHERE `ID` = 900020;
INSERT INTO `quest_template_addon`
(`ID`, `MaxLevel`, `AllowableClasses`, `SourceSpellID`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`, `BreadcrumbForQuestId`, `RewardMailTemplateID`, `RewardMailDelay`, `RequiredSkillID`, `RequiredSkillPoints`, `RequiredMinRepFaction`, `RequiredMaxRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepValue`, `ProvidedItemCount`, `SpecialFlags`)
VALUES
(900020, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2); -- SpecialFlags 2 = EXPLORATION_OR_EVENT, needed for AreaExploredOrEventHappens() to complete it from script

-- quest_offer_reward
DELETE FROM `quest_offer_reward` WHERE `ID` = 900020;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`)
VALUES
(900020, 0, 0, 0, 0, 0, 0, 0, 0, 'You have seen what the Legion built, and what we have made of it. Go now - there is far more work ahead than a single tour of this ship.', 0);

-- quest_request_items
DELETE FROM `quest_request_items` WHERE `ID` = 900020;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`)
VALUES
(900020, 0, 0, 'The ship holds no more secrets for you today. What lies beyond it is another matter entirely.', 0);

-- creature_queststarter
DELETE FROM `creature_queststarter` WHERE `id` = 7007719 AND `quest` = 900020;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES (7007719, 900020);

-- creature_questender
DELETE FROM `creature_questender` WHERE `id` IN (1173924, 1173927) AND `quest` = 900020;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(1173924, 900020),
(1173927, 900020);

-- ----------------------------
-- creature_text (base enUS) - Illidan
-- ----------------------------
DELETE FROM `creature_text` WHERE `CreatureID` = 7007719;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(7007719, 0, 0, 'So. Another soul answers the call against the Legion. Welcome aboard, champion - this vessel is all that remains of an armada we tore from the Legion''s grasp. Seek out Illidari Starr or Illidari Kilbride; either will show you what we have won, and what still lies ahead.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Illidan greeting');

-- ----------------------------
-- creature_text (base enUS) - Illidari Starr
-- ----------------------------
DELETE FROM `creature_text` WHERE `CreatureID` = 1173924;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(1173924, 0, 0, 'Stay close. This ship has more ghosts than crew, and not all of them are friendly.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr start'),
(1173924, 1, 0, 'Careful here - the deck gave way when we took this ship. Brace yourself, this jump isn''t pleasant.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr pre-jump'),
(1173924, 2, 0, 'There. Still in one piece. Keep moving.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr post-jump'),
(1173924, 3, 0, 'Every corridor here once marched with Legion soldiers. Now it echoes with our footsteps instead.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr midway'),
(1173924, 4, 0, 'We''re close now. I can feel the Legionnaires of Jaedenar still lingering ahead - old wounds this ship hasn''t forgotten.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr near end'),
(1173924, 5, 0, 'This is as far as the tour goes. You''ve seen the ship - now go show the Legion what you''ve learned.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Starr end');

-- ----------------------------
-- creature_text (base enUS) - Illidari Kilbride
-- ----------------------------
DELETE FROM `creature_text` WHERE `CreatureID` = 1173927;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(1173927, 0, 0, 'Come on then. I''ll show you around - just don''t wander off, this hull still has surprises.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride start'),
(1173927, 1, 0, 'Mind your footing - this section nearly swallowed the ship when we boarded her. We jump from here.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride pre-jump'),
(1173927, 2, 0, 'Made it. Breathe if you need to, then follow.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride post-jump'),
(1173927, 3, 0, 'Hard to believe Legion boots ever marched these same halls. Feels almost quiet now.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride midway'),
(1173927, 4, 0, 'Almost there. Something about this stretch always makes my skin crawl - Jaedenar''s legionnaires left their mark here.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride near end'),
(1173927, 5, 0, 'That''s the tour done. The rest of the fight is out there, not in here.', 12, 0, 100, 0, 0, 0, 0, 0, 'LegionShipIntro - Kilbride end');

-- ----------------------------
-- creature_text_locale (frFR) 
-- ----------------------------
DELETE FROM `creature_text_locale` WHERE `CreatureID` IN (7007719, 1173924, 1173927) AND `Locale` = 'frFR';
INSERT INTO `creature_text_locale` (`CreatureID`, `GroupID`, `ID`, `Locale`, `Text`) VALUES
(7007719, 0, 0, 'frFR', 'Ainsi, une âme de plus répond à l''appel contre la Légion. Bienvenue à bord, champion. Ce vaisseau est tout ce qu''il nous reste d''une flotte arrachée aux griffes de la Légion. Trouvez Illidari Starr ou Illidari Kilbride ; l''un comme l''autre vous montrera ce que nous avons conquis, et ce qu''il nous reste à affronter.'),

(1173924, 0, 0, 'frFR', 'Restez près de moi. Ce vaisseau compte plus de fantômes que d''équipage, et tous ne sont pas amicaux.'),
(1173924, 1, 0, 'frFR', 'Attention ici - le pont s''est effondré quand nous avons pris ce vaisseau. Accrochez-vous, ce saut n''a rien d''agréable.'),
(1173924, 2, 0, 'frFR', 'Voilà. Toujours entier. On continue.'),
(1173924, 3, 0, 'frFR', 'Chaque couloir ici a autrefois vu défiler des soldats de la Légion. Maintenant, ce sont nos pas qui y résonnent.'),
(1173924, 4, 0, 'frFR', 'Nous approchons. Je sens encore la présence des légionnaires de Jaedenar plus loin - de vieilles blessures que ce vaisseau n''a pas oubliées.'),
(1173924, 5, 0, 'frFR', 'Le circuit s''arrête ici. Vous avez vu le vaisseau - allez maintenant montrer à la Légion ce que vous avez appris.'),

(1173927, 0, 0, 'frFR', 'Allons-y. Je vais vous faire visiter - ne vous éloignez pas, cette coque cache encore des surprises.'),
(1173927, 1, 0, 'frFR', 'Attention où vous marchez - cette section a presque englouti le vaisseau quand nous l''avons pris d''assaut. On saute d''ici.'),
(1173927, 2, 0, 'frFR', 'On y est. Reprenez votre souffle si besoin, puis suivez-moi.'),
(1173927, 3, 0, 'frFR', 'Difficile de croire que des bottes de la Légion ont un jour foulé ces mêmes couloirs. Ça semble presque calme, maintenant.'),
(1173927, 4, 0, 'frFR', 'On y est presque. Ce passage me donne toujours la chair de poule - les légionnaires de Jaedenar y ont laissé leur marque.'),
(1173927, 5, 0, 'frFR', 'Voilà, la visite est terminée. Le reste du combat se joue là-bas, pas ici.');
