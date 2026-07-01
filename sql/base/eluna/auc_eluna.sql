/*
SQLyog Community v13.3.0 (64 bit)
MySQL - 10.5.8-MariaDB : Database - auc_eluna
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`auc_eluna` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci */;

USE `auc_eluna`;

/*Table structure for table `account_parangon` */

DROP TABLE IF EXISTS `account_parangon`;

CREATE TABLE `account_parangon` (
  `account_id` int(11) NOT NULL,
  `level` int(11) DEFAULT 1,
  `exp` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `account_parangon` */

/*Table structure for table `account_parangonbase` */

DROP TABLE IF EXISTS `account_parangonbase`;

CREATE TABLE `account_parangonbase` (
  `account_id` int(11) NOT NULL,
  `level` int(11) DEFAULT 1,
  `exp` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `account_parangonbase` */

/*Table structure for table `account_parangonone` */

DROP TABLE IF EXISTS `account_parangonone`;

CREATE TABLE `account_parangonone` (
  `account_id` int(11) NOT NULL,
  `level` int(11) DEFAULT 1,
  `exp` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `account_parangonone` */

/*Table structure for table `account_parangontree` */

DROP TABLE IF EXISTS `account_parangontree`;

CREATE TABLE `account_parangontree` (
  `account_id` int(11) NOT NULL,
  `level` int(11) DEFAULT 1,
  `exp` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `account_parangontree` */

/*Table structure for table `account_parangontwo` */

DROP TABLE IF EXISTS `account_parangontwo`;

CREATE TABLE `account_parangontwo` (
  `account_id` int(11) NOT NULL,
  `level` int(11) DEFAULT 1,
  `exp` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `account_parangontwo` */

/*Table structure for table `characters_exp_rates` */

DROP TABLE IF EXISTS `characters_exp_rates`;

CREATE TABLE `characters_exp_rates` (
  `guid` int(10) NOT NULL,
  `mod_exp` int(2) NOT NULL DEFAULT 1,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_exp_rates` */

/*Table structure for table `characters_parangon` */

DROP TABLE IF EXISTS `characters_parangon`;

CREATE TABLE `characters_parangon` (
  `account_id` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `strength` int(11) DEFAULT 0,
  `agility` int(11) DEFAULT 0,
  `stamina` int(11) DEFAULT 0,
  `intellect` int(11) DEFAULT 0,
  `criticalhit` int(11) DEFAULT 0,
  `haste` int(11) DEFAULT 0,
  `spellpower` int(11) DEFAULT 0,
  `attackpower` int(11) DEFAULT 0,
  `dodge` int(11) DEFAULT 0,
  `defense` int(11) DEFAULT 0,
  `parry` int(11) DEFAULT 0,
  `healthregeneration` int(11) DEFAULT 0,
  `resistance` int(11) DEFAULT 0,
  `armor` int(11) DEFAULT 0,
  `resilience` int(11) DEFAULT 0,
  `hitrating` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`,`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_parangon` */

/*Table structure for table `characters_parangonbase` */

DROP TABLE IF EXISTS `characters_parangonbase`;

CREATE TABLE `characters_parangonbase` (
  `account_id` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `strength` int(11) DEFAULT 0,
  `agility` int(11) DEFAULT 0,
  `stamina` int(11) DEFAULT 0,
  `intellect` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`,`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_parangonbase` */

/*Table structure for table `characters_parangonone` */

DROP TABLE IF EXISTS `characters_parangonone`;

CREATE TABLE `characters_parangonone` (
  `account_id` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `strength` int(11) DEFAULT 0,
  `agility` int(11) DEFAULT 0,
  `stamina` int(11) DEFAULT 0,
  `intellect` int(11) DEFAULT 0,
  `spirit` int(11) DEFAULT 0,
  `armor` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`,`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_parangonone` */

/*Table structure for table `characters_parangontree` */

DROP TABLE IF EXISTS `characters_parangontree`;

CREATE TABLE `characters_parangontree` (
  `account_id` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `attackpower` int(11) DEFAULT 0,
  `spellpower` int(11) DEFAULT 0,
  `regen` int(11) DEFAULT 0,
  `resistance` int(11) DEFAULT 0,
  `resilience` int(11) DEFAULT 0,
  `hitrating` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`,`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_parangontree` */

/*Table structure for table `characters_parangontwo` */

DROP TABLE IF EXISTS `characters_parangontwo`;

CREATE TABLE `characters_parangontwo` (
  `account_id` int(11) NOT NULL,
  `guid` int(11) NOT NULL,
  `defense` int(11) DEFAULT 0,
  `dodge` int(11) DEFAULT 0,
  `parry` int(11) DEFAULT 0,
  `block` int(11) DEFAULT 0,
  `criticalstrike` int(11) DEFAULT 0,
  PRIMARY KEY (`account_id`,`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

/*Data for the table `characters_parangontwo` */

/*Table structure for table `index_spell_bonus_action` */

DROP TABLE IF EXISTS `index_spell_bonus_action`;

CREATE TABLE `index_spell_bonus_action` (
  `spell_id` int(10) NOT NULL,
  `overlay_texture` varchar(150) NOT NULL DEFAULT 'stormyellow-extrabutton',
  PRIMARY KEY (`spell_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

/*Data for the table `index_spell_bonus_action` */

insert  into `index_spell_bonus_action`(`spell_id`,`overlay_texture`) values 
(203394,'fire-extrabutton'),
(316907,'default'),
(338010,'ysera');

/*Table structure for table `index_spell_bonus_action_conditions` */

DROP TABLE IF EXISTS `index_spell_bonus_action_conditions`;

CREATE TABLE `index_spell_bonus_action_conditions` (
  `spell_id` int(11) NOT NULL,
  `condition_type` enum('aura','item','item_equipped','map_id','zone_id','area_id','active_event','min_level','class','race','phase_mask','quest_rewarded','quest_incomplete','min_hp_pct','max_hp_pct') NOT NULL,
  `condition_value` int(11) DEFAULT NULL,
  PRIMARY KEY (`spell_id`,`condition_type`) USING BTREE,
  CONSTRAINT `spell_id` FOREIGN KEY (`spell_id`) REFERENCES `index_spell_bonus_action` (`spell_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

/*Data for the table `index_spell_bonus_action_conditions` */

insert  into `index_spell_bonus_action_conditions`(`spell_id`,`condition_type`,`condition_value`) values 
(203394,'map_id',875),
(203394,'area_id',50533),
(316907,'item',316908),
(316907,'map_id',831),
(316907,'zone_id',5203),
(316907,'area_id',5203),
(316907,'class',6),
(338010,'item',338008),
(338010,'map_id',631),
(338010,'zone_id',4812),
(338010,'area_id',4812);

/*Table structure for table `mod_account_rank` */

DROP TABLE IF EXISTS `mod_account_rank`;

CREATE TABLE `mod_account_rank` (
  `accountID` int(11) NOT NULL,
  `rank` int(11) DEFAULT NULL,
  PRIMARY KEY (`accountID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_account_rank` */

insert  into `mod_account_rank`(`accountID`,`rank`) values 
(1,1);

/*Table structure for table `mod_account_ranks` */

DROP TABLE IF EXISTS `mod_account_ranks`;

CREATE TABLE `mod_account_ranks` (
  `account_id` int(11) NOT NULL,
  `acm` int(11) DEFAULT NULL,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_account_ranks` */

insert  into `mod_account_ranks`(`account_id`,`acm`) values 
(1,1);

/*Table structure for table `mod_item_visual_enchants` */

DROP TABLE IF EXISTS `mod_item_visual_enchants`;

CREATE TABLE `mod_item_visual_enchants` (
  `itemGUID` int(11) NOT NULL,
  `originDisplay` int(11) DEFAULT NULL,
  `newDisplay` int(11) DEFAULT NULL,
  PRIMARY KEY (`itemGUID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_item_visual_enchants` */

/*Table structure for table `mod_link_account_mascotte` */

DROP TABLE IF EXISTS `mod_link_account_mascotte`;

CREATE TABLE `mod_link_account_mascotte` (
  `accountID` int(11) NOT NULL,
  `commonMountID` int(11) NOT NULL,
  `allianceMountID` int(11) NOT NULL,
  `hordeMountID` int(11) NOT NULL,
  PRIMARY KEY (`accountID`,`commonMountID`,`allianceMountID`,`hordeMountID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_link_account_mascotte` */

/*Table structure for table `mod_link_account_mount` */

DROP TABLE IF EXISTS `mod_link_account_mount`;

CREATE TABLE `mod_link_account_mount` (
  `accountID` int(11) NOT NULL,
  `commonMountID` int(11) NOT NULL,
  `allianceMountID` int(11) NOT NULL,
  `hordeMountID` int(11) NOT NULL,
  PRIMARY KEY (`accountID`,`commonMountID`,`allianceMountID`,`hordeMountID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_link_account_mount` */

/*Table structure for table `mod_link_account_pet` */

DROP TABLE IF EXISTS `mod_link_account_pet`;

CREATE TABLE `mod_link_account_pet` (
  `accountID` int(11) NOT NULL,
  `petID` int(11) NOT NULL,
  PRIMARY KEY (`accountID`,`petID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_link_account_pet` */

/*Table structure for table `mod_me_shop_logs` */

DROP TABLE IF EXISTS `mod_me_shop_logs`;

CREATE TABLE `mod_me_shop_logs` (
  `pGUID` int(11) DEFAULT NULL,
  `date` varchar(100) DEFAULT NULL,
  `logs` varchar(255) DEFAULT NULL,
  `active` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_me_shop_logs` */

/*Table structure for table `mod_player_informations` */

DROP TABLE IF EXISTS `mod_player_informations`;

CREATE TABLE `mod_player_informations` (
  `pGUID` int(11) NOT NULL,
  `expRate` int(1) DEFAULT 1,
  `isSesame` int(1) DEFAULT 0,
  PRIMARY KEY (`pGUID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_player_informations` */

/*Table structure for table `mod_sesamestuff` */

DROP TABLE IF EXISTS `mod_sesamestuff`;

CREATE TABLE `mod_sesamestuff` (
  `classid` int(2) NOT NULL,
  `entry` int(11) NOT NULL,
  `count` int(3) NOT NULL DEFAULT 1,
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`classid`,`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_sesamestuff` */

insert  into `mod_sesamestuff`(`classid`,`entry`,`count`,`comment`) values 
(1,8000880,1,'Warrior'),
(1,8000881,1,'Warrior'),
(1,8000882,1,'Warrior'),
(1,8000989,1,'Warrior'),
(2,8000883,1,'Paladin'),
(2,8000884,1,'Paladin'),
(2,8000885,1,'Paladin'),
(2,8000950,1,'Paladin'),
(2,8000951,1,'Paladin'),
(3,8000886,1,'Hunter'),
(3,8000887,1,'Hunter'),
(3,8000888,1,'Hunter'),
(4,8000889,1,'Rogue'),
(4,8000890,1,'Rogue'),
(4,8000891,1,'Rogue'),
(5,8000892,1,'Priest'),
(5,8000893,1,'Priest'),
(5,8000894,1,'Priest'),
(6,8000911,1,'Death Knight'),
(6,8000912,1,'Death Knight'),
(6,8000913,1,'Death Knight'),
(6,8000914,1,'Death Knight'),
(7,8000895,1,'Shaman'),
(7,8000896,1,'Shaman'),
(7,8000897,1,'Shaman'),
(7,8000898,1,'Shaman'),
(7,8000899,1,'Shaman'),
(8,8000900,1,'Mage'),
(8,8000901,1,'Mage'),
(8,8000902,1,'Mage'),
(9,8000903,1,'Warlock'),
(9,8000904,1,'Warlock'),
(9,8000905,1,'Warlock'),
(10,8000870,1,'Unknown'),
(10,8000871,1,'Unknown'),
(10,8000872,1,'Unknown'),
(10,8000873,1,'Unknown'),
(11,8000906,1,'Druid'),
(11,8000907,1,'Druid'),
(11,8000908,1,'Druid'),
(11,8000909,1,'Druid'),
(11,8000910,1,'Druid'),
(13,8000789,1,'Unknown'),
(13,8000790,1,'Unknown'),
(13,8000791,1,'Unknown'),
(14,8000792,1,'Unknown'),
(14,8000793,1,'Unknown'),
(14,8000794,1,'Unknown'),
(16,8000940,1,'Unknown'),
(16,8000941,1,'Unknown'),
(16,8000942,1,'Unknown'),
(16,8000943,1,'Unknown');

/*Table structure for table `mod_sesamestufftbc` */

DROP TABLE IF EXISTS `mod_sesamestufftbc`;

CREATE TABLE `mod_sesamestufftbc` (
  `classid` int(2) NOT NULL,
  `entry` int(11) NOT NULL,
  `count` int(3) NOT NULL DEFAULT 1,
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`classid`,`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_sesamestufftbc` */

insert  into `mod_sesamestufftbc`(`classid`,`entry`,`count`,`comment`) values 
(1,800060,1,NULL),
(1,800061,1,NULL),
(1,800062,1,NULL),
(1,8000601,1,NULL),
(2,800054,1,NULL),
(2,800055,1,NULL),
(2,800056,1,NULL),
(2,8000540,1,NULL),
(3,800063,1,NULL),
(3,8000631,1,NULL),
(4,800048,1,NULL),
(4,800049,1,NULL),
(4,8000481,1,NULL),
(5,800051,1,NULL),
(5,8000511,1,NULL),
(7,800073,1,NULL),
(7,800074,1,NULL),
(7,8000731,1,NULL),
(8,800051,1,NULL),
(8,8000512,1,NULL),
(9,800051,1,NULL),
(9,8000513,1,NULL),
(11,800066,1,NULL),
(11,800067,1,NULL),
(11,8000661,1,NULL);

/*Table structure for table `mod_sesametrainer` */

DROP TABLE IF EXISTS `mod_sesametrainer`;

CREATE TABLE `mod_sesametrainer` (
  `classid` int(2) NOT NULL,
  `entry` int(11) NOT NULL,
  `count` int(3) NOT NULL DEFAULT 1,
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`classid`,`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_sesametrainer` */

insert  into `mod_sesametrainer`(`classid`,`entry`,`count`,`comment`) values 
(1,44452,1,NULL),
(2,44452,1,NULL),
(3,44452,1,NULL),
(4,44452,1,NULL),
(5,44452,1,NULL),
(6,44452,1,NULL),
(7,44452,1,NULL),
(8,44452,1,NULL),
(9,44452,1,NULL),
(11,44452,1,NULL);

/*Table structure for table `mod_sesametrainerfak` */

DROP TABLE IF EXISTS `mod_sesametrainerfak`;

CREATE TABLE `mod_sesametrainerfak` (
  `classid` int(2) NOT NULL,
  `entry` int(11) NOT NULL,
  `count` int(3) NOT NULL DEFAULT 1,
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`classid`,`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `mod_sesametrainerfak` */

/*Table structure for table `rebirth_accounts` */

DROP TABLE IF EXISTS `rebirth_accounts`;

CREATE TABLE `rebirth_accounts` (
  `account_id` int(10) NOT NULL,
  `RebirthLevel` int(10) NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

/*Data for the table `rebirth_accounts` */

/*Table structure for table `rebirth_characters` */

DROP TABLE IF EXISTS `rebirth_characters`;

CREATE TABLE `rebirth_characters` (
  `guid` int(10) NOT NULL,
  `account_id` int(10) NOT NULL,
  `RebirthLevel` int(10) NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

/*Data for the table `rebirth_characters` */

/*Table structure for table `sesame_welcome` */

DROP TABLE IF EXISTS `sesame_welcome`;

CREATE TABLE `sesame_welcome` (
  `account_id` int(10) NOT NULL,
  `SesameActive` int(10) NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

/*Data for the table `sesame_welcome` */

/* Procedure structure for procedure `SetComment` */

/*!50003 DROP PROCEDURE IF EXISTS  `SetComment` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `SetComment`()
BEGIN
    UPDATE `auc_eluna_github`.`mod_sesamestuff`
    SET comment = CASE classid
        WHEN 1  THEN 'Warrior'
        WHEN 2  THEN 'Paladin'
        WHEN 3  THEN 'Hunter'
        WHEN 4  THEN 'Rogue'
        WHEN 5  THEN 'Priest'
        WHEN 6  THEN 'Death Knight'
        WHEN 7  THEN 'Shaman'
        WHEN 8  THEN 'Mage'
        WHEN 9  THEN 'Warlock'
        WHEN 11 THEN 'Druid'
        ELSE 'Unknown'
    END;
END */$$
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
