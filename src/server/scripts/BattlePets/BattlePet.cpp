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
 */

#include "PetBattleMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Creature.h"
#include "WorldSession.h"
#include "GossipDef.h"
#include <algorithm>
#include <cctype>

using namespace Syphrena::ChatCommands;

class mod_pet_battle_commandscript : public CommandScript
{
public:
    mod_pet_battle_commandscript() : CommandScript("mod_pet_battle_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "dp", HandleDpCommand, rbac::RBAC_PERM_COMMAND_HELP, Console::No },
        };
        return commandTable;
    }

    static bool HandleDpCommand(ChatHandler* handler, Optional<std::string> arg)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (!sConfigMgr->GetBoolDefault("PetBattle.Enable", true))
        {
            handler->PSendSysMessage(
                "%s",
                sPetBattleMgr->GetText(player, PETTXT_SYSTEM_DISABLED).c_str());
            return true;
        }

        // /dp aceptar | /dp rechazar
        if (arg && !arg->empty())
        {
            std::string sub = *arg;
            std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

            if (sub == "aceptar")
            {
                sPetBattleMgr->HandleDuelAccept(player);
                return true;
            }
            if (sub == "rechazar")
            {
                sPetBattleMgr->HandleDuelDecline(player);
                return true;
            }
        }

        // /dp con un jugador seleccionado -> desafiar
        Unit* target = player->GetSelectedUnit();
        if (target && target->GetTypeId() == TYPEID_PLAYER && target != player)
        {
            sPetBattleMgr->StartDuelRequest(player, target->ToPlayer());
            return true;
        }

        // /dp con una criatura del mundo seleccionada (npc "alimania" u otra
        // criatura normal): primero verificamos si esa criatura es un
        // companero capturable (existe un hechizo de mascota que invoca ese
        // mismo entry). De ser asi, el duelo contra ella arranca solo.
        if (target && target->GetTypeId() == TYPEID_UNIT && target->ToCreature())
        {
            if (sPetBattleMgr->TryStartWildBattle(player, target->ToCreature()))
                return true;

            handler->PSendSysMessage(
                "%s",
                sPetBattleMgr->GetText(player, PETTXT_WILD_NOT_CAPTUREABLE).c_str());
            return true;
        }

        // /dp sin objetivo valido -> abrir menu de configuracion de equipo
        sPetBattleMgr->ShowTeamMenu(player);
        return true;
    }
};

class mod_pet_battle_gossip : public PlayerScript
{
public:
    mod_pet_battle_gossip() : PlayerScript("mod_pet_battle_gossip") {}

    void OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 sender, uint32 action) override
    {
        // Menu de configuracion de equipo (principal y sub-menu de eleccion)
        if (sender == 9001 || sender == 9002)
        {
            sPetBattleMgr->HandleTeamGossipAction(player, sender, action);
            return;
        }

        // Menu de tirada de dados
        if (sender == 9010)
        {
            if (ActivePetBattle* battle = sPetBattleMgr->GetBattleByPlayer(player->GetGUID()))
                sPetBattleMgr->HandleDiceRoll(player, *battle);
            return;
        }

        // Menu de seleccion de ataque
        if (sender == 9020)
        {
            if (ActivePetBattle* battle = sPetBattleMgr->GetBattleByPlayer(player->GetGUID()))
                sPetBattleMgr->HandleAttack(player, *battle, static_cast<uint8>(action));
            return;
        }

        // Popup de aceptar/rechazar duelo (se abre solo, sin que el jugador
        // tenga que escribir /dp aceptar o /dp rechazar)
        if (sender == 9030)
        {
            if (action == 1)
                sPetBattleMgr->HandleDuelAccept(player);
            else if (action == 2)
                sPetBattleMgr->HandleDuelDecline(player);
            else
                player->PlayerTalkClass->SendCloseGossip();
            return;
        }
    }
};

class mod_pet_battle_playerscript : public PlayerScript
{
public:
    mod_pet_battle_playerscript() : PlayerScript("mod_pet_battle_playerscript") {}

    // OnPlayerLearnSpell a ete backporte dans ScriptMgr.h/.cpp + Player.cpp
    // (n'existait pas nativement dans ce fork).
    void OnPlayerLearnSpell(Player* player, uint32 spellID) override
    {
        if (!sConfigMgr->GetBoolDefault("PetBattle.Enable", true))
            return;

        sPetBattleMgr->RegisterPetIfSummonSpell(player, spellID);
    }
    // ============================================================
    // Puente de addon messages (reemplaza los gossip)
    // ============================================================
    //
    // PetBattleUI utiliza SendAddonMessage() mediante WHISPER hacia
    // el propio jugador:
    //
    // SendAddonMessage("PETBTL", mensaje, "WHISPER", UnitName("player"))
    //
    // OnPlayerCanUseChat n'existe pas dans ce fork. On utilise a la place
    // le OnChat() natif avec Player* receiver (meme scope : whisper cible),
    // qui ne bloque pas le message (retour void), ce qui est sans
    // consequence ici car les messages LANG_ADDON ne s'affichent jamais
    // dans le chat du client de toute facon.
    //

    void OnChat(
        Player* player,
        uint32 /*type*/,
        uint32 lang,
        std::string& msg,
        Player* /*receiver*/) override
    {
        if (!player)
            return;

        // Solo nos interesan mensajes enviados por el addon.
        if (lang != LANG_ADDON)
            return;

        // Buscar el separador:
        //
        // PETBTL<TAB>accion|datos
        //
        size_t tab = msg.find('\t');

        if (tab == std::string::npos)
            return;

        // Comprobar prefijo.
        if (msg.substr(0, tab) != "PETBTL")
            return;

        // Entregar el contenido a PetBattleMgr.
        sPetBattleMgr->HandleAddonMessage(
            player,
            msg.substr(tab + 1));
    }
};

void AddSC_pet_battle()
{
    new mod_pet_battle_commandscript();
    new mod_pet_battle_gossip();
    new mod_pet_battle_playerscript();
}
