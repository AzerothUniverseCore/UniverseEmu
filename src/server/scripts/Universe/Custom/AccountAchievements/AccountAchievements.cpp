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
#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "AchievementMgr.h"
#include "WorldSession.h"

class AccountAchievements : public PlayerScript
{
	static const bool limitrace = true; // Ce réglage à true ne permet d'obtenir que les succès des personnages de la même équipe, faites ce que vous voulez. IL N'EST PAS RECOMMANDÉ DE LE CHANGER !!!
	static const bool limitlevel = false; // Cette fonction vérifie le niveau du joueur et n'ajoutera des réalisations qu'aux joueurs de ce niveau.
	int minlevel = 80; // Il est fixé aux joueurs de niveau 60. Nécessite que limitlevel soit réglé sur true.
	int setlevel = 1; // Ne pas changer

public:
	AccountAchievements() : PlayerScript("AccountAchievements") { }

	void OnLogin(Player* pPlayer, bool)
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

		std::vector<uint32> Achievement;

		for (auto& i : Guids)
		{
			QueryResult result2 = CharacterDatabase.PQuery("SELECT achievement FROM character_achievement WHERE guid = {}", i);
			if (!result2)
				continue;

			do
			{
				Achievement.push_back(result2->Fetch()[0].GetUInt32());
			} while (result2->NextRow());
		}

		for (auto& i : Achievement)
		{
			auto sAchievement = sAchievementStore.LookupEntry(i);
				AddAchievements(pPlayer, sAchievement->ID);
		}
	}

	void AddAchievements(Player* player, uint32 AchievementID)
	{
		if (limitlevel)
			setlevel = minlevel;

		if (player->GetLevel() >= setlevel)
			player->CompletedAchievement(sAchievementStore.LookupEntry(AchievementID));
	}
};

void AddSC_accountachievement()
{
	new AccountAchievements();
}