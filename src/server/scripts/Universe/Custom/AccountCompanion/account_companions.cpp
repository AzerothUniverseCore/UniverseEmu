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

#include "Config.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"

class AccountCompanions : public PlayerScript
{
    static const bool limitrace = false; // Cette option permet d'apprendre les familiers des personnages de la même équipe.
public:
	AccountCompanions() : PlayerScript("AccountCompanions") { }

    void OnLogin(Player* pPlayer, bool /*first_login*/)
	{
        std::vector<uint32> Guids;
        QueryResult result1 = CharacterDatabase.PQuery("SELECT guid, race FROM characters WHERE account = {}", pPlayer->GetSession()->GetAccountId());
        if (!result1)
            return;

        do
        {
            Field* fields = result1->Fetch();

            /*uint32 guid = fields[0].GetUInt32();*/
            uint32 race = fields[1].GetUInt8();

            if ((Player::TeamForRace(race) == Player::TeamForRace(pPlayer->GetRace())) || !limitrace)
                Guids.push_back(result1->Fetch()[0].GetUInt32());

        } while (result1->NextRow());

          std::vector<uint32> Spells;

           for (auto& i : Guids)
           {
               QueryResult result2 = CharacterDatabase.PQuery("SELECT DISTINCT spell FROM character_spell WHERE guid = {}", i);
               if (!result2)
                   continue;

               do
               {
                   Spells.push_back(result2->Fetch()[0].GetUInt32());
               } while (result2->NextRow());
           }

		for (auto& i : Spells)
		{
			auto sSpell = sSpellStore.LookupEntry(i);
			 if (sSpell->Effect[0] == SPELL_EFFECT_SUMMON && sSpell->EffectMiscValueB[0] == 41)
			 pPlayer->LearnSpell(sSpell->ID, false);
		}
	}
};

void AddSC_AccountCompanions()
{
	new AccountCompanions();
}