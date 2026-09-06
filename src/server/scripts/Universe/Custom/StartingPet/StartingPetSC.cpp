/*
 * This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
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

#include "StartingPet.h"
#include "Player.h"
#include "Chat.h"
#include "WorldSession.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectAccessor.h"

static bool StartingPetEnable;
static bool StartingPetAnnounce;
static bool StartingPetHunter;
static bool StartingPetName;
static uint32 StartingMinion;

class StartingPetWorldConfig : public WorldScript
{
public:
    StartingPetWorldConfig() : WorldScript("StartingPetWorldConfig") { }

    void OnConfigLoad(bool /*reload*/) override
    {
        StartingPetEnable = sConfigMgr->GetBoolDefault("StartingPet.Enable", true);
        StartingPetAnnounce = sConfigMgr->GetBoolDefault("StartingPet.Announce", true);
        StartingPetHunter = sConfigMgr->GetBoolDefault("StartingPet.HunterPet", true);
        StartingPetName = sConfigMgr->GetBoolDefault("StartingPet.RandName", true);
        StartingMinion = uint32(sConfigMgr->GetIntDefault("StartingPet.WarlockMinion", 47));
    }
};

class StartingPetScripts : public PlayerScript
{
public:
    StartingPetScripts() : PlayerScript("StartingPetScripts") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        if (!firstLogin)
            return;

        if (StartingPetEnable)
        {
            if (StartingPetAnnounce)
            {
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00StartingPet |rmodule.");
            }

            bool const isHunter = (StartingPetHunter && player->GetClass() == CLASS_HUNTER);
            bool const isWarlock = (StartingMinion > 0 && player->GetClass() == CLASS_WARLOCK);

            if (isHunter || isWarlock)
            {
                ObjectGuid playerGuid = player->GetGUID();

                player->m_Events.AddEventAtOffset([playerGuid, isHunter, isWarlock]()
                {
                    Player* p = ObjectAccessor::FindPlayer(playerGuid);
                    if (!p || !p->IsInWorld())
                        return;

                    if (isHunter)
                    {
                        sStartingPet->CreateRandomPet(p, StartingPetName);
                        sStartingPet->LearnPetSpells(p);
                    }

                    if (isWarlock)
                    {
                        sStartingPet->LearnWarlockSpells(p, StartingMinion);
                        sStartingPet->SummonWarlockMinion(p, StartingMinion);
                    }
                }, 2s);
            }
        }
    }
};

void AddSC_starting_pet()
{
    new StartingPetWorldConfig();
    new StartingPetScripts();
}
