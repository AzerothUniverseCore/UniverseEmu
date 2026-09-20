-- Antorus, the Burning Throne (map 737)

UPDATE `creature_template` SET `ScriptName` = 'npc_antorus_argus_the_unmaker' WHERE `entry` = 124828; -- Argus l'Annihilateur

UPDATE `creature_template` SET `ScriptName` = 'npc_antorus_pantheon_guardian' WHERE `entry` IN
(
    125885, -- Aman'Thul
    126268, -- Golganneth
    125893, -- "Aggramar" (spirit)
    126266, -- Norgannon
    125886, -- Khaz'goroth
    126267  -- Eonar
);
