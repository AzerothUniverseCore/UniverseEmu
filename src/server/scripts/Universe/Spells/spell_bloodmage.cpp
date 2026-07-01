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

#include "ScriptMgr.h"
#include "Creature.h"
#include "Player.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "SharedDefines.h"

#include "Log.h"
#include "Chat.h"

/// ======================================
/// | BBM - Mage de Combat Sanglant      |
/// ======================================

enum BloodMageSpells
{
    SPELL_BBM_BARRIERE_DE_SANG  = 300109,
    SPELL_BBM_SANG_EVEILLE      = 300175,
    SPELL_BBM_SANG_AMELIORATION = 300180,
};

/// ===========================================
/// | BBM - Barrière de sang (ID: 300109)     |
/// ===========================================
// DBC : EFFECT_0 = APPLY_AURA + SPELL_AURA_SCHOOL_ABSORB

class spell_bbm_barriere_de_sang : public SpellScript
{
    PrepareSpellScript(spell_bbm_barriere_de_sang);

    void Register() override {}
};

class spell_bbm_barriere_de_sang_aura : public AuraScript
{
    PrepareAuraScript(spell_bbm_barriere_de_sang_aura);

    void CalculateAbsorb(AuraEffect const*, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = false;

        if (Unit* caster = GetCaster())
        {
            float spellPower = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
            amount           = int32(spellPower * 1.5f);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_bbm_barriere_de_sang_aura::CalculateAbsorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

/// ================================================
/// | BBM - Aptitude : Potentiel (ID: 300175)      |
/// ================================================
// DBC : EFFECT_0 = APPLY_AREA_AURA_RAID + aura 174 (SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT)
// EFFECT_1 et EFFECT_2 = NONE

class spell_bbm_aptitude_potentiel_aura : public AuraScript
{
    PrepareAuraScript(spell_bbm_aptitude_potentiel_aura);

    void OnEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        float spellPower = caster->ToPlayer()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_MAGIC);
        int32 bonus      = static_cast<int32>(spellPower * 0.10f);

        const_cast<AuraEffect*>(aurEff)->SetAmount(bonus);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_bbm_aptitude_potentiel_aura::OnEffectApply,
            EFFECT_0,
            SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT,
            AURA_EFFECT_HANDLE_REAL
        );
    }
};

/// =================================================
/// | BBM - Aptitude : Amélioration (ID: 300180)    |
/// =================================================
// DBC : EFFECT_0 = APPLY_AREA_AURA_RAID + aura 268 (SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT)
// EFFECT_1 et EFFECT_2 = NONE

class spell_bbm_aptitude_amelioration : public SpellScriptLoader
{
public:
    spell_bbm_aptitude_amelioration()
        : SpellScriptLoader("spell_bbm_aptitude_amelioration") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void CalculateBonus(AuraEffect const* /*aurEff*/, int32& amount, bool& canRecalc)
        {
            canRecalc = true;

            Unit* caster = GetCaster();
            if (!caster)
            {
                amount = 0;
                return;
            }

            float baseAP = caster->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE);
            amount       = int32(baseAP * 0.10f);
        }

        void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            float baseAP = caster->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE);
            const_cast<AuraEffect*>(aurEff)->SetAmount(int32(baseAP * 0.10f));
            caster->UpdateAttackPowerAndDamage();
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_impl::CalculateBonus, EFFECT_0, SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT);
            OnEffectApply      += AuraEffectApplyFn(aura_impl::AfterApply,          EFFECT_0, SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new aura_impl(); }
};

void AddSC_bloodmage_spell_scripts()
{
    RegisterSpellAndAuraScriptPair(spell_bbm_barriere_de_sang, spell_bbm_barriere_de_sang_aura);
    RegisterSpellScript(spell_bbm_aptitude_potentiel_aura);
    new spell_bbm_aptitude_amelioration();
}
