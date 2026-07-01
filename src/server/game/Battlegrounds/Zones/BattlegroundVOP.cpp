/*
 * This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

#include "BattlegroundVOP.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "BattlegroundMgr.h"
#include "Creature.h"
#include "Language.h"
#include "Object.h"
#include "Player.h"

void BattlegroundVOPScore::BuildObjectivesBlock(WorldPacket& data)
{
    data << uint32(2);
    data << uint32(OrbControl);
    data << uint32(OrbScore);
}

BattlegroundVOP::BattlegroundVOP()
{
    m_BuffChange = true;
    BgObjects.resize(BG_VOP_OBJECT_MAX);
    BgCreatures.resize(BG_VOP_CREATURE_MAX);

    StartMessageIds[BG_STARTING_EVENT_FIRST] = BG_VOP_TEXT_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = BG_VOP_TEXT_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD] = BG_VOP_TEXT_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = BG_VOP_TEXT_BATTLE_HAS_BEGUN;

    bool isBGWeekend = BattlegroundMgr::IsBGWeekend(GetTypeID());
    m_ReputationCapture = (isBGWeekend) ? 45 : 35;
    m_HonorWinKills = (isBGWeekend) ? 3 : 1;
    m_HonorEndKills = (isBGWeekend) ? 4 : 2;
}

void BattlegroundVOP::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        int team_points[PVP_TEAMS_COUNT] = { 0, 0 };
         
        // Accumulate points
        for (int team = 0; team < PVP_TEAMS_COUNT; ++team)
        {
            int points = ++team_points[team];
            if (!points)
                continue;

            m_lastTick[team] += diff;
            if (m_lastTick[team] > BG_VOP_TickIntervals[points])
            {
                m_lastTick[team] -= BG_VOP_TickIntervals[points];
                m_TeamScores[team] += BG_VOP_TickPoints[points];
                m_HonorScoreTics[team] += BG_VOP_TickPoints[points];

                for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                {
                    if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                    {
                        if (player->GetTeamId() == team)
                        {
                            if (player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_1) ||
                                player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_2) ||
                                player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_3) ||
                                player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_4))
                            {
                                m_TeamScores[team] += BG_VOP_TickPoints[points];
                                UpdatePlayerScore(player, SCORE_ORB_SCORE, BG_VOP_TickPoints[points]);
                            }
                        }
                    }
                }

                if (m_HonorScoreTics[team] >= m_HonorTics)
                {
                    RewardHonorToTeam(GetBonusHonorFromKill(1), (team == TEAM_ALLIANCE) ? ALLIANCE : HORDE);
                    m_HonorScoreTics[team] -= m_HonorTics;
                }

                if (m_TeamScores[team] > BG_VOP_MAX_TEAM_SCORE)
                    m_TeamScores[team] = BG_VOP_MAX_TEAM_SCORE;

                if (team == TEAM_ALLIANCE)
                    UpdateWorldState(BG_VOP_OP_RESOURCES_A, m_TeamScores[team]);
                else if (team == TEAM_HORDE)
                    UpdateWorldState(BG_VOP_OP_RESOURCES_H, m_TeamScores[team]);

                uint8 otherTeam = (team + 1) % PVP_TEAMS_COUNT;
                if (m_TeamScores[team] > m_TeamScores[otherTeam] + 500)
                    m_TeamScores500Disadvantage[otherTeam] = true;
            }
        }

        // Test win condition
        if (m_TeamScores[TEAM_ALLIANCE] >= BG_VOP_MAX_TEAM_SCORE)
            EndBattleground(ALLIANCE);
        else if (m_TeamScores[TEAM_HORDE] >= BG_VOP_MAX_TEAM_SCORE)
            EndBattleground(HORDE);
    }
}

void BattlegroundVOP::StartingEventCloseDoors()
{
    SpawnBGObject(BG_VOP_OBJECT_DOOR_A, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_VOP_OBJECT_DOOR_H, RESPAWN_IMMEDIATELY);

    DoorClose(BG_VOP_OBJECT_DOOR_A);
    DoorClose(BG_VOP_OBJECT_DOOR_H);
}

void BattlegroundVOP::StartingEventOpenDoors()
{
    DoorOpen(BG_VOP_OBJECT_DOOR_A);
    DoorOpen(BG_VOP_OBJECT_DOOR_H);

    for (uint32 i = BG_VOP_OBJECT_ORB_1; i < BG_VOP_OBJECT_MAX; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundVOP::RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/)
{
}

void BattlegroundVOP::UpdateTeamScore(uint32 Team)
{
    uint32 score = GetTeamScore(Team);

    if (score >= BG_VOP_MAX_TEAM_SCORE)
    {
        score = BG_VOP_MAX_TEAM_SCORE;
        if (Team == TEAM_ALLIANCE)
            EndBattleground(ALLIANCE);
        else
            EndBattleground(HORDE);
    }

    if (Team == TEAM_ALLIANCE)
        UpdateWorldState(BG_VOP_OP_RESOURCES_A, score);
    else
        UpdateWorldState(BG_VOP_OP_RESOURCES_H, score);
}

void BattlegroundVOP::EventPlayerClickedOnFlag(Player* player, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_1) ||
        player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_2) ||
        player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_3) ||
        player->HasAura(BG_VOP_SPELL_ORB_PICKED_UP_4))
        return;

    switch (target_obj->GetEntry())
    {
        case BG_VOP_OBJECT_ORB_1_ENTRY:
        {
            player->CastSpell(player, BG_VOP_SPELL_ORB_PICKED_UP_3);
            SpawnBGObject(BG_VOP_OBJECT_ORB_1, RESPAWN_ONE_DAY);
            m_FlagKeeper[0] = player->GetGUID();
            break;
        }
        case BG_VOP_OBJECT_ORB_2_ENTRY:
        {
            player->CastSpell(player, BG_VOP_SPELL_ORB_PICKED_UP_2);
            SpawnBGObject(BG_VOP_OBJECT_ORB_2, RESPAWN_ONE_DAY);
            m_FlagKeeper[1] = player->GetGUID();
            break;
        }
        case BG_VOP_OBJECT_ORB_3_ENTRY:
        {
            player->CastSpell(player, BG_VOP_SPELL_ORB_PICKED_UP_4);
            SpawnBGObject(BG_VOP_OBJECT_ORB_3, RESPAWN_ONE_DAY);
            m_FlagKeeper[2] = player->GetGUID();
            break;
        }
        case BG_VOP_OBJECT_ORB_4_ENTRY:
        {
            player->CastSpell(player, BG_VOP_SPELL_ORB_PICKED_UP_1);
            SpawnBGObject(BG_VOP_OBJECT_ORB_4, RESPAWN_ONE_DAY);
            m_FlagKeeper[3] = player->GetGUID();
            break;
        }
        default:
            break;
    }

    UpdatePlayerScore(player, SCORE_ORB_CONTROL, 1);
}

bool BattlegroundVOP::SetupBattleground()
{
    // Doors
    if (!AddObject(BG_VOP_OBJECT_DOOR_A, BG_VOP_OBJECT_DOOR_ENTRY, 1783.84f, 1100.66f, 20.60f, 1.625020f, 0, 0, sin(1.625020f / 2), cos(1.625020f / 2), RESPAWN_IMMEDIATELY)
        || !AddObject(BG_VOP_OBJECT_DOOR_H, BG_VOP_OBJECT_DOOR_ENTRY, 1780.15f, 1570.22f, 24.59f, 4.711630f, 0, 0, sin(4.711630f / 2), cos(4.711630f / 2), RESPAWN_IMMEDIATELY))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn doors object. Battleground not created!");
        return false;
    }

    // Definition des positions des guides d'esprit
    Position spiritPos1 = { 1892.61f, 1151.69f, 14.7160f, 2.523528f }; // Position pour l'Alliance
    Position spiritPos2 = { 1672.40f, 1524.10f, 16.7387f, 6.032206f }; // Position pour la Horde

    // Ajout des guides d'esprit pour l'Alliance
    if (!AddSpiritGuide(static_cast<uint32>(BG_VOP_CREATURE_SPIRIT_1), spiritPos1, static_cast<TeamId>(ALLIANCE)))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn Spirit Guide for Alliance at position: (%f, %f, %f)", spiritPos1.m_positionX, spiritPos1.m_positionY, spiritPos1.m_positionZ);
        return false;
    }
    // Ajout des guides d'esprit pour la Horde
    if (!AddSpiritGuide(static_cast<uint32>(BG_VOP_CREATURE_SPIRIT_2), spiritPos2, static_cast<TeamId>(HORDE)))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn Spirit Guide for Horde at position: (%f, %f, %f)", spiritPos2.m_positionX, spiritPos2.m_positionY, spiritPos2.m_positionZ);
        return false;
    }

    // Buffs
    if (!AddObject(BG_VOP_OBJECT_BUFF_NORTH, Buff_Entries[2], 1856.007935f, 1333.637392f, 10.554197f, 3.147876f, 0, 0, sin(3.147876f / 2), cos(3.147876f / 2), BUFF_RESPAWN_TIME)
        || !AddObject(BG_VOP_OBJECT_BUFF_SOUTH, Buff_Entries[2], 1710.443604f, 1333.375000f, 10.554073f, 0.002354f, 0, 0, sin(0.002354f / 2), cos(0.002354f / 2), BUFF_RESPAWN_TIME))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn buff object. Battleground not created!");
        return false;
    }

    if (!AddObject(BG_VOP_OBJECT_ORB_1, BG_VOP_OBJECT_ORB_1_ENTRY, 1716.78f, 1416.64f, 13.5709f, 1.57239f, 0, 0, sin(1.57239f / 2), cos(1.57239f / 2), RESPAWN_ONE_DAY))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn ball object. Battleground not created!");
        return false;
    }

    Position triggerPos = { 1716.78f, 1416.64f, 13.5709f, 1.57239f }; // Définition de la position

    if (Creature* trigger3 = AddCreature(WORLD_TRIGGER, BG_VOP_CREATURE_ORB_AURA_3, triggerPos, TEAM_NEUTRAL, RESPAWN_IMMEDIATELY))
    {
        trigger3->AddAura(BG_VOP_SPELL_ORB_AURA_3, trigger3);
    }

    if (!AddObject(BG_VOP_OBJECT_ORB_2, BG_VOP_OBJECT_ORB_2_ENTRY, 1850.26f, 1416.77f, 13.5709f, 1.56061f, 0, 0, sin(1.56061f / 2), cos(1.56061f / 2), RESPAWN_ONE_DAY))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn ball object. Battleground not created!");
        return false;
    }

    Position triggerPos2 = { 1850.26f, 1416.77f, 13.5709f, 1.56061f }; // Définition de la position

    if (Creature* trigger2 = AddCreature(WORLD_TRIGGER, BG_VOP_CREATURE_ORB_AURA_2, triggerPos2, TEAM_NEUTRAL, RESPAWN_IMMEDIATELY))
    {
        trigger2->AddAura(BG_VOP_SPELL_ORB_AURA_2, trigger2);
    }

    if (!AddObject(BG_VOP_OBJECT_ORB_3, BG_VOP_OBJECT_ORB_3_ENTRY, 1850.29f, 1250.31f, 13.5708f, 4.70848f, 0, 0, sin(4.70848f / 2), cos(4.70848f / 2), RESPAWN_ONE_DAY))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn ball object. Battleground not created!");
        return false;
    }

    Position triggerPos4 = { 1850.29f, 1250.31f, 13.5708f, 4.70848f }; // Définition de la position

    if (Creature* trigger4 = AddCreature(WORLD_TRIGGER, BG_VOP_CREATURE_ORB_AURA_4, triggerPos4, TEAM_NEUTRAL, RESPAWN_IMMEDIATELY))
    {
        trigger4->AddAura(BG_VOP_SPELL_ORB_AURA_4, trigger4);
    }

    if (!AddObject(BG_VOP_OBJECT_ORB_4, BG_VOP_OBJECT_ORB_4_ENTRY, 1716.83f, 1249.93f, 13.5706f, 4.71397f, 0, 0, sin(4.71397f / 2), cos(4.71397f / 2), RESPAWN_ONE_DAY))
    {
        SC_LOG_ERROR("battlegrounds", "BattlegroundVOP: Failed to spawn ball object. Battleground not created!");
        return false;
    }

    Position triggerPos1 = { 1716.83f, 1249.93f, 13.5706f, 4.71397f }; // Définition de la position

    if (Creature* trigger1 = AddCreature(WORLD_TRIGGER, BG_VOP_CREATURE_ORB_AURA_1, triggerPos1, TEAM_NEUTRAL, RESPAWN_IMMEDIATELY))
    {
        trigger1->AddAura(BG_VOP_SPELL_ORB_AURA_1, trigger1);
    }

    return true;
}

void BattlegroundVOP::EndBattleground(uint32 winner)
{
    // Win reward
    if (winner == ALLIANCE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);
    if (winner == HORDE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);
    // Complete map reward
    RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);

    Battleground::EndBattleground(winner);
}

void BattlegroundVOP::Reset()
{
    //call parent's class reset
    Battleground::Reset();

    m_TeamScores[TEAM_ALLIANCE] = 0;
    m_TeamScores[TEAM_HORDE] = 0;
    m_lastTick[TEAM_ALLIANCE] = 0;
    m_lastTick[TEAM_HORDE] = 0;
    m_FlagKeeper[0] = 0;
    m_FlagKeeper[1] = 0;
    m_FlagKeeper[2] = 0;
    m_FlagKeeper[3] = 0;
    m_HonorScoreTics[TEAM_ALLIANCE] = 0;
    m_HonorScoreTics[TEAM_HORDE] = 0;
    m_HonorTics = 260;
}

void BattlegroundVOP::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    //create score and add it to map
    BattlegroundVOPScore* sc = new BattlegroundVOPScore(player->GetGUID());
    PlayerScores[player->GetGUID()] = sc;
}

void BattlegroundVOP::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (m_FlagKeeper[0] == player->GetGUID())
    {
        SpawnBGObject(BG_VOP_OBJECT_ORB_1, RESPAWN_IMMEDIATELY);
        m_FlagKeeper[0] = 0;
    }
    if (m_FlagKeeper[1] == player->GetGUID())
    {
        SpawnBGObject(BG_VOP_OBJECT_ORB_2, RESPAWN_IMMEDIATELY);
        m_FlagKeeper[1] = 0;
    }
    if (m_FlagKeeper[2] == player->GetGUID())
    {
        SpawnBGObject(BG_VOP_OBJECT_ORB_3, RESPAWN_IMMEDIATELY);
        m_FlagKeeper[2] = 0;
    }
    if (m_FlagKeeper[3] == player->GetGUID())
    {
        SpawnBGObject(BG_VOP_OBJECT_ORB_4, RESPAWN_IMMEDIATELY);
        m_FlagKeeper[3] = 0;
    }

    if (killer->GetBGTeam() == ALLIANCE)
    {
        m_TeamScores[TEAM_ALLIANCE] += BG_VOP_PK_VP;
        UpdateWorldState(BG_VOP_OP_RESOURCES_A, m_TeamScores[TEAM_ALLIANCE]);
    }
    else if (killer->GetBGTeam() == HORDE)
    {
        m_TeamScores[TEAM_HORDE] += BG_VOP_PK_VP;
        UpdateWorldState(BG_VOP_OP_RESOURCES_H, m_TeamScores[TEAM_HORDE]);
    }
    UpdatePlayerScore(killer, SCORE_ORB_SCORE, BG_VOP_PK_VP);

    Battleground::HandleKillPlayer(player, killer);
}

bool BattlegroundVOP::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    BattlegroundScoreMap::iterator itr = PlayerScores.find(player->GetGUID());
    if (itr == PlayerScores.end()) // player not found
        return false;

    switch (type)
    {
    case SCORE_ORB_CONTROL:
        ((BattlegroundVOPScore*)itr->second)->OrbControl += value;
        break;
    case SCORE_ORB_SCORE:
        ((BattlegroundVOPScore*)itr->second)->OrbScore += value;
        break;
    default:
        return Battleground::UpdatePlayerScore(player, type, value, doAddHonor);
    }

    return true;
}

WorldSafeLocsEntry const* BattlegroundVOP::GetClosestGraveYard(Player* player)
{
    if (!player)
        return nullptr;

    uint8 team = player->GetTeamId();
    uint32 graveyardID = 0;

    // Si le statut du champ de bataille n'est pas "en cours", envoie le joueur au cimetière de la salle de drapeau
    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        if (team == TEAM_ALLIANCE)
            graveyardID = BG_VOP_GRAVEYARD_RECTANGLEA2; // Cimetière pour l'Alliance pendant la préparation
        else
            graveyardID = BG_VOP_GRAVEYARD_RECTANGLEH2; // Cimetière pour la Horde pendant la préparation
    }
    else
    {
        // Si le champ de bataille est en cours, vérifie si le joueur est plus proche d'un cimetière de l'ennemi ou d'un cimetière central
        WorldSafeLocsEntry const* graveyard_enemy_base = nullptr;
        WorldSafeLocsEntry const* graveyard_enemy_middle = nullptr;

        // Définir les cimetières ennemis pour l'Alliance et la Horde
        if (team == TEAM_ALLIANCE)
        {
            graveyard_enemy_base = sWorldSafeLocsStore.LookupEntry(BG_VOP_GRAVEYARD_RECTANGLEH1); // Cimetière de la Horde
            graveyard_enemy_middle = sWorldSafeLocsStore.LookupEntry(BG_VOP_GRAVEYARD_RECTANGLEH2); // Autre cimetière de la Horde
        }
        else if (team == TEAM_HORDE)
        {
            graveyard_enemy_base = sWorldSafeLocsStore.LookupEntry(BG_VOP_GRAVEYARD_RECTANGLEA1); // Cimetière de l'Alliance
            graveyard_enemy_middle = sWorldSafeLocsStore.LookupEntry(BG_VOP_GRAVEYARD_RECTANGLEA2); // Autre cimetière de l'Alliance
        }

        // Si les cimetières ennemis existent, comparez les distances
        if (graveyard_enemy_base && graveyard_enemy_middle)
        {
            // Vérifier la distance 2D entre le joueur et les cimetières ennemis
            if (player->GetDistance2d(graveyard_enemy_base->Loc.X, graveyard_enemy_base->Loc.Y) <
                player->GetDistance2d(graveyard_enemy_middle->Loc.X, graveyard_enemy_middle->Loc.Y))
            {
                // Retourne le cimetière du début pour l'équipe en cours
                graveyardID = (team == TEAM_ALLIANCE) ? BG_VOP_GRAVEYARD_RECTANGLEA1 : BG_VOP_GRAVEYARD_RECTANGLEH1;
            }
            else
            {
                // Retourne le cimetière central pour l'équipe en cours
                graveyardID = (team == TEAM_ALLIANCE) ? BG_VOP_GRAVEYARD_RECTANGLEA2 : BG_VOP_GRAVEYARD_RECTANGLEH2;
            }
        }
    }

    // Recherche le cimetière à l'ID déterminé
    WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(graveyardID);

    if (!entry)
    {
        SC_LOG_ERROR("bg.battleground", "BattlegroundVOP: Graveyard not found. Graveyard system isn't working!");
        return nullptr;
    }

    return entry;
}

/*
//npcbot
WorldSafeLocsEntry const* BattlegroundVOP::GetClosestGraveyardForBot(WorldLocation const& curPos, uint32 team) const
{
    TeamId teamIndex = GetTeamIndexByTeamId(team);

    WorldSafeLocsEntry const* good_entry = nullptr;

    float plr_x = curPos.GetPositionX();
    float plr_y = curPos.GetPositionY();

    float mindist = 999999.0f;
    for (uint8 i = VOP_GRAVEYARD_START_ALLIANCE + teamIndex; i < VOP_MAX_GRAVEYARDS; i += 2)
    {
        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(BG_VOP_GraveyardIds[i]);

        if (!entry)
            continue;

        float dist = (entry->Loc.X - plr_x) * (entry->Loc.X - plr_x) + (entry->Loc.Y - plr_y) * (entry->Loc.Y - plr_y);
        if (mindist > dist)
        {
            mindist = dist;
            good_entry = entry;
        }
    }

    if (!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry(BG_VOP_GraveyardIds[teamIndex + 2]);

    return good_entry;
}
//end npcbot
*/

uint32 BattlegroundVOP::GetPrematureWinner()
{
    if (GetTeamScore(TEAM_ALLIANCE) > GetTeamScore(TEAM_HORDE))
        return ALLIANCE;
    else if (GetTeamScore(TEAM_HORDE) > GetTeamScore(TEAM_ALLIANCE))
        return HORDE;

    return Battleground::GetPrematureWinner();
}
