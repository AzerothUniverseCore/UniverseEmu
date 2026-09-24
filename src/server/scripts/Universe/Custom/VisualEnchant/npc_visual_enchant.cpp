/*
 * This file is part of the UniverseEmu Project. See AUTHORS file for Copyright information
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
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CreatureAI.h"
#include "Player.h"
#include "Item.h"
#include "Chat.h"
#include "ScriptedGossip.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Common.h"
#include "WorldSession.h"

#include <unordered_map>
#include <string>

namespace VisualEnchant
{
    inline char const* L(Player* player, char const* fr, char const* en)
    {
        if (player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_frFR)
            return fr;
        return en;
    }

    enum VisualWeaponGossip
    {
        VIS_DEFAULT_MESSAGE         = 907,
        VIS_GOSSIP_MAIN_MENU_ACTION = 100,
        VIS_GOSSIP_MAIN_HAND_ACTION = 200,
        VIS_GOSSIP_OFF_HAND_ACTION  = 300,
        VIS_GOSSIP_CLOSE_ACTION     = 400
    };

    struct VisualData
    {
        uint32          Menu;
        uint32          Submenu;
        GossipOptionIcon Icon;
        uint32          Id;
        char const*     NameFR;
        char const*     NameEN;
    };

    VisualData const vData[] =
    {
        { 1, VIS_GOSSIP_MAIN_MENU_ACTION, GOSSIP_ICON_TALK,       0, "Retour..",   "Back.." },
        { 1, 2,                           GOSSIP_ICON_INTERACT_1, 0, "Suivant..",  "Next.." },
        { 1, 0, GOSSIP_ICON_BATTLE, 3789, "Berserker",                          "Berserking" },
        { 1, 0, GOSSIP_ICON_BATTLE, 3854, "Puissance des sorts",                "Spell Power" },
        { 1, 0, GOSSIP_ICON_BATTLE, 3273, "Givre mortel",                       "Deathfrost" },
        { 1, 0, GOSSIP_ICON_BATTLE, 3225, "Bourreau",                           "Executioner" },
        { 1, 0, GOSSIP_ICON_BATTLE, 3870, "Ponction de sang",                   "Blood Draining" },
        { 1, 0, GOSSIP_ICON_BATTLE, 1899, "Arme impie",                         "Unholy Weapon" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2674, "Eruption de sort",                   "Spellsurge" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2675, "Maitre de guerre",                   "Battlemaster" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2671, "Puissance des sorts (Arcane/Feu)",   "Arcane and Fire Spell Power" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2672, "Puissance des sorts (Ombre/Givre)",  "Shadow and Frost Spell Power" },
        { 1, 0, GOSSIP_ICON_BATTLE, 3365, "Rune de fracas d'epee",              "Rune of Swordshattering" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2673, "Mangouste",                          "Mongoose" },
        { 1, 0, GOSSIP_ICON_BATTLE, 2343, "Puissance des sorts",                "Spell Power" },

        { 2, VIS_GOSSIP_MAIN_MENU_ACTION, GOSSIP_ICON_TALK,       0, "Retour..",     "Back.." },
        { 2, 3,                           GOSSIP_ICON_INTERACT_1, 0, "Suivant..",    "Next.." },
        { 2, 1,                           GOSSIP_ICON_INTERACT_1, 0, "Precedent..",  "Previous.." },
        { 2, 0, GOSSIP_ICON_BATTLE, 425,  "Mannequin du Temple Noir",           "Black Temple Dummy" },
        { 2, 0, GOSSIP_ICON_BATTLE, 3855, "Puissance des sorts III",            "Spell Power III" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1894, "Arme de givre",                      "Icy Weapon" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1103, "Agilite",                            "Agility" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1898, "Vampirique",                         "Lifestealing" },
        { 2, 0, GOSSIP_ICON_BATTLE, 3345, "Vie terrestre I",                    "Earthliving I" },
        { 2, 0, GOSSIP_ICON_BATTLE, 3093, "Puissance d'attaque (Mort-vivants/Demons)", "Attack Power vs Undead and Demons" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1900, "Croise",                             "Crusader" },
        { 2, 0, GOSSIP_ICON_BATTLE, 3846, "Puissance des sorts II",             "Spell Power II" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1606, "Puissance d'attaque",                "Attack Power" },
        { 2, 0, GOSSIP_ICON_BATTLE, 283,  "Vent violent I",                     "Windfury I" },
        { 2, 0, GOSSIP_ICON_BATTLE, 1,    "Croque-roc III",                     "Rockbiter III" },

        { 3, VIS_GOSSIP_MAIN_MENU_ACTION, GOSSIP_ICON_TALK,       0, "Retour..",     "Back.." },
        { 3, 2,                           GOSSIP_ICON_INTERACT_1, 0, "Precedent..",  "Previous.." },
        { 3, 0, GOSSIP_ICON_BATTLE, 3265, "Enduit d'arme beni",                 "Blessed Weapon Coating" },
        { 3, 0, GOSSIP_ICON_BATTLE, 2,    "Arme de givre I",                    "Frostbrand I" },
        { 3, 0, GOSSIP_ICON_BATTLE, 3,    "Langue de feu III",                  "Flametongue III" },
        { 3, 0, GOSSIP_ICON_BATTLE, 3266, "Enduit d'arme vertueux",             "Righteous Weapon Coating" },
        { 3, 0, GOSSIP_ICON_BATTLE, 1903, "Esprit",                             "Spirit" },
        { 3, 0, GOSSIP_ICON_BATTLE, 13,   "Aiguise",                            "Sharpened" },
        { 3, 0, GOSSIP_ICON_BATTLE, 26,   "Huile de givre",                     "Frost Oil" },
        { 3, 0, GOSSIP_ICON_BATTLE, 7,    "Poison mortel",                      "Deadly Poison" },
        { 3, 0, GOSSIP_ICON_BATTLE, 803,  "Arme flamboyante",                   "Fiery Weapon" },
        { 3, 0, GOSSIP_ICON_BATTLE, 1896, "Degats d'arme",                      "Weapon Damage" },
        { 3, 0, GOSSIP_ICON_BATTLE, 2666, "Intellect",                          "Intellect" },
        { 3, 0, GOSSIP_ICON_BATTLE, 25,   "Huile d'ombre",                      "Shadow Oil" },
    };

    uint32 const VisualDataCount = sizeof(vData) / sizeof(vData[0]);

    class npc_visual_enchant : public CreatureScript
    {
    public:
        npc_visual_enchant() : CreatureScript("npc_visual_enchant") { }

        struct npc_visual_enchantAI : public ScriptedAI
        {
            npc_visual_enchantAI(Creature* creature) : ScriptedAI(creature) { }

            std::unordered_map<ObjectGuid, bool> m_mainHandChoice;

            bool IsMainHand(Player* player) const
            {
                auto itr = m_mainHandChoice.find(player->GetGUID());
                return itr == m_mainHandChoice.end() || itr->second;
            }

            void SetVisual(Player* player, uint32 visualId) const
            {
                uint8 slot = IsMainHand(player) ? EQUIPMENT_SLOT_MAINHAND : EQUIPMENT_SLOT_OFFHAND;
                Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

                if (!item)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        L(player,
                            "Tu n'as pas d'arme equipee a cet emplacement.",
                            "No weapon equipped in that slot."));
                    return;
                }

                ItemTemplate const* itemTemplate = item->GetTemplate();
                if (itemTemplate->Class != ITEM_CLASS_WEAPON)
                    return;

                switch (itemTemplate->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    case ITEM_SUBCLASS_WEAPON_SPEAR:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    case ITEM_SUBCLASS_WEAPON_WAND:
                    case ITEM_SUBCLASS_WEAPON_FISHING_POLE:
                        return;
                    default:
                        break;
                }

                player->SetUInt16Value(PLAYER_VISIBLE_ITEM_1_ENCHANTMENT + (item->GetSlot() * 2), 0, visualId);

                CharacterDatabase.PExecute(
                    "REPLACE INTO `mod_weapon_visual_effect` (`item_guid`, `enchant_visual_id`) VALUES ({}, {})",
                    item->GetGUID().GetCounter(), visualId);
            }

            void GetMenu(Player* player, uint32 menuId) const
            {
                for (uint32 i = 0; i < VisualDataCount; ++i)
                {
                    if (vData[i].Menu == menuId)
                        AddGossipItemFor(player, vData[i].Icon,
                            L(player, vData[i].NameFR, vData[i].NameEN),
                            GOSSIP_SENDER_MAIN, i);
                }

                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            }

            void GetMainMenu(Player* player) const
            {
                std::string mainHandLabel = std::string("|TInterface/PaperDoll/UI-PaperDoll-Slot-MainHand:40:40:-18|t")
                    + L(player, "Main droite", "Main-Hand");
                std::string offHandLabel = std::string("|TInterface/PaperDoll/UI-PaperDoll-Slot-SecondaryHand:40:40:-18|t")
                    + L(player, "Main gauche", "Off-Hand");
                std::string closeLabel = std::string("|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:40:40:-18|t")
                    + L(player, "Laisser tomber", "Nevermind");

                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, mainHandLabel, GOSSIP_SENDER_MAIN, VIS_GOSSIP_MAIN_HAND_ACTION);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, offHandLabel, GOSSIP_SENDER_MAIN, VIS_GOSSIP_OFF_HAND_ACTION);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, closeLabel, GOSSIP_SENDER_MAIN, VIS_GOSSIP_CLOSE_ACTION);

                SendGossipMenuFor(player, VIS_DEFAULT_MESSAGE, me->GetGUID());
            }

            bool OnGossipHello(Player* player) override
            {
                GetMainMenu(player);
                return true;
            }

            bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
            {
                uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
                ClearGossipMenuFor(player);

                switch (action)
                {
                    case VIS_GOSSIP_MAIN_HAND_ACTION:
                        m_mainHandChoice[player->GetGUID()] = true;
                        GetMenu(player, 1);
                        return true;

                    case VIS_GOSSIP_OFF_HAND_ACTION:
                        m_mainHandChoice[player->GetGUID()] = false;
                        GetMenu(player, 1);
                        return true;

                    case VIS_GOSSIP_CLOSE_ACTION:
                        CloseGossipMenuFor(player);
                        return true;
                }

                if (action >= VisualDataCount)
                {
                    CloseGossipMenuFor(player);
                    return true;
                }

                uint32 menuData = vData[action].Submenu;

                if (menuData == VIS_GOSSIP_MAIN_MENU_ACTION)
                {
                    GetMainMenu(player);
                    return true;
                }
                else if (menuData == 0)
                {
                    SetVisual(player, vData[action].Id);
                    menuData = vData[action].Menu;
                }

                GetMenu(player, menuData);
                return true;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_visual_enchantAI(creature);
        }
    };

    class visual_enchant_player : public PlayerScript
    {
    public:
        visual_enchant_player() : PlayerScript("visual_enchant_player")
        {
            CharacterDatabase.Execute(
                "DELETE FROM `mod_weapon_visual_effect` WHERE NOT EXISTS "
                "(SELECT 1 FROM item_instance WHERE `mod_weapon_visual_effect`.item_guid = item_instance.guid)");
        }

        void RestoreVisual(Player* player) const
        {
            if (!player)
                return;

            QueryResult result = CharacterDatabase.PQuery(
                "SELECT item_guid, enchant_visual_id FROM `mod_weapon_visual_effect` "
                "WHERE item_guid IN (SELECT guid FROM item_instance WHERE owner_guid = {})",
                player->GetGUID().GetCounter());

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();
                uint32 itemGuid = fields[0].GetUInt32();
                uint32 visual   = fields[1].GetUInt32();

                for (uint8 slot = EQUIPMENT_SLOT_MAINHAND; slot <= EQUIPMENT_SLOT_OFFHAND; ++slot)
                {
                    Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                    if (item && item->GetGUID().GetCounter() == itemGuid)
                        player->SetUInt16Value(PLAYER_VISIBLE_ITEM_1_ENCHANTMENT + (item->GetSlot() * 2), 0, visual);
                }
            } while (result->NextRow());
        }

        void OnLogin(Player* player, bool /*firstLogin*/) override
        {
            RestoreVisual(player);

            if (sConfigMgr->GetBoolDefault("VisualWeapon.AnnounceEnable", true))
                ChatHandler(player->GetSession()).SendSysMessage(
                    L(player,
                        "Ce serveur utilise le systeme |cff4CFF00Visuel d'enchantement|r.",
                        "This server is running the |cff4CFF00Visual Enchant|r system."));
        }
    };

    class visual_enchant_world : public WorldScript
    {
    public:
        visual_enchant_world() : WorldScript("visual_enchant_world") { }

        void OnStartup() override
        {
            CharacterDatabase.Execute(
                "DELETE FROM `mod_weapon_visual_effect` WHERE NOT EXISTS "
                "(SELECT 1 FROM item_instance WHERE `mod_weapon_visual_effect`.item_guid = item_instance.guid)");
        }
    };

}

void AddSC_npc_visual_enchant()
{
    new VisualEnchant::npc_visual_enchant();
    new VisualEnchant::visual_enchant_player();
    new VisualEnchant::visual_enchant_world();
}
