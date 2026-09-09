-- quest_bypass_log

DROP TABLE IF EXISTS `quest_bypass_log`;
CREATE TABLE `quest_bypass_log` (
  `quest_id` INT UNSIGNED NOT NULL COMMENT 'ID de la quete concernee / affected quest id',
  `quest_title` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Titre de la quete au moment du bypass / quest title at bypass time',
  `missing_type` ENUM('creature','gameobject','item') NOT NULL COMMENT 'Type d objectif manquant / type of missing objective',
  `missing_entry` INT UNSIGNED NOT NULL COMMENT 'Entry (creature_template/gameobject_template/item_template) manquant ou jamais spawn / missing or never-spawned entry',
  `reason` ENUM('template_missing','no_spawn') NOT NULL COMMENT 'template_missing = le modele n existe pas ; no_spawn = le modele existe mais n est jamais place / template_missing = template does not exist; no_spawn = template exists but is never placed',
  `hit_count` INT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Nombre de fois que ce bypass a ete declenche / number of times this bypass fired',
  `first_seen` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Premiere occurrence / first occurrence',
  `last_seen` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Derniere occurrence / last occurrence',
  `last_player_guid` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'GUID (partie basse) du dernier joueur concerne / low-part GUID of the last affected player',
  `fixed` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'A mettre a 1 manuellement une fois la quete corrigee / manually set to 1 once the quest has been fixed',
  PRIMARY KEY (`quest_id`,`missing_type`,`missing_entry`),
  KEY `idx_qbl_fixed` (`fixed`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
