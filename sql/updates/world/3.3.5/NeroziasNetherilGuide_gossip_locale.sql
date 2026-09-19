-- npc_aysa_cloudsinger_nerozias_guide
DELETE FROM `syphrena_string` WHERE `entry` IN (900025, 900026);
INSERT INTO `syphrena_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
(900025, 'Take me to visit Nerozias and the Dreamway.', NULL, 'Emmenez-moi visiter Nerozias et le Chemin du Reve d''Emeraude.', NULL, NULL, NULL, NULL, NULL, NULL),
(900026, '(You are already visiting Nerozias with another guide.)', NULL, '(Vous visitez deja Nerozias avec un autre guide.)', NULL, NULL, NULL, NULL, NULL, NULL);

-- npc_jaedenar_legionnaire_netheril_guide
DELETE FROM `syphrena_string` WHERE `entry` IN (900027, 900028);
INSERT INTO `syphrena_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
(900027, 'Take me to visit the Netheril camp.', NULL, 'Emmenez-moi visiter le camp de Netheril.', NULL, NULL, NULL, NULL, NULL, NULL),
(900028, '(You are already visiting the Netheril camp with another guide.)', NULL, '(Vous visitez deja le camp de Netheril avec un autre guide.)', NULL, NULL, NULL, NULL, NULL, NULL);
