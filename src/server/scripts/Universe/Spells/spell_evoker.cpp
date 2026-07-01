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
#include "Containers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "SpellPackets.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Unit.h"
#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "World.h"

enum EvokerSpells
{
	SPELL_EVOKER_PLANER = 358733,
	SPELL_EVOKER_PLANER_KNOCKBACK = 358734,
	SPELL_EVOKER_LIVING_FLAME = 361469,
	SPELL_EVOKER_LIVING_FLAME_DAMAGE = 361500,
	SPELL_EVOKER_LIVING_FLAME_HEAL = 361509,
	SPELL_EVOKER_ENERGIZING_FLAME = 400006,
};

// 361469 - Living Flame (Red)
class spell_evo_living_flame : public SpellScript
{
	bool Validate(SpellInfo const* /*spellInfo*/) override
	{
		return ValidateSpellInfo({ SPELL_EVOKER_LIVING_FLAME_DAMAGE, SPELL_EVOKER_LIVING_FLAME_HEAL, SPELL_EVOKER_ENERGIZING_FLAME });
	}

	void HandleHitTarget(SpellEffIndex /*effIndex*/)
	{
		Unit* caster = GetCaster();
		Unit* hitUnit = GetHitUnit();
		if (caster->IsValidAssistTarget(hitUnit))
			caster->CastSpell(hitUnit, SPELL_EVOKER_LIVING_FLAME_HEAL, true);
		else
			caster->CastSpell(hitUnit, SPELL_EVOKER_LIVING_FLAME_DAMAGE, true);
	}

	void HandleLaunchTarget(SpellEffIndex /*effIndex*/)
	{
		Unit* caster = GetCaster();
		if (caster->IsValidAssistTarget(GetHitUnit()))
			return;

		if (AuraEffect* auraEffect = caster->GetAuraEffect(SPELL_EVOKER_ENERGIZING_FLAME, EFFECT_0))
		{
			//int32 manaCost = GetSpell()->GetPowerTypeCostAmount(POWER_MANA).value_or(0);
			//if (manaCost != 0)
				//GetCaster()->ModifyPower(POWER_MANA, CalculatePct(manaCost, auraEffect->GetAmount()));
		}
	}

	void Register() override
	{
		//OnEffectHitTarget += SpellEffectFn(spell_evo_living_flame::HandleHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
		//OnEffectLaunchTarget += SpellEffectFn(spell_evo_living_flame::HandleLaunchTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
	}
};

// 131347 - Glide
class spell_evoker_planer : public SpellScriptLoader
{
public:
	spell_evoker_planer() : SpellScriptLoader("spell_evoker_planer") {}

	class spell_evoker_planer_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_evoker_planer_SpellScript);

		bool Validate(SpellInfo const* /*spellInfo*/) override
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_EVOKER_PLANER))
				return false;
			return true;
		}

		SpellCastResult CheckCast()
		{
			if (!GetCaster()->IsFalling())
				return SPELL_FAILED_NOT_ON_GROUND;

			return SPELL_CAST_OK;
		}

		void HandleOnHit(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			caster->CastSpell(caster, SPELL_EVOKER_PLANER_KNOCKBACK, true);
		}

		void Register() override
		{
			OnCheckCast += SpellCheckCastFn(spell_evoker_planer_SpellScript::CheckCast);
			OnEffectHitTarget += SpellEffectFn(spell_evoker_planer_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new spell_evoker_planer_SpellScript();
	}
};

void AddSC_evoker_spell_scripts()
{
	new spell_evoker_planer();
	RegisterSpellScript(spell_evo_living_flame);
}
