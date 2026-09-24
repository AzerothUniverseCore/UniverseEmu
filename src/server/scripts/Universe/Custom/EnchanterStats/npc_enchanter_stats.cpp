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
#include "Common.h"
#include "WorldSession.h"

#include <string>

namespace EnchanterStatsNPC
{
    // -----------------------------------------------------------------
    // Helper bilingue frFR / enUS
    // -----------------------------------------------------------------
    inline char const* L(Player* player, char const* fr, char const* en)
    {
        if (player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_frFR)
            return fr;
        return en;
    }

    // -----------------------------------------------------------------
    // Enchantements
    // -----------------------------------------------------------------
    enum Enchants
    {
        ENCHANT_WEP_BERSERKING              = 3789,
        ENCHANT_WEP_BLADE_WARD              = 3869,
        ENCHANT_WEP_BLOOD_DRAINING          = 3870,
        ENCHANT_WEP_ACCURACY                = 3788,
        ENCHANT_WEP_AGILITY_1H              = 1103,
        ENCHANT_WEP_SPIRIT                  = 3844,
        ENCHANT_WEP_BATTLEMASTER            = 2675,
        ENCHANT_WEP_BLACK_MAGIC             = 3790,
        ENCHANT_WEP_ICEBREAKER              = 3239,
        ENCHANT_WEP_LIFEWARD                = 3241,
        ENCHANT_WEP_MIGHTY_SPELL_POWER      = 3834, // One-hand
        ENCHANT_WEP_EXECUTIONER             = 3225,
        ENCHANT_WEP_POTENCY                 = 3833,
        ENCHANT_WEP_TITANGUARD              = 3851,
        ENCHANT_2WEP_MASSACRE               = 3827,
        ENCHANT_2WEP_SCOURGEBANE            = 3247,
        ENCHANT_2WEP_GIANT_SLAYER           = 3251,
        ENCHANT_2WEP_GREATER_SPELL_POWER    = 3854,
        ENCHANT_2WEP_AGILITY                = 2670,
        ENCHANT_2WEP_MONGOOSE               = 2673,

        ENCHANT_SHIELD_DEFENSE              = 1952,
        ENCHANT_SHIELD_INTELLECT            = 1128,
        ENCHANT_SHIELD_RESILIENCE           = 3229,
        ENCHANT_SHIELD_BLOCK                = 2655,
        ENCHANT_SHIELD_STAMINA              = 1071,
        ENCHANT_SHIELD_TOUGHSHIELD          = 2653,
        ENCHANT_SHIELD_TITANIUM_PLATING     = 3849,

        ENCHANT_HEAD_BLISSFUL_MENDING       = 3819,
        ENCHANT_HEAD_BURNING_MYSTERIES      = 3820,
        ENCHANT_HEAD_DOMINANCE              = 3796,
        ENCHANT_HEAD_SAVAGE_GLADIATOR       = 3842,
        ENCHANT_HEAD_STALWART_PROTECTOR     = 3818,
        ENCHANT_HEAD_TORMENT                = 3817,
        ENCHANT_HEAD_TRIUMPH                = 3795,
        ENCHANT_HEAD_ECLIPSED_MOON          = 3815,
        ENCHANT_HEAD_FLAME_SOUL             = 3816,
        ENCHANT_HEAD_FLEEING_SHADOW         = 3814,
        ENCHANT_HEAD_FROSTY_SOUL            = 3812,
        ENCHANT_HEAD_TOXIC_WARDING          = 3813,

        ENCHANT_SHOULDER_MASTERS_AXE        = 3835,
        ENCHANT_SHOULDER_MASTERS_CRAG       = 3836,
        ENCHANT_SHOULDER_MASTERS_PINNACLE   = 3837,
        ENCHANT_SHOULDER_MASTERS_STORM      = 3838,
        ENCHANT_SHOULDER_GREATER_AXE        = 3808,
        ENCHANT_SHOULDER_GREATER_CRAG       = 3809,
        ENCHANT_SHOULDER_GREATER_GLADIATOR  = 3852,
        ENCHANT_SHOULDER_GREATER_PINNACLE   = 3811,
        ENCHANT_SHOULDER_GREATER_STORM      = 3810,
        ENCHANT_SHOULDER_DOMINANCE          = 3794,
        ENCHANT_SHOULDER_TRIUMPH            = 3793,

        ENCHANT_CLOAK_DARKGLOW_EMBROIDERY   = 3728,
        ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY = 3730,
        ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY = 3722,
        ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE  = 3859,
        ENCHANT_CLOAK_WISDOM                = 3296,
        ENCHANT_CLOAK_TITANWEAVE            = 1951,
        ENCHANT_CLOAK_SPELL_PIERCING        = 3243,
        ENCHANT_CLOAK_SHADOW_ARMOR          = 3256,
        ENCHANT_CLOAK_MIGHTY_ARMOR          = 3294,
        ENCHANT_CLOAK_MAJOR_AGILITY         = 1099,
        ENCHANT_CLOAK_GREATER_SPEED         = 3831,

        ENCHANT_LEG_EARTHEN                 = 3853,
        ENCHANT_LEG_FROSTHIDE               = 3822,
        ENCHANT_LEG_ICESCALE                = 3823,
        ENCHANT_LEG_BRILLIANT_SPELLTHREAD   = 3719,
        ENCHANT_LEG_SAPPHIRE_SPELLTHREAD    = 3721,
        ENCHANT_LEG_DRAGONSCALE             = 3331,
        ENCHANT_LEG_WYRMSCALE               = 3332,

        ENCHANT_GLOVES_GREATER_BLASTING     = 3249,
        ENCHANT_GLOVES_ARMSMAN              = 3253,
        ENCHANT_GLOVES_CRUSHER              = 1603,
        ENCHANT_GLOVES_AGILITY              = 3222,
        ENCHANT_GLOVES_PRECISION            = 3234,
        ENCHANT_GLOVES_EXPERTISE            = 3231,
        ENCHANT_GLOVES_HYPERSPEED           = 3604,

        ENCHANT_BRACERS_MAJOR_STAMINA       = 3850,
        ENCHANT_BRACERS_SUPERIOR_SP         = 2332,
        ENCHANT_BRACERS_GREATER_ASSUALT     = 3845,
        ENCHANT_BRACERS_MAJOR_SPIRT         = 1147,
        ENCHANT_BRACERS_EXPERTISE           = 3231,
        ENCHANT_BRACERS_GREATER_STATS       = 2661,
        ENCHANT_BRACERS_INTELLECT           = 1119,
        ENCHANT_BRACERS_FURL_ARCANE         = 3763,
        ENCHANT_BRACERS_FURL_FIRE           = 3759,
        ENCHANT_BRACERS_FURL_FROST          = 3760,
        ENCHANT_BRACERS_FURL_NATURE         = 3762,
        ENCHANT_BRACERS_FURL_SHADOW         = 3761,
        ENCHANT_BRACERS_FURL_ATTACK         = 3756,
        ENCHANT_BRACERS_FURL_STAMINA        = 3757,
        ENCHANT_BRACERS_FURL_SPELLPOWER     = 3758,

        ENCHANT_CHEST_POWERFUL_STATS        = 3832,
        ENCHANT_CHEST_SUPER_HEALTH          = 3297,
        ENCHANT_CHEST_GREATER_MAINA_REST    = 2381,
        ENCHANT_CHEST_EXCEPTIONAL_RESIL     = 3245,
        ENCHANT_CHEST_GREATER_DEFENSE       = 1953,

        ENCHANT_BOOTS_GREATER_ASSULT        = 1597,
        ENCHANT_BOOTS_TUSKARS_VITLIATY      = 3232,
        ENCHANT_BOOTS_SUPERIOR_AGILITY      = 983,
        ENCHANT_BOOTS_GREATER_SPIRIT        = 1147,
        ENCHANT_BOOTS_GREATER_VITALITY      = 3244,
        ENCHANT_BOOTS_ICEWALKER             = 3826,
        ENCHANT_BOOTS_GREATER_FORTITUDE     = 1075,
        ENCHANT_BOOTS_NITRO_BOOTS           = 3606,
        ENCHANT_BOOTS_PYRO_ROCKET           = 3603,
        ENCHANT_BOOTS_ARMOR_WEBBING         = 3860,

        ENCHANT_RING_ASSULT                 = 3839,
        ENCHANT_RING_GREATER_SP             = 3840,
        ENCHANT_RING_STAMINA                = 3791,
    };

    // -----------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------
    bool   EnchanterStatsEnableModule   = true;
    bool   EnchanterStatsAnnounceModule = true;
    uint32 EnchanterStatsNumPhrases     = 3;
    uint32 EnchanterStatsMessageTimer   = 60000;
    uint32 EnchanterStatsEmoteSpell     = 0;
    uint32 EnchanterStatsEmoteCommand   = 3;

    class EnchanterStatsConfig : public WorldScript
    {
    public:
        EnchanterStatsConfig() : WorldScript("EnchanterStatsConfig") { }

        void OnConfigLoad(bool reload) override
        {
            if (reload)
                return;

            EnchanterStatsEnableModule   = sConfigMgr->GetBoolDefault("EnchanterStats.Enable", true);
            EnchanterStatsAnnounceModule = sConfigMgr->GetBoolDefault("EnchanterStats.Announce", true);
            EnchanterStatsNumPhrases     = sConfigMgr->GetIntDefault("EnchanterStats.NumPhrases", 3);
            EnchanterStatsMessageTimer   = sConfigMgr->GetIntDefault("EnchanterStats.MessageTimer", 60000);
            EnchanterStatsEmoteSpell     = sConfigMgr->GetIntDefault("EnchanterStats.EmoteSpell", 0);
            EnchanterStatsEmoteCommand   = sConfigMgr->GetIntDefault("EnchanterStats.EmoteCommand", 3);

            // Le timer de phrase reste entre 1 et 5 minutes (ou 0 pour le desactiver)
            if (EnchanterStatsMessageTimer != 0 && (EnchanterStatsMessageTimer < 60000 || EnchanterStatsMessageTimer > 300000))
                EnchanterStatsMessageTimer = 60000;
        }
    };

    // -----------------------------------------------------------------
    // Annonce a la connexion (bilingue)
    // -----------------------------------------------------------------
    class EnchanterStatsAnnounce : public PlayerScript
    {
    public:
        EnchanterStatsAnnounce() : PlayerScript("EnchanterStatsAnnounce") { }

        void OnLogin(Player* player, bool /*firstLogin*/) override
        {
            if (EnchanterStatsAnnounceModule)
                ChatHandler(player->GetSession()).SendSysMessage(L(player,
                    "Ce serveur propose le PNJ d'enchantements de statistiques |cff4CFF00Beauregard|r.",
                    "This server features the stat enchanting NPC |cff4CFF00Beauregard|r."));
        }
    };

    // -----------------------------------------------------------------
    // PNJ principal : entree 2000002, ScriptName "npc_enchanter_stats"
    // -----------------------------------------------------------------
    class npc_enchanter_stats : public CreatureScript
    {
    public:
        npc_enchanter_stats() : CreatureScript("npc_enchanter_stats") { }

        struct npc_enchanter_statsAI : public ScriptedAI
        {
            npc_enchanter_statsAI(Creature* creature) : ScriptedAI(creature) { }

            uint32 MessageTimer = 0;

            void Reset() override
            {
                if (EnchanterStatsMessageTimer != 0)
                    MessageTimer = urand(EnchanterStatsMessageTimer, 300000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!EnchanterStatsEnableModule || EnchanterStatsMessageTimer == 0)
                    return;

                if (MessageTimer <= diff)
                {
                    if (EnchanterStatsNumPhrases > 0)
                    {
                        std::string message = PickPhrase();
                        if (!message.empty())
                            me->Say(message.c_str(), LANG_UNIVERSAL);
                    }

                    if (EnchanterStatsEmoteCommand != 0)
                        me->HandleEmoteCommand(static_cast<Emote>(EnchanterStatsEmoteCommand));

                    if (EnchanterStatsEmoteSpell != 0)
                        me->CastSpell(me, EnchanterStatsEmoteSpell);

                    MessageTimer = urand(EnchanterStatsMessageTimer, 300000);
                }
                else
                {
                    MessageTimer -= diff;
                }
            }

            // Phrase ambiante bilingue : dite a tous les joueurs proches en une
            // seule fois (pas de joueur cible unique), donc on tire au sort
            // aussi bien le numero de phrase que la langue (50/50 FR / EN).
            // Cles attendues dans le .conf : EST.P1.FR / EST.P1.EN, EST.P2.FR / EST.P2.EN, ...
            static std::string PickPhrase()
            {
                uint32 phraseNum = urand(1, EnchanterStatsNumPhrases);
                bool french = urand(0, 1) == 0;
                std::string key = "EST.P" + std::to_string(phraseNum) + (french ? ".FR" : ".EN");
                return sConfigMgr->GetStringDefault(key.c_str(), "");
            }

            // ---------------------------------------------------------
            // Menu principal
            // ---------------------------------------------------------
            bool OnGossipHello(Player* player) override
            {
                if (!EnchanterStatsEnableModule)
                    return false;

                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/Inv_mace_116:24:24:-18|t[Enchanter l'arme principale]",
                    "|TInterface/ICONS/Inv_mace_116:24:24:-18|t[Enchant Main Weapon]"), GOSSIP_SENDER_MAIN, 1);

                if (player->HasSpell(674))
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                        "|TInterface/ICONS/Inv_mace_116:24:24:-18|t[Enchanter l'arme secondaire]",
                        "|TInterface/ICONS/Inv_mace_116:24:24:-18|t[Enchant Offhand Weapon]"), GOSSIP_SENDER_MAIN, 13);

                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/Inv_axe_113:24:24:-18|t[Enchanter une arme a 2 mains]",
                    "|TInterface/ICONS/Inv_axe_113:24:24:-18|t[Enchant 2H Weapon]"), GOSSIP_SENDER_MAIN, 2);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/Inv_shield_71:24:24:-18|t[Enchanter le bouclier]",
                    "|TInterface/ICONS/Inv_shield_71:24:24:-18|t[Enchant Shield]"), GOSSIP_SENDER_MAIN, 3);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_helmet_29:24:24:-18|t[Enchanter la tete]",
                    "|TInterface/ICONS/inv_helmet_29:24:24:-18|t[Enchant Head]"), GOSSIP_SENDER_MAIN, 4);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_shoulder_23:24:24:-18|t[Enchanter les epaulieres]",
                    "|TInterface/ICONS/inv_shoulder_23:24:24:-18|t[Enchant Shoulders]"), GOSSIP_SENDER_MAIN, 5);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/Inv_misc_cape_18:24:24:-18|t[Enchanter la cape]",
                    "|TInterface/ICONS/Inv_misc_cape_18:24:24:-18|t[Enchant Cloak]"), GOSSIP_SENDER_MAIN, 6);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_chest_cloth_04:24:24:-18|t[Enchanter le torse]",
                    "|TInterface/ICONS/inv_chest_cloth_04:24:24:-18|t[Enchant Chest]"), GOSSIP_SENDER_MAIN, 7);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_bracer_14:24:24:-18|t[Enchanter les brassards]",
                    "|TInterface/ICONS/inv_bracer_14:24:24:-18|t[Enchant Bracers]"), GOSSIP_SENDER_MAIN, 8);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_gauntlets_06:24:24:-18|t[Enchanter les gantelets]",
                    "|TInterface/ICONS/inv_gauntlets_06:24:24:-18|t[Enchant Gloves]"), GOSSIP_SENDER_MAIN, 9);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_pants_11:24:24:-18|t[Enchanter les jambieres]",
                    "|TInterface/ICONS/inv_pants_11:24:24:-18|t[Enchant Legs]"), GOSSIP_SENDER_MAIN, 10);
                AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                    "|TInterface/ICONS/inv_boots_05:24:24:-18|t[Enchanter les bottes]",
                    "|TInterface/ICONS/inv_boots_05:24:24:-18|t[Enchant Boots]"), GOSSIP_SENDER_MAIN, 11);

                if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player,
                        "|TInterface/ICONS/Inv_jewelry_ring_85:24:24:-18|t[Enchanter les anneaux]",
                        "|TInterface/ICONS/Inv_jewelry_ring_85:24:24:-18|t[Enchant Rings]"), GOSSIP_SENDER_MAIN, 12);

                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                return true;
            }

            // ---------------------------------------------------------
            // Selection dans le gossip
            // ---------------------------------------------------------
            bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
            {
                if (!EnchanterStatsEnableModule)
                    return false;

                uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
                Item* item = nullptr;
                ClearGossipMenuFor(player);

                switch (action)
                {
                case 1: // Enchanter Main Droite / Enchant Main Hand
                    if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Garde tranchante", "Blade Ward"), GOSSIP_SENDER_MAIN, 102);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Ponction de sang", "Blood Draining"), GOSSIP_SENDER_MAIN, 103);
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "26 Agilite", "26 Agility"), GOSSIP_SENDER_MAIN, 100);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "45 Esprit", "45 Spirit"), GOSSIP_SENDER_MAIN, 101);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Berserker", "Berserking"), GOSSIP_SENDER_MAIN, 104);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "25 Toucher + 25 Critique", "25 Hit Rating + 25 Critical"), GOSSIP_SENDER_MAIN, 105);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Magie noire", "Black Magic"), GOSSIP_SENDER_MAIN, 106);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Maitre de guerre", "Battlemaster"), GOSSIP_SENDER_MAIN, 107);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Brise-glace", "Icebreaker"), GOSSIP_SENDER_MAIN, 108);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Preservation de vie", "Lifeward"), GOSSIP_SENDER_MAIN, 109);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Endurance", "50 Stamina"), GOSSIP_SENDER_MAIN, 110);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "65 Puissance d'attaque", "65 Attack Power"), GOSSIP_SENDER_MAIN, 111);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "63 Puissance des sorts", "63 Spell Power"), GOSSIP_SENDER_MAIN, 112);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Mangouste", "Mongoose"), GOSSIP_SENDER_MAIN, 113);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Bourreau", "Executioner"), GOSSIP_SENDER_MAIN, 114);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 2: // Enchanter une arme a 2 mains / Enchant 2H Weapon
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!item || item->GetTemplate()->InventoryType != INVTYPE_2HWEAPON)
                    {
                        me->Whisper(L(player,
                            "Cet enchantement necessite une arme a deux mains equipee.",
                            "This enchant requires a 2H weapon to be equipped."), LANG_UNIVERSAL, player);
                        CloseGossipMenuFor(player);
                        return true;
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Berserker", "Berserking"), GOSSIP_SENDER_MAIN, 104);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Mangouste", "Mongoose"), GOSSIP_SENDER_MAIN, 113);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Bourreau", "Executioner"), GOSSIP_SENDER_MAIN, 114);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "81 Puissance des sorts", "81 Spell Power"), GOSSIP_SENDER_MAIN, 115);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "35 Agilite", "35 Agility"), GOSSIP_SENDER_MAIN, 116);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "110 Puissance d'attaque", "110 Attack Power"), GOSSIP_SENDER_MAIN, 117);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 3: // Enchanter le bouclier / Enchant Shield
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item || item->GetTemplate()->InventoryType != INVTYPE_SHIELD)
                    {
                        me->Whisper(L(player,
                            "Cet enchantement necessite un bouclier equipe.",
                            "This enchant requires a shield to be equipped."), LANG_UNIVERSAL, player);
                        CloseGossipMenuFor(player);
                        return true;
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "20 Defense", "20 Defense"), GOSSIP_SENDER_MAIN, 118);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "25 Intelligence", "25 Intellect"), GOSSIP_SENDER_MAIN, 119);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "12 Resilience", "12 Resilience"), GOSSIP_SENDER_MAIN, 120);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "36 Blocage", "36 Block"), GOSSIP_SENDER_MAIN, 121);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "18 Endurance", "18 Stamina"), GOSSIP_SENDER_MAIN, 122);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "81 Blocage + 50% de desarmement en moins", "81 Block + 50% Less Disarm"), GOSSIP_SENDER_MAIN, 123);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 4: // Enchanter la tete / Enchant Head
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Puissance des sorts + 10 Mana/5s", "30 Spell Power + 10 Mp5"), GOSSIP_SENDER_MAIN, 124);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Puissance des sorts + 20 Critique", "30 Spell Power + 20 Crit"), GOSSIP_SENDER_MAIN, 125);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "29 Puissance des sorts + 20 Resilience", "29 Spell Power + 20 Resilience"), GOSSIP_SENDER_MAIN, 126);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Endurance + 25 Resilience", "30 Stamina + 25 Resilience"), GOSSIP_SENDER_MAIN, 127);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "37 Endurance + 20 Defense", "37 Stamina + 20 Defense"), GOSSIP_SENDER_MAIN, 128);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Puissance d'attaque + 20 Critique", "50 Attack Power + 20 Crit"), GOSSIP_SENDER_MAIN, 129);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Puissance d'attaque + 20 Resilience", "50 Attack Power + 20 Resilience"), GOSSIP_SENDER_MAIN, 130);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Arcane de la lune eclipsee", "Arcanum of Eclipsed Moon"), GOSSIP_SENDER_MAIN, 131);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Arcane de l'ame ardente", "Arcanum of the Flame's Soul"), GOSSIP_SENDER_MAIN, 132);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Arcane de l'ombre fuyante", "Arcanum of the Fleeing Shadow"), GOSSIP_SENDER_MAIN, 133);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Arcane de l'ame givree", "Arcanum of the Frosty Soul"), GOSSIP_SENDER_MAIN, 134);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Arcane de la protection toxique", "Arcanum of Toxic Warding"), GOSSIP_SENDER_MAIN, 135);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 5: // Enchanter les epaulieres / Enchant Shoulders
                    if (player->HasSkill(SKILL_INSCRIPTION) && player->GetSkillValue(SKILL_INSCRIPTION) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "120 Puissance d'attaque + 15 Critique", "120 Attack Power + 15 Crit"), GOSSIP_SENDER_MAIN, 136);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "70 Puissance des sorts + 8 Mana/5s", "70 Spell Power + 8 Mp5"), GOSSIP_SENDER_MAIN, 137);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "60 Esquive + 15 Defense", "60 Dodge + 15 Defense"), GOSSIP_SENDER_MAIN, 138);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "70 Puissance des sorts + 15 Critique", "70 Spell Power + 15 Crit"), GOSSIP_SENDER_MAIN, 139);
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "40 Puissance d'attaque + 15 Critique", "40 Attack Power + 15 Crit"), GOSSIP_SENDER_MAIN, 140);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "24 Puissance des sorts + 8 Mana/5s", "24 Spell Power + 8 Mp5"), GOSSIP_SENDER_MAIN, 141);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Endurance + 15 Resilience", "30 Stamina + 15 Resilience"), GOSSIP_SENDER_MAIN, 142);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "20 Esquive + 15 Defense", "20 Dodge + 15 Defense"), GOSSIP_SENDER_MAIN, 143);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "24 Puissance des sorts + 15 Critique", "24 Spell Power + 15 Crit"), GOSSIP_SENDER_MAIN, 144);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "23 Puissance des sorts + 15 Resilience", "23 Spell Power + 15 Resilience"), GOSSIP_SENDER_MAIN, 145);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "40 Puissance d'attaque + 15 Resilience", "40 Attack Power + 15 Resilience"), GOSSIP_SENDER_MAIN, 146);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 6: // Enchanter la cape / Enchant Cloak
                    if (player->HasSkill(SKILL_TAILORING) && player->GetSkillValue(SKILL_TAILORING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Broderie de lueur sombre", "Darkglow Embroidery"), GOSSIP_SENDER_MAIN, 149);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Broderie de trame de lumiere", "Lightweave Embroidery"), GOSSIP_SENDER_MAIN, 150);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Broderie de garde-lame", "Swordguard Embroidery"), GOSSIP_SENDER_MAIN, 151);
                    }
                    if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Parachute", "Parachute"), GOSSIP_SENDER_MAIN, 147);
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Armure de l'ombre", "Shadow Armor"), GOSSIP_SENDER_MAIN, 148);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "10 Esprit + 2% de menace en moins", "10 Spirit + 2% Reduced Threat"), GOSSIP_SENDER_MAIN, 152);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "16 Defense", "16 Defense"), GOSSIP_SENDER_MAIN, 153);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "35 Penetration des sorts", "35 Spell Penetration"), GOSSIP_SENDER_MAIN, 154);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "225 Armure", "225 Armor"), GOSSIP_SENDER_MAIN, 155);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "22 Agilite", "22 Agility"), GOSSIP_SENDER_MAIN, 156);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "23 Hate", "23 Haste"), GOSSIP_SENDER_MAIN, 157);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 7: // Enchanter le torse / Enchant Chest
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "+10 a toutes les statistiques", "+10 All Stats"), GOSSIP_SENDER_MAIN, 158);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "225 Points de vie", "225 Health"), GOSSIP_SENDER_MAIN, 159);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "10 Mana/5s", "10 Mp5"), GOSSIP_SENDER_MAIN, 160);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "20 Resilience", "20 Resilience"), GOSSIP_SENDER_MAIN, 161);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "22 Defense", "22 Defense"), GOSSIP_SENDER_MAIN, 162);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 8: // Enchanter les brassards / Enchant Bracers
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "40 Endurance", "40 Stamina"), GOSSIP_SENDER_MAIN, 163);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Puissance des sorts", "30 Spell Power"), GOSSIP_SENDER_MAIN, 164);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Puissance d'attaque", "50 Attack Power"), GOSSIP_SENDER_MAIN, 165);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "18 Esprit", "18 Spirit"), GOSSIP_SENDER_MAIN, 166);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "15 Expertise", "15 Expertise"), GOSSIP_SENDER_MAIN, 167);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "+6 a toutes les statistiques", "+6 All Stats"), GOSSIP_SENDER_MAIN, 168);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "16 Intelligence", "16 Intellect"), GOSSIP_SENDER_MAIN, 169);
                    if (player->HasSkill(SKILL_LEATHERWORKING) && player->GetSkillValue(SKILL_LEATHERWORKING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Resist. Arcane", "Fur Lining - Arcane Resist"), GOSSIP_SENDER_MAIN, 170);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Resist. Feu", "Fur Lining - Fire Resist"), GOSSIP_SENDER_MAIN, 171);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Resist. Givre", "Fur Lining - Frost Resist"), GOSSIP_SENDER_MAIN, 172);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Resist. Nature", "Fur Lining - Nature Resist"), GOSSIP_SENDER_MAIN, 173);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Resist. Ombre", "Fur Lining - Shadow Resist"), GOSSIP_SENDER_MAIN, 174);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Puissance d'attaque", "Fur Lining - Attack Power"), GOSSIP_SENDER_MAIN, 175);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Endurance", "Fur Lining - Stamina"), GOSSIP_SENDER_MAIN, 176);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Doublure fourree - Puissance des sorts", "Fur Lining - Spellpower"), GOSSIP_SENDER_MAIN, 177);
                    }
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 9: // Enchanter les gantelets / Enchant Gloves
                    if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 400)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Accelerateurs hypervitesse", "Hyperspeed Accelerators"), GOSSIP_SENDER_MAIN, 200);
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "16 Coup critique", "16 Critical Strike"), GOSSIP_SENDER_MAIN, 178);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "2% de menace + 10 Parade", "2% Threat + 10 Parry"), GOSSIP_SENDER_MAIN, 179);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "44 Puissance d'attaque", "44 Attack Power"), GOSSIP_SENDER_MAIN, 180);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "20 Agilite", "20 Agility"), GOSSIP_SENDER_MAIN, 181);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "20 Toucher", "20 Hit Rating"), GOSSIP_SENDER_MAIN, 182);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "15 Expertise", "15 Expertise"), GOSSIP_SENDER_MAIN, 183);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 10: // Enchanter les jambieres / Enchant Legs
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "40 Resilience + 28 Endurance", "40 Resilience + 28 Stamina"), GOSSIP_SENDER_MAIN, 184);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "55 Endurance + 22 Agilite", "55 Stamina + 22 Agility"), GOSSIP_SENDER_MAIN, 185);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "75 Puissance d'attaque + 22 Critique", "75 Attack Power + 22 Critical"), GOSSIP_SENDER_MAIN, 186);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Puissance des sorts + 22 Esprit", "50 Spell Power + 22 Spirit"), GOSSIP_SENDER_MAIN, 187);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Puissance des sorts + 30 Endurance", "50 Spell Power + 30 Stamina"), GOSSIP_SENDER_MAIN, 188);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "72 Endurance + 35 Agilite", "72 Stamina + 35 Agility"), GOSSIP_SENDER_MAIN, 189);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "100 Puissance d'attaque + 36 Critique", "100 Attack Power + 36 Critical"), GOSSIP_SENDER_MAIN, 190);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 11: // Enchanter les bottes / Enchant Boots
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "32 Puissance d'attaque", "32 Attack Power"), GOSSIP_SENDER_MAIN, 191);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "15 Endurance + legere vitesse de deplacement", "15 Stamina + Minor Speed Increase"), GOSSIP_SENDER_MAIN, 192);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "16 Agilite", "16 Agility"), GOSSIP_SENDER_MAIN, 193);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "18 Esprit", "18 Spirit"), GOSSIP_SENDER_MAIN, 194);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Restaure 7 PV + Mana/5s", "Restore 7 Health + Mp5"), GOSSIP_SENDER_MAIN, 195);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "12 Toucher + 12 Critique", "12 Hit Rating + 12 Critical"), GOSSIP_SENDER_MAIN, 196);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "22 Endurance", "22 Stamina"), GOSSIP_SENDER_MAIN, 197);
                    if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Bottes a reaction nitro", "Nitro Boots"), GOSSIP_SENDER_MAIN, 198);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Fusee pyrotechnique manuelle", "Hand-Mounted Pyro Rocket"), GOSSIP_SENDER_MAIN, 199);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Treillis d'armure reticule", "Reticulated Armor Webbing"), GOSSIP_SENDER_MAIN, 201);
                    }
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 12: // Enchanter les anneaux / Enchant Rings
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "40 Puissance d'attaque", "40 Attack Power"), GOSSIP_SENDER_MAIN, 202);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "23 Puissance des sorts", "23 Spell Power"), GOSSIP_SENDER_MAIN, 203);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "30 Endurance", "30 Stamina"), GOSSIP_SENDER_MAIN, 204);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                case 13: // Enchanter Main Gauche / Enchant Offhand Weapon
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item || item->GetTemplate()->InventoryType != INVTYPE_WEAPON)
                    {
                        me->Whisper(L(player,
                            "Cet enchantement necessite une arme equipee en main gauche.",
                            "This enchant requires a weapon to be equipped in offhand."), LANG_UNIVERSAL, player);
                        CloseGossipMenuFor(player);
                        return true;
                    }
                    if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                    {
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Garde tranchante", "Blade Ward"), GOSSIP_SENDER_MAIN, 207);
                        AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Ponction de sang", "Blood Draining"), GOSSIP_SENDER_MAIN, 208);
                    }
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "26 Agilite", "26 Agility"), GOSSIP_SENDER_MAIN, 205);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "45 Esprit", "45 Spirit"), GOSSIP_SENDER_MAIN, 206);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Berserker", "Berserking"), GOSSIP_SENDER_MAIN, 209);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "25 Toucher + 25 Critique", "25 Hit Rating + 25 Critical"), GOSSIP_SENDER_MAIN, 210);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Magie noire", "Black Magic"), GOSSIP_SENDER_MAIN, 211);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Maitre de guerre", "Battlemaster"), GOSSIP_SENDER_MAIN, 212);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Brise-glace", "Icebreaker"), GOSSIP_SENDER_MAIN, 213);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Preservation de vie", "Lifeward"), GOSSIP_SENDER_MAIN, 214);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "50 Endurance", "50 Stamina"), GOSSIP_SENDER_MAIN, 215);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "65 Puissance d'attaque", "65 Attack Power"), GOSSIP_SENDER_MAIN, 216);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "63 Puissance des sorts", "63 Spell Power"), GOSSIP_SENDER_MAIN, 217);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Mangouste", "Mongoose"), GOSSIP_SENDER_MAIN, 218);
                    AddGossipItemFor(player, static_cast<GossipOptionIcon>(1), L(player, "Bourreau", "Executioner"), GOSSIP_SENDER_MAIN, 219);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, L(player, "Retour", "Back"), GOSSIP_SENDER_MAIN, 300);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    return true;

                // -----------------------------------------------------
                // Application des enchantements (arme principale / 2M)
                // -----------------------------------------------------
                case 100: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_AGILITY_1H); break;
                case 101: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_SPIRIT); break;
                case 102: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLADE_WARD); break;
                case 103: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLOOD_DRAINING); break;
                case 104: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BERSERKING); break;
                case 105: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ACCURACY); break;
                case 106: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLACK_MAGIC); break;
                case 107: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BATTLEMASTER); break;
                case 108: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ICEBREAKER); break;
                case 109: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_LIFEWARD); break;
                case 110: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_TITANGUARD); break;
                case 111: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_POTENCY); break;
                case 112: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER); break;
                case 113: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MONGOOSE); break;
                case 114: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_EXECUTIONER); break;
                case 115: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_GREATER_SPELL_POWER); break;
                case 116: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_AGILITY); break;
                case 117: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MASSACRE); break;

                // Bouclier
                case 118: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_DEFENSE); break;
                case 119: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_INTELLECT); break;
                case 120: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_RESILIENCE); break;
                case 121: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TITANIUM_PLATING); break;
                case 122: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_STAMINA); break;
                case 123: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TOUGHSHIELD); break;

                // Tete
                case 124: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BLISSFUL_MENDING); break;
                case 125: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BURNING_MYSTERIES); break;
                case 126: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_DOMINANCE); break;
                case 127: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_SAVAGE_GLADIATOR); break;
                case 128: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_STALWART_PROTECTOR); break;
                case 129: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TORMENT); break;
                case 130: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TRIUMPH); break;
                case 131: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_ECLIPSED_MOON); break;
                case 132: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLAME_SOUL); break;
                case 133: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLEEING_SHADOW); break;
                case 134: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FROSTY_SOUL); break;
                case 135: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TOXIC_WARDING); break;

                // Epaulieres
                case 136: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_AXE); break;
                case 137: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_CRAG); break;
                case 138: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_PINNACLE); break;
                case 139: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_STORM); break;
                case 140: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_AXE); break;
                case 141: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_CRAG); break;
                case 142: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_GLADIATOR); break;
                case 143: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_PINNACLE); break;
                case 144: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_STORM); break;
                case 145: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_DOMINANCE); break;
                case 146: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_TRIUMPH); break;

                // Cape
                case 147: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE); break;
                case 148: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SHADOW_ARMOR); break;
                case 149: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_DARKGLOW_EMBROIDERY); break;
                case 150: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY); break;
                case 151: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY); break;
                case 152: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_WISDOM); break;
                case 153: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_TITANWEAVE); break;
                case 154: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPELL_PIERCING); break;
                case 155: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MIGHTY_ARMOR); break;
                case 156: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MAJOR_AGILITY); break;
                case 157: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_GREATER_SPEED); break;

                // Torse
                case 158: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_POWERFUL_STATS); break;
                case 159: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_SUPER_HEALTH); break;
                case 160: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_MAINA_REST); break;
                case 161: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_EXCEPTIONAL_RESIL); break;
                case 162: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_DEFENSE); break;

                // Brassards
                case 163: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_STAMINA); break;
                case 164: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_SUPERIOR_SP); break;
                case 165: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_GREATER_ASSUALT); break;
                case 166: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_SPIRT); break;
                case 167: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_EXPERTISE); break;
                case 168: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_GREATER_STATS); break;
                case 169: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_INTELLECT); break;
                case 170: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ARCANE); break;
                case 171: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FIRE); break;
                case 172: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FROST); break;
                case 173: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_NATURE); break;
                case 174: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SHADOW); break;
                case 175: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ATTACK); break;
                case 176: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_STAMINA); break;
                case 177: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SPELLPOWER); break;

                // Gantelets
                case 178: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_GREATER_BLASTING); break;
                case 179: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_ARMSMAN); break;
                case 180: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_CRUSHER); break;
                case 181: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_AGILITY); break;
                case 182: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_PRECISION); break;
                case 183: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_EXPERTISE); break;

                // Jambieres
                case 184: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_EARTHEN); break;
                case 185: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_FROSTHIDE); break;
                case 186: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_ICESCALE); break;
                case 187: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_BRILLIANT_SPELLTHREAD); break;
                case 188: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_SAPPHIRE_SPELLTHREAD); break;
                case 189: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_DRAGONSCALE); break;
                case 190: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_WYRMSCALE); break;

                // Bottes
                case 191: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_ASSULT); break;
                case 192: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_TUSKARS_VITLIATY); break;
                case 193: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_SUPERIOR_AGILITY); break;
                case 194: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_SPIRIT); break;
                case 195: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_VITALITY); break;
                case 196: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_ICEWALKER); break;
                case 197: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_FORTITUDE); break;
                case 198: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_NITRO_BOOTS); break;
                case 199: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_PYRO_ROCKET); break;
                case 200: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_HYPERSPEED); break;
                case 201: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_ARMOR_WEBBING); break;

                // Anneaux (les deux doigts en meme temps)
                case 202:
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_ASSULT);
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_ASSULT);
                    break;
                case 203:
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_GREATER_SP);
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_GREATER_SP);
                    break;
                case 204:
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_STAMINA);
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_STAMINA);
                    break;

                // Main gauche (arme 1M)
                case 205: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_AGILITY_1H); break;
                case 206: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_SPIRIT); break;
                case 207: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLADE_WARD); break;
                case 208: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLOOD_DRAINING); break;
                case 209: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BERSERKING); break;
                case 210: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ACCURACY); break;
                case 211: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLACK_MAGIC); break;
                case 212: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BATTLEMASTER); break;
                case 213: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ICEBREAKER); break;
                case 214: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_LIFEWARD); break;
                case 215: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_TITANGUARD); break;
                case 216: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_POTENCY); break;
                case 217: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER); break;
                case 218: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_2WEP_MONGOOSE); break;
                case 219: Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_EXECUTIONER); break;

                case 300: // Retour au menu principal / Back to main menu
                    return OnGossipHello(player);

                default:
                    break;
                }

                CloseGossipMenuFor(player);
                return true;
            }

            // ---------------------------------------------------------
            // Application reelle de l'enchantement sur l'objet (bilingue)
            // ---------------------------------------------------------
            void Enchant(Player* player, Item* item, uint32 enchantId)
            {
                if (!item)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->Whisper(L(player,
                        "Equipez d'abord l'objet que vous souhaitez enchanter !",
                        "Please equip the item you would like to enchant!"), LANG_UNIVERSAL, player);
                    CloseGossipMenuFor(player);
                    return;
                }

                if (!enchantId)
                {
                    ChatHandler(player->GetSession()).SendSysMessage(L(player,
                        "Une erreur est survenue dans le code. Elle a ete enregistree pour les developpeurs, desole pour la gene occasionnee.",
                        "Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience."));
                    CloseGossipMenuFor(player);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    return;
                }

                uint32 roll = urand(1, 100);

                item->ClearEnchantment(PERM_ENCHANTMENT_SLOT);
                item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantId, 0, 0);

                if (roll < 33)
                    ChatHandler(player->GetSession()).PSendSysMessage(L(player,
                        "|cff00ff00Beauregard fait crepiter ses doigts osseux d'energie en touchant |cffDA70D6%s|cff00ff00 !",
                        "|cff00ff00Beauregard's bony finger crackles with energy when he touches |cffDA70D6%s|cff00ff00!"), item->GetTemplate()->Name1.c_str());
                else if (roll < 75)
                    ChatHandler(player->GetSession()).PSendSysMessage(L(player,
                        "|cff00ff00Beauregard brandit |cffDA70D6%s |cff00ff00dans les airs en psalmodiant une etrange incantation !",
                        "|cff00ff00Beauregard holds |cffDA70D6%s |cff00ff00up in the air and utters a strange incantation!"), item->GetTemplate()->Name1.c_str());
                else
                    ChatHandler(player->GetSession()).PSendSysMessage(L(player,
                        "|cff00ff00Beauregard se concentre intensement en agitant sa baguette au-dessus de |cffDA70D6%s|cff00ff00 !",
                        "|cff00ff00Beauregard concentrates deeply while waving his wand over |cffDA70D6%s|cff00ff00!"), item->GetTemplate()->Name1.c_str());

                me->CastSpell(player, 12512); // visuel d'enchantement
                CloseGossipMenuFor(player);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_enchanter_statsAI(creature);
        }
    };
}

using namespace EnchanterStatsNPC;

void AddSC_npc_enchanter_stats()
{
    new EnchanterStatsConfig();
    new EnchanterStatsAnnounce();
    new npc_enchanter_stats();
}
