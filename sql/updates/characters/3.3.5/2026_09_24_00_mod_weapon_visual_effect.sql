-- mod_weapon_visual_effect
CREATE TABLE IF NOT EXISTS `mod_weapon_visual_effect` (
    `item_guid` INT(10) UNSIGNED NOT NULL,
    `enchant_visual_id` INT(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`item_guid`)
)
COMMENT = 'Visuel d''enchantement (PNJ 2000001) - stocke uniquement un ID visuel par objet, aucun effet de jeu'
DEFAULT CHARSET = utf8 ENGINE = InnoDB;
