/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *Original Script by Mindsear
 *Refactored by SPP Developer MDic
*/

#include "DatabaseEnv.h"
#include "Define.h"
#include "GossipDef.h"
#include "Item.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Config.h"
#include "WorldSession.h"

enum Vendors
{
    NPC_VENDOR_A    = 54,
    NPC_VENDOR_H    = 3163,
    NPC_AUCTION_H   = 9856,
    NPC_AUCTION_A   = 8670
};

enum Trainers
{
    // Alliance
    DRUID_A     = 5504,
    HUNTER_A    = 5515,
    MAGE_A      = 5497,
    PALADIN_A   = 928,
    PRIEST_A    = 376,
    ROGUE_A     = 918,
    SHAMAN_A    = 20407,
    WARLOCK_A   = 461,
    WARRIOR_A   = 5479,

    // Horde
    DRUID_H     = 3033,
    HUNTER_H    = 3406,
    MAGE_H      = 5883,
    PALADIN_H   = 23128,
    PRIEST_H    = 3045,
    ROGUE_H     = 3401,
    SHAMAN_H    = 3344,
    WARLOCK_H   = 3324,
    WARRIOR_H   = 3354,

    DEATHKNIGHT_AH = 28472,
    BLOODMAGE_AH = 300049,
    DEMON_HUNTER_AH = 150318
};

enum Mounts
{
    HUMAN_MOUNT               = 22717,
    ORC_MOUNT                 = 22724,
    GNOME_MOUNT               = 22719,
    NIGHTELF_MOUNT            = 22723,
    DWARF_MOUNT               = 22720,
    UNEAD_MOUNT               = 63643,
    TAUREN_MOUNT              = 22718,
    TROLL_MOUNT               = 22721,
    BLOODELF_MOUNT            = 65639,
    DRAENEI_MOUNT             = 48027,
	PANDAREN_HORDE_MOUNT	  = 150050,
	PANDAREN_ALLIANCE_MOUNT	  = 150050,
	BLOODELF_ILLIDARI_MOUNT	  = 142790,
	NIGHTELF_ILLIDARI_MOUNT	  = 142790,
	EREDAR_MOUNT	          = 150301,
	VOIDELF_MOUNT	          = 142789,
	VULPERA_MOUNT             = 142770,
	NIGHTBORNE_MOUNT          = 150300,
	LIGHTFORGED_DRAENEI_MOUNT = 150301
};

class premium_account : public ItemScript
{
public:
    premium_account() : ItemScript("premium_account") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!sConfigMgr->GetBoolDefault("PremiumAccount", true))
            return false;

        QueryResult result = CharacterDatabase.PQuery("SELECT AccountId FROM premium WHERE active = 1 AND AccountId = {}", player->GetSession()->GetAccountId());

        if (!result)
            return false;

        if (player->IsInCombat())
            return false;

        float rangeCheck = 10.0f;
        if (player->FindNearestCreature(NPC_AUCTION_A, rangeCheck) ||
            player->FindNearestCreature(NPC_AUCTION_H, rangeCheck) ||
            player->FindNearestCreature(NPC_VENDOR_A, rangeCheck) ||
            player->FindNearestCreature(NPC_VENDOR_H, rangeCheck) ||
            player->FindNearestCreature(ROGUE_A, rangeCheck) ||
            player->FindNearestCreature(WARRIOR_A, rangeCheck) ||
            player->FindNearestCreature(HUNTER_A, rangeCheck) ||
            player->FindNearestCreature(PRIEST_A, rangeCheck) ||
            player->FindNearestCreature(PALADIN_A, rangeCheck) ||
            player->FindNearestCreature(DRUID_A, rangeCheck) ||
            player->FindNearestCreature(SHAMAN_A, rangeCheck) ||
            player->FindNearestCreature(MAGE_A, rangeCheck) ||
            player->FindNearestCreature(WARLOCK_A, rangeCheck) ||
            player->FindNearestCreature(HUNTER_H, rangeCheck) ||
            player->FindNearestCreature(WARRIOR_H, rangeCheck) ||
            player->FindNearestCreature(SHAMAN_H, rangeCheck) ||
            player->FindNearestCreature(PALADIN_H, rangeCheck) ||
            player->FindNearestCreature(ROGUE_H, rangeCheck) ||
            player->FindNearestCreature(WARLOCK_H, rangeCheck) ||
            player->FindNearestCreature(MAGE_H, rangeCheck) ||
            player->FindNearestCreature(PRIEST_H, rangeCheck) ||
            player->FindNearestCreature(DRUID_H, rangeCheck) ||
            player->FindNearestCreature(DEATHKNIGHT_AH, rangeCheck) ||
			player->FindNearestCreature(BLOODMAGE_AH, rangeCheck) ||
			player->FindNearestCreature(DEMON_HUNTER_AH, rangeCheck))
            return false;

        ClearGossipMenuFor(player);

        if (sConfigMgr->GetBoolDefault("Morph", true))
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Changer d'apparence", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Rétablir l'apparence", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        }

        if (sConfigMgr->GetBoolDefault("Mount", true))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT_16, "Monture", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

        if (sConfigMgr->GetBoolDefault("Trainers", true))
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);

        if (sConfigMgr->GetBoolDefault("PlayerInteraction", true))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Options", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());

        return false; // Cast the spell on use normally
    }

    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) override
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1: /*Morph*/
            {
                CloseGossipMenuFor(player);
                ApplyRandomMorph(player);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 2: /*Demorph*/
            {
                player->DeMorph();
                CloseGossipMenuFor(player);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 3: /*Show Bank*/
            {
                player->GetSession()->SendShowBank(player->GetGUID());
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 4: /*Mail Box*/
            {
                player->GetSession()->SendShowMailBox(player->GetGUID());
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 5: /*Vendor*/
            {
                uint32 vendorId = 0;
                std::string salute;
                if (player->GetTeamId() == TEAM_ALLIANCE)
                {
                    vendorId = NPC_VENDOR_A;
                    salute = "Bonjour, je pars dans une minutes. Je dois retourner à Bowerstone.";
                } else {
                    vendorId = NPC_VENDOR_H;
                    salute = "Lok’tar, je pars dans une minutes. Je dois retourner à Bowerstone.";
                }

                SummonTempNPC(player, vendorId, salute.c_str());
                CloseGossipMenuFor(player);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 6: /*Mount*/
            {
                CloseGossipMenuFor(player);
                switch (player->GetRace())
                {
                    case RACE_HUMAN:                  player->CastSpell(player, HUMAN_MOUNT); break;
                    case RACE_ORC:                    player->CastSpell(player, ORC_MOUNT); break;
                    case RACE_GNOME:                  player->CastSpell(player, GNOME_MOUNT); break;
                    case RACE_NIGHTELF:               player->CastSpell(player, NIGHTELF_MOUNT); break;
                    case RACE_DWARF:                  player->CastSpell(player, DWARF_MOUNT); break;
                    case RACE_DRAENEI:                player->CastSpell(player, DRAENEI_MOUNT); break;
                    case RACE_UNDEAD_PLAYER:          player->CastSpell(player, UNEAD_MOUNT); break;
                    case RACE_TAUREN:                 player->CastSpell(player, TAUREN_MOUNT); break;
                    case RACE_TROLL:                  player->CastSpell(player, TROLL_MOUNT); break;
                    case RACE_BLOODELF:               player->CastSpell(player, BLOODELF_MOUNT); break;
					case RACE_PANDAREN_HORDE:         player->CastSpell(player, PANDAREN_HORDE_MOUNT); break;
					case RACE_PANDAREN_ALLIANCE:      player->CastSpell(player, PANDAREN_ALLIANCE_MOUNT); break;
					case RACE_BLOODELF_ILLIDARI:      player->CastSpell(player, BLOODELF_ILLIDARI_MOUNT); break;
					case RACE_NIGHTELF_ILLIDARI:      player->CastSpell(player, NIGHTELF_ILLIDARI_MOUNT); break;
					case RACE_EREDAR:                 player->CastSpell(player, EREDAR_MOUNT); break;
					case RACE_VOIDELF:                player->CastSpell(player, VOIDELF_MOUNT); break;
					case RACE_VULPERA:                player->CastSpell(player, VULPERA_MOUNT); break;
					case RACE_NIGHTBORNE:             player->CastSpell(player, NIGHTBORNE_MOUNT); break;
					case RACE_LIGHTFORGED_DRAENEI:    player->CastSpell(player, LIGHTFORGED_DRAENEI_MOUNT); break;
                }
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 7: /*Auction House*/
            {
                uint32 auctionId = 0;
                std::string salute;
                if (player->GetTeamId() == TEAM_HORDE)
                {
                    auctionId = NPC_AUCTION_H;
                    salute = "Lok’tar, je pars dans une minutes. Je dois retourner à Bowerstone.";
                }
                else
                {
                    auctionId = NPC_AUCTION_A;
                    salute = "Bonjour, je pars dans une minutes. Je dois retourner à Bowerstone.";
                }

                SummonTempNPC(player, auctionId, salute.c_str());
                CloseGossipMenuFor(player);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 8: /* Class Trainers*/
            {
                uint32 trainerId = 0;
                switch (player->GetClass())
                {
                    case CLASS_ROGUE:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? ROGUE_A : ROGUE_H;
                        break;
                    case CLASS_WARRIOR:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? WARRIOR_A : WARRIOR_H;
                        break;
                    case CLASS_PRIEST:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? PRIEST_A : PRIEST_H;
                        break;
                    case CLASS_MAGE:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? MAGE_A : MAGE_H;
                        break;
                    case CLASS_PALADIN:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? PALADIN_A : PALADIN_H;
                        break;
                    case CLASS_HUNTER:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? HUNTER_A : HUNTER_H;
                        break;
                    case CLASS_DRUID:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? DRUID_A : DRUID_H;
                        break;
                    case CLASS_SHAMAN:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? SHAMAN_A : SHAMAN_H;
                        break;
                    case CLASS_WARLOCK:
                        trainerId = player->GetTeamId() == TEAM_ALLIANCE ? WARLOCK_A : WARLOCK_H;
                        break;
                    case CLASS_DEATH_KNIGHT:
                        trainerId = DEATHKNIGHT_AH;
                        break;
					case CLASS_BLOODMAGE:
                        trainerId = BLOODMAGE_AH;
                        break;
					case CLASS_DEMON_HUNTER:
                        trainerId = DEMON_HUNTER_AH;
                        break;
                }

                SummonTempNPC(player, trainerId);
                CloseGossipMenuFor(player);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 9: /*Player Interactions*/
            {
                ClearGossipMenuFor(player);

                if (sConfigMgr->GetBoolDefault("Vendor", true))
                    AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Vendeur", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

                if (sConfigMgr->GetBoolDefault("MailBox", true))
                    AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Boite aux lettres", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

                if (sConfigMgr->GetBoolDefault("Bank", true))
                    AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Banque", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

                if (sConfigMgr->GetBoolDefault("Auction", true))
                    AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Hotel des ventes", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);

                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            }
        }
    }

    void ApplyRandomMorph(Player* player)
    {
        uint32 random = (urand(1, 26)); // Change this line when adding more morphs
        {
            switch (random)
            {
                case 1: player->SetDisplayId(10134); break;     // Troll Female                 'Orb of Deception'
                case 2: player->SetDisplayId(10135); break;     // Troll Male                   'Orb of Deception'
                case 3: player->SetDisplayId(10136); break;     // Tauren Male                  'Orb of Deception'
                case 4: player->SetDisplayId(10137); break;     // Human Male                   'Orb of Deception'
                case 5: player->SetDisplayId(10138); break;     // Human Female                 'Orb of Deception'
                case 6: player->SetDisplayId(10139); break;     // Orc Male                     'Orb of Deception'
                case 7: player->SetDisplayId(10140); break;     // Orc Female                   'Orb of Deception' 
                case 8: player->SetDisplayId(10141); break;     // Dwarf Male                   'Orb of Deception'
                case 9: player->SetDisplayId(10142); break;     // Dwarf Female                 'Orb of Deception' 
                case 10: player->SetDisplayId(10143); break;    // NightElf Male                'Orb of Deception'
                case 11: player->SetDisplayId(10144); break;    // NightElf Female              'Orb of Deception'
                case 12: player->SetDisplayId(10145); break;    // Undead Female                'Orb of Deception'
                case 13: player->SetDisplayId(10146); break;    // Undead Male                  'Orb of Deception'
                case 14: player->SetDisplayId(10147); break;    // Tauren Female                'Orb of Deception'
                case 15: player->SetDisplayId(10148); break;    // Gnome Male                   'Orb of Deception'
                case 16: player->SetDisplayId(10149); break;    // Gnome Female                 'Orb of Deception'
                case 17: player->SetDisplayId(4527); break;     // Thrall                       'Orgrimmar Boss'
                case 18: player->SetDisplayId(11657); break;    // Lady Sylvanas                'Undercity Boss'
                case 19: player->SetDisplayId(4307); break;     // Cairne Bloodhoof             'Thunderbluff Boss'
                case 20: player->SetDisplayId(17122); break;    // Lor'themar Theron            'Silvermoon City Boss'
                case 21: player->SetDisplayId(3597); break;     // King Magni Bronzebeard       'Ironforge Boss'
                case 22: player->SetDisplayId(5566); break;     // Highlord Bolvar Fordragon    'Stormwind Boss'
                case 23: player->SetDisplayId(7006); break;     // High Tinker Mekkatorque      'Gnomer Boss'
                case 24: player->SetDisplayId(7274); break;     // Tyrande Whisperwind          'Darnassus Boss'
                case 25: player->SetDisplayId(21976); break;    // Arthus Small                 'Arthus'
                case 26: player->SetDisplayId(24641); break;    // Arthus Ghost                 'Arthus Ghost'

                default:
                    break;
            }
        }
    }

    void SummonTempNPC(Player* player, uint32 entry, const char* salute = "")
    {
        if (!player || entry == 0)
            return;

        int npcDuration = sConfigMgr->GetIntDefault("Premium.NpcDuration", 60) * IN_MILLISECONDS;
        if (npcDuration <= 0) // Safeguard
            npcDuration = 60;

        Creature* npc = player->SummonCreature(entry, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60s);
        npc->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        npc->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, player->GetFollowAngle());
        npc->SetFaction(player->GetFaction());

        if (salute && !(salute[0] == '\0'))
            npc->Whisper(salute, LANG_UNIVERSAL, player, false);;
    }
};

void AddSC_premium_account()
{
    new premium_account();
}
