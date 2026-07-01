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

#include <chrono>
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellMgr.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellPackets.h"
#include "TemporarySummon.h"
#include "Unit.h"
#include "World.h"

enum DemonHunterSpells
{
	       SPELL_DH_GLIDE = 131347,
	       SPELL_DH_GLIDE_KNOCKBACK = 196353,
};

// 131347 - Glide
 class spell_dh_glide : public SpellScriptLoader
 {
 public:
     spell_dh_glide() : SpellScriptLoader("spell_dh_glide") { }
 
     class spell_dh_glide_SpellScript : public SpellScript
     {
         PrepareSpellScript(spell_dh_glide_SpellScript);
 
         SpellCastResult CheckRequirement()
         {
             if (auto player = GetCaster()->ToPlayer())
                 if (player->IsMounted())
                     return SPELL_FAILED_NOT_ON_MOUNTED;
 
             return SPELL_CAST_OK;
         }
 
         void Register() override
         {
             OnCheckCast += SpellCheckCastFn(spell_dh_glide_SpellScript::CheckRequirement);
         }
     };
 
     SpellScript* GetSpellScript() const override
     {
         return new spell_dh_glide_SpellScript();
     }
 };

void AddSC_demon_hunter_spell_scripts()
{
	new spell_dh_glide();
}
