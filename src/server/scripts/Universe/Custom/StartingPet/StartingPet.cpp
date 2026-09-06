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
#include "ObjectMgr.h"
#include "Log.h"

enum HunterVars
{
    SPELL_TAME_BEAST    = 13481,
    SPELL_MEND_PET      = 136,
    PET_MAX_HAPPINESS   = 1048000
};

StartingPet* StartingPet::instance()
{
    static StartingPet instance;
    return &instance;
}

std::string StartingPet::RandName()
{
    uint16 const len = urand(4, 8);
    std::ostringstream newName;
    std::string nextChar;

    for (uint8 i = 1; i < len + 1; i++)
    {
        if (i == 1)
        {
            nextChar = toupper(_consonants[urand(0, _consonants.length() - 1)]);
        }
        else if (i % 2 == 0 || i % 5 == 0)
        {
            nextChar = _vowels[urand(0, _vowels.length() - 1)];
        }
        else
        {
            nextChar = _consonants[urand(0, _consonants.length() - 1)];
        }
        newName << nextChar;
    }

    return newName.str();
}

void StartingPet::CreateRandomPet(Player* player, bool petName)
{
    uint32 entry;
    std::string newName;
    entry = _pets[urand(0, 204)];

    // Generate new name for pet
    if (petName)
    {
        newName = RandName();
    }

    // Prevent crash due to bad configuration (entry manquant en base, ou
    // sans famille de dressage renseignee) - le nullptr est verifie ici,
    // contrairement a l'original qui deferencait directement le pointeur.
    CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
    if (!creatureTemplate)
    {
        SC_LOG_ERROR("module", "StartingPet::CreateRandomPet - Tried to create pet from unknown creature entry {}", entry);
        return;
    }

    if (!creatureTemplate->family)
    {
        SC_LOG_ERROR("module", "StartingPet::CreateRandomPet - Tried to create nonTamablePet {}", creatureTemplate->Entry);
        return;
    }

    if (player->GetPet())
    {
        return;
    }

    Pet* pet = player->CreateTamedPetFrom(entry, SPELL_TAME_BEAST);
    if (!pet)
    {
        return;
    }

    float px, py, pz;
    player->GetClosePoint(px, py, pz, pet->GetCombatReach(), PET_FOLLOW_DIST, pet->GetFollowAngle());
    if (!pet->IsPositionValid())
    {
        SC_LOG_DEBUG("module", "StartingPet::CreateRandomPet - Pet (entry {}) not loaded. Suggested coordinates isn't valid (X: {} Y: {})", pet->GetEntry(), pet->GetPositionX(), pet->GetPositionY());
        delete pet;
        return;
    }
    pet->Relocate(px, py, pz, player->GetOrientation());

    pet->SetGuidValue(UNIT_FIELD_CREATEDBY, player->GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, player->GetFaction());
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->GetLevel());

    pet->SetPower(POWER_HAPPINESS, PET_MAX_HAPPINESS);
    pet->SetReactState(REACT_DEFENSIVE);

    // Add to world
    pet->GetMap()->AddToMap(pet->ToCreature());

    pet->SetFullHealth();

    // Caster has pet now
    player->SetMinion(pet, true);

    // Set pet name
    if (!newName.empty())
    {
        pet->SetName(newName);
    }

    // Initialize Pet Stats
    pet->InitTalentForLevel();
    pet->InitStatsForLevel(player->GetLevel());

    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    player->PetSpellInitialize();
}

void StartingPet::LearnPetSpells(Player* player)
{
    for (uint32 i = 0; i < _hunterPetSpells.size(); ++i)
    {
        LearnSpellAndRanksForLevel(_hunterPetSpells[i], player);
    }
}

void StartingPet::LearnWarlockSpells(Player* player, uint32 minionMask)
{
    for (uint32 i = 0; i < 6; ++i)
    {
        if (i == 4)
            continue;

        if (minionMask & (1 << i))
        {
            LearnSpellAndRanksForLevel(_warlockSpells[i], player);
        }
    }
}

void StartingPet::SummonWarlockMinion(Player* player, uint32 minionMask)
{
    std::vector<uint32> warlockMinions;

    for (uint32 i = 0; i < 5; ++i)
    {
        if (minionMask & (1 << i))
        {
            warlockMinions.push_back(_warlockSpells[i]);
        }
    }

    if (warlockMinions.empty())
        return;

    uint32 spellId = warlockMinions[urand(0, warlockMinions.size() - 1)];
    player->CastSpell(player, spellId, true);
}

void StartingPet::LearnSpellAndRanksForLevel(uint32 spellId, Player* player)
{
    player->LearnSpell(spellId, false);
    uint32 next = sSpellMgr->GetNextSpellInChain(spellId);
    if (next && sSpellMgr->GetSpellInfo(next)->BaseLevel <= player->GetLevel())
    {
        LearnSpellAndRanksForLevel(next, player);
    }
}
