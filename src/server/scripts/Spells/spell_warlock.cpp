/*
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/*
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "ScriptPCH.h"
#include "Spell.h"
#include "SpellAuraEffects.h"

enum WarlockSpells
{
    WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS    = 54435,
    WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER  = 54443,
    WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD    = 54508,
    WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER   = 54509,
    WARLOCK_DEMONIC_EMPOWERMENT_IMP         = 54444,
    WARLOCK_IMPROVED_HEALTHSTONE_R1         = 18692,
    WARLOCK_IMPROVED_HEALTHSTONE_R2         = 18693,
};

class spell_warl_banish : public SpellScriptLoader
{
public:
    spell_warl_banish() : SpellScriptLoader("spell_warl_banish") { }

    class spell_warl_banish_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_banish_SpellScript);

        bool Load()
        {
            _removed = false;
            return true;
        }

        void HandleBanish()
        {
            if (Unit* target = GetHitUnit())
            {
                if (target->GetAuraEffect(SPELL_AURA_SCHOOL_IMMUNITY, SPELLFAMILY_WARLOCK, 0, 0x08000000, 0))
                {
                    //No need to remove old aura since its removed due to not stack by current Banish aura
                    PreventHitDefaultEffect(EFFECT_0);
                    PreventHitDefaultEffect(EFFECT_1);
                    PreventHitDefaultEffect(EFFECT_2);
                    _removed = true;
                }
            }
        }

        void RemoveAura()
        {
            if (_removed)
                PreventHitAura();
        }

        void Register()
        {
            BeforeHit += SpellHitFn(spell_warl_banish_SpellScript::HandleBanish);
            AfterHit += SpellHitFn(spell_warl_banish_SpellScript::RemoveAura);
        }

        bool _removed;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warl_banish_SpellScript();
    }
};

// 47193 Demonic Empowerment
class spell_warl_demonic_empowerment : public SpellScriptLoader
{
    public:
        spell_warl_demonic_empowerment() : SpellScriptLoader("spell_warl_demonic_empowerment") { }

        class spell_warl_demonic_empowerment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_empowerment_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS))
                    return false;
                if (!sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_IMP))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Creature* targetCreature = GetHitCreature())
                {
                    if (targetCreature->isPet())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(targetCreature->GetEntry());
                        switch (ci->family)
                        {
                        case CREATURE_FAMILY_SUCCUBUS:
                            targetCreature->CastSpell(targetCreature, WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS, true);
                            break;
                        case CREATURE_FAMILY_VOIDWALKER:
                        {
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER);
                            int32 hp = int32(targetCreature->CountPctFromMaxHealth(GetCaster()->CalculateSpellDamage(targetCreature, spellInfo, 0)));
                            targetCreature->CastCustomSpell(targetCreature, WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER, &hp, NULL, NULL, true);
                            //unitTarget->CastSpell(unitTarget, 54441, true);
                            break;
                        }
                        case CREATURE_FAMILY_FELGUARD:
                            targetCreature->CastSpell(targetCreature, WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD, true);
                            break;
                        case CREATURE_FAMILY_FELHUNTER:
                            targetCreature->CastSpell(targetCreature, WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER, true);
                            break;
                        case CREATURE_FAMILY_IMP:
                            targetCreature->CastSpell(targetCreature, WARLOCK_DEMONIC_EMPOWERMENT_IMP, true);
                            break;
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_demonic_empowerment_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_empowerment_SpellScript();
        }
};

// 6201 Create Healthstone (and ranks)
class spell_warl_create_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_create_healthstone() : SpellScriptLoader("spell_warl_create_healthstone") { }

        class spell_warl_create_healthstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_create_healthstone_SpellScript);

            static uint32 const iTypes[8][3];

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(WARLOCK_IMPROVED_HEALTHSTONE_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(WARLOCK_IMPROVED_HEALTHSTONE_R2))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 rank = 0;
                    // Improved Healthstone
                    if (AuraEffect const* aurEff = unitTarget->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 284, 0))
                    {
                        switch (aurEff->GetId())
                        {
                            case WARLOCK_IMPROVED_HEALTHSTONE_R1: rank = 1; break;
                            case WARLOCK_IMPROVED_HEALTHSTONE_R2: rank = 2; break;
                            default:
                                sLog->outError("Unknown rank of Improved Healthstone id: %d", aurEff->GetId());
                                break;
                        }
                    }
                    uint8 spellRank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);
                    if (spellRank > 0 && spellRank <= 8)
                        CreateItem(effIndex, iTypes[spellRank - 1][rank]);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_create_healthstone_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_create_healthstone_SpellScript();
        }
};

uint32 const spell_warl_create_healthstone::spell_warl_create_healthstone_SpellScript::iTypes[8][3] = {
    { 5512, 19004, 19005},             // Minor Healthstone
    { 5511, 19006, 19007},             // Lesser Healthstone
    { 5509, 19008, 19009},             // Healthstone
    { 5510, 19010, 19011},             // Greater Healthstone
    { 9421, 19012, 19013},             // Major Healthstone
    {22103, 22104, 22105},             // Master Healthstone
    {36889, 36890, 36891},             // Demonic Healthstone
    {36892, 36893, 36894}               // Fel Healthstone
};

// 47422 Everlasting Affliction
class spell_warl_everlasting_affliction : public SpellScriptLoader
{
    public:
        spell_warl_everlasting_affliction() : SpellScriptLoader("spell_warl_everlasting_affliction") { }

        class spell_warl_everlasting_affliction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_everlasting_affliction_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh corruption on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x2, 0, 0, GetCaster()->GetGUID()))
                        aur->GetBase()->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_everlasting_affliction_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_everlasting_affliction_SpellScript();
        }
};

class spell_warl_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_seed_of_corruption() : SpellScriptLoader("spell_warl_seed_of_corruption") { }

        class spell_warl_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seed_of_corruption_SpellScript);

            void FilterTargets(std::list<Unit*>& unitList)
            {
                unitList.remove(GetTargetUnit());
            }

            void Register()
            {
                OnUnitTargetSelect += SpellUnitTargetFn(spell_warl_seed_of_corruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seed_of_corruption_SpellScript();
        }
};

// Life Tap
// Spell Id: 1454
class spell_warl_life_tap : public SpellScriptLoader
{
public:
    spell_warl_life_tap() : SpellScriptLoader("spell_warl_life_tap") { }

    class spell_warl_life_tap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_life_tap_SpellScript);

        SpellCastResult CheckCast()
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue()) >= caster->GetHealth()) // You cant kill yourself with this
                    return SPELL_FAILED_FIZZLE;

                return SPELL_CAST_OK;
            }
            return SPELL_FAILED_DONT_REPORT;
        }

        void HandleDummy(SpellEffIndex /*EffIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                int32 damage = int32(caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue()));
                int32 mana = 0;

                float multiplier = 1.2f;

                // Should not appear in combat log
                caster->ModifyHealth(-damage);

                // Improved Life Tap mod
                if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 208, 0))
                    multiplier += int32(aurEff->GetAmount() / 100);

                mana = int32(damage * multiplier);
                caster->CastCustomSpell(caster, 31818, &mana, NULL, NULL, false);

                // Mana Feed
                int32 manaFeedVal = 0;
                if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, 1982, 0))
                    manaFeedVal = aurEff->GetAmount();

                if (manaFeedVal > 0)
                {
                    manaFeedVal = int32(mana * manaFeedVal / 100);
                    caster->CastCustomSpell(caster, 32553, &manaFeedVal, NULL, NULL, true, NULL);
                }
            }
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_warl_life_tap_SpellScript::CheckCast);
            OnEffectHitTarget += SpellEffectFn(spell_warl_life_tap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warl_life_tap_SpellScript();
    }
};

// Fear
// Spell Id: 5782
class spell_warl_fear : public SpellScriptLoader
{
    public:
        spell_warl_fear() : SpellScriptLoader("spell_warl_fear") { }

        class spell_warl_fear_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_fear_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                // Check for Improved Fear
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 98, 0))
                {
                    uint32 spellId = 0;
                    switch (aurEff->GetId())
                    {
                        case 53759:
                            spellId = 60947;
                            break;
                        case 53754:
                            spellId = 60946;
                            break;
                    }
                    if (spellId)
                        GetCaster()->CastSpell(GetTarget(), spellId, true);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_fear_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_FEAR, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_fear_AuraScript();
        }
};

// Drain Life
// Drain Life: Soulburn
// Spell Id: 689
// Spell Id: 89420
class spell_warl_drain_life : public SpellScriptLoader
{
public:
    spell_warl_drain_life() : SpellScriptLoader("spell_warl_drain_life") { }

    class spell_warl_drain_life_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_drain_life_AuraScript);

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            int32 bp = 2; // Normal, restore 2% of health

            // Check for Death's Embrace
            if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 3223, 0))
                if (GetCaster()->HealthBelowPct(25))
                    bp += int32(aurEff->GetAmount());

            GetCaster()->CastCustomSpell(GetCaster(), 89653, &bp, NULL, NULL, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_life_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_drain_life_AuraScript();
    }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_banish();
    new spell_warl_demonic_empowerment();
    new spell_warl_create_healthstone();
    new spell_warl_everlasting_affliction();
    new spell_warl_seed_of_corruption();
    new spell_warl_life_tap();
    new spell_warl_fear();
    new spell_warl_drain_life();
}
