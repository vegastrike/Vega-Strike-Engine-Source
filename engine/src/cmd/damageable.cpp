/*
 * damageable.cpp
 *
 * Copyright (C) 2020-2025 Daniel Horn, Roy Falk, Stephen G. Tuggy and
 * other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "damageable.h"

#include "gfx/vec.h"
#include "vs_math.h"
#include "configuration/game_config.h"
#include "vs_globals.h"
#include "configxml.h"
#include "lin_time.h"
#include "damage.h"
#include "unit_generic.h"
#include "ai/communication.h"
#include "universe.h"
#include "ai/order.h"
#include "pilot.h"
#include "ai/comm_ai.h"
#include "gfx/mesh.h"
#include "vega_cast_utils.h"

#include <algorithm>
#include "configuration/configuration.h"


float Damageable::DealDamageToHull(const Vector &pnt, float damage) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    
    Damage dmg(0, damage); // Bypass shield with phase damage
    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);
    InflictedDamage inflicted_damage(3);
    unit->armor.DealDamage(attack_vector, dmg, inflicted_damage);
    unit->hull.DealDamage(attack_vector, dmg, inflicted_damage);
    int facet_index = unit->armor.GetFacetIndex(attack_vector);

    float denominator = unit->armor.facets[facet_index].Value() + 
        unit->hull.Get();
    if (denominator == 0) {
        return 0;
    }

    return damage / denominator;
}

float Damageable::DealDamageToShield(const Vector &pnt, float &damage) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    Damage dmg(damage);
    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);
    InflictedDamage inflicted_damage(3);
    unit->shield.DealDamage(attack_vector, dmg, inflicted_damage);
    int facet_index = unit->shield.GetFacetIndex(attack_vector);

    float denominator = unit->shield.facets[facet_index].Value() + 
        unit->hull.Get();
    if (denominator == 0) {
        return 0;
    }

    return damage / denominator;
}

// TODO: deal with this
extern void ScoreKill(Cockpit *cp, Unit *killer, Unit *killedUnit);

void Damageable::ApplyDamage(const Vector &pnt,
        const Vector &normal,
        Damage damage,
        Unit *affected_unit,
        const GFXColor &color,
        void *ownerDoNotDereference) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    
    InflictedDamage inflicted_damage(3);

    //We also do the following lock on client side in order not to display shield hits
    const bool no_dock_damage = configuration()->physics_config.no_damage_to_docked_ships;
    const bool apply_difficulty_enemy_damage = configuration()->physics_config.difficulty_based_enemy_damage;

    // Stop processing if the affected unit isn't this unit
    // How could this happen? Why even have two parameters (this and affected_unit)???
    if (affected_unit != unit) {
        return;
    }

    // Stop processing for destroyed units
    if (unit->Destroyed()) {
        return;
    }

    // Stop processing for docked ships if no_dock_damage is set
    if (no_dock_damage && (unit->DockedOrDocking() & (unit->DOCKED_INSIDE | unit->DOCKED))) {
        return;
    }

    Cockpit *shooter_cockpit = _Universe->isPlayerStarshipVoid(ownerDoNotDereference);
    bool shooter_is_player = (shooter_cockpit != nullptr);
    bool shot_at_is_player = _Universe->isPlayerStarship(unit);
    Vector localpnt(InvTransform(unit->cumulative_transformation_matrix, pnt));
    Vector localnorm(unit->ToLocalCoordinates(normal));
    CoreVector attack_vector(localpnt.i, localpnt.j, localpnt.k);
    float previous_hull_percent = unit->hull.Percent();

    inflicted_damage = DealDamage(attack_vector, damage);

    if (shooter_is_player) {
        // Why is color relevant here?
        if (color.a != 2 && apply_difficulty_enemy_damage) {
            damage.phase_damage *= g_game.difficulty;
            damage.normal_damage *= g_game.difficulty;
        }

        // Anger Management
        float inflicted_armor_damage = inflicted_damage.inflicted_damage_by_layer[1];
        int anger = inflicted_armor_damage ? configuration()->ai.hull_damage_anger : configuration()->ai.shield_damage_anger;

        // If we damage the armor, we do this 10 times by default
        for (int i = 0; i < anger; ++i) {
            //now we can dereference it because we checked it against the parent
            CommunicationMessage c(reinterpret_cast< Unit * > (ownerDoNotDereference), unit, nullptr, 0);
            c.SetCurrentState(c.fsm->GetHitNode(), nullptr, 0);
            if (unit->getAIState()) {
                unit->getAIState()->Communicate(c);
            }
        }

        //the dark danger is real!
        unit->Threaten(reinterpret_cast< Unit * > (ownerDoNotDereference), 10);
    } else {
        //if only the damage contained which faction it belonged to
        unit->pilot->DoHit(unit, ownerDoNotDereference, FactionUtil::GetNeutralFaction());

        // Non-player ships choose a target when hit. Presumably the shooter.
        if (unit->aistate) {
            unit->aistate->ChooseTarget();
        }
    }

    if (unit->Destroyed()) {
        unit->ClearMounts();

        if (shooter_is_player) {
            ScoreKill(shooter_cockpit, reinterpret_cast< Unit * > (ownerDoNotDereference), unit);
        } else {
            Unit *tmp;
            if ((tmp = findUnitInStarsystem(ownerDoNotDereference)) != nullptr) {
                if ((nullptr != (shooter_cockpit = _Universe->isPlayerStarshipVoid(tmp->owner)))
                        && (shooter_cockpit->GetParent() != nullptr)) {
                    ScoreKill(shooter_cockpit, shooter_cockpit->GetParent(), unit);
                } else {
                    ScoreKill(NULL, tmp, unit);
                }
            }
        }

        // Additional house cleaning
        unit->PrimeOrders();
        unit->energy.Zero();
        unit->Split(rand() % 3 + 1);


        // Effect on factions
        int neutralfac = FactionUtil::GetNeutralFaction();
        int upgradesfac = FactionUtil::GetUpgradeFaction();

        // Neutral factions (planets, jump points) and upgrades (turrets) aren't people
        // and can't eject themselves or cargo
        if (unit->faction == neutralfac || unit->faction == upgradesfac) {
            return;
        }

        // Eject cargo
        const float cargo_eject_percent = configuration()->eject_config.eject_cargo_percent;
        const uint32_t max_dump_cargo = configuration()->eject_config.max_dumped_cargo;
        uint32_t dumped_cargo = 0;

        for (unsigned int i = 0; i < unit->numCargo(); ++i) {
            if (rand() < (RAND_MAX * cargo_eject_percent) &&
                    dumped_cargo++ < max_dump_cargo) {
                unit->EjectCargo(i);
            }
        }


        // Eject Pilot
        // Can't use this as we can't reach negative hull damage
//        const float hull_dam_to_eject = configuration()->eject_config.hull_damage_to_eject;
        const float auto_eject_percent = configuration()->eject_config.auto_eject_percent;
        const bool player_autoeject = configuration()->eject_config.player_auto_eject;

        if (shot_at_is_player) {
            if (player_autoeject
                    && rand() < (RAND_MAX * auto_eject_percent)) {
                VS_LOG(debug, "Auto ejecting player");
                unit->EjectCargo((unsigned int) -1);
            } else {
                VS_LOG(debug, "Not auto ejecting player");
            }
        } else {
            if (unit->isUnit() == Vega_UnitType::unit
                    && rand() < (RAND_MAX * auto_eject_percent)) {
                VS_LOG(debug, "Auto ejecting NPC");
                unit->EjectCargo((unsigned int) -1);
            } else {
                VS_LOG(debug, "Not auto ejecting NPC");
            }
        }

        return;
    }

    // Light shields if hit
    if (inflicted_damage.inflicted_damage_by_layer[2] > 0) {
        unit->LightShields(pnt, normal, unit->shield.Percent(), color);
    }

    // Apply damage to meshes
    // TODO: move to drawable as a function
    if (inflicted_damage.inflicted_damage_by_layer[0] > 0 ||
            inflicted_damage.inflicted_damage_by_layer[1] > 0) {

        for (unsigned int i = 0; i < unit->nummesh(); ++i) {
            // TODO: figure out how to adjust looks for armor damage
            float hull_damage_percent = unit->hull.Percent();
            unit->meshdata[i]->AddDamageFX(pnt, unit->shieldtight ? unit->shieldtight * normal : Vector(0, 0, 0),
                    hull_damage_percent, color);
        }
    }

    // Shake cockpit
    Cockpit *shot_at_cockpit = _Universe->isPlayerStarship(unit);

    // The second condition should always be met, but if not, at least we won't crash
    if (shot_at_is_player && shot_at_cockpit) {
        if (inflicted_damage.inflicted_damage_by_layer[0] > 0) {
            // Hull is hit - shake hardest
            shot_at_cockpit->Shake(inflicted_damage.total_damage, 2);

            unit->playHullDamageSound(pnt);
        } else if (inflicted_damage.inflicted_damage_by_layer[1] > 0) {
            // Armor is hit - shake harder
            shot_at_cockpit->Shake(inflicted_damage.total_damage, 1);

            unit->playArmorDamageSound(pnt);
        } else {
            // Shield is hit - shake
            shot_at_cockpit->Shake(inflicted_damage.total_damage, 0);

            unit->playShieldDamageSound(pnt);
        }
    }

    // Only happens if we crossed the threshold in this attack
    if (previous_hull_percent >= configuration()->ai.hull_percent_for_comm &&
            unit->hull.Percent() < configuration()->ai.hull_percent_for_comm &&
            (shooter_is_player || shot_at_is_player)) {
        Unit *computer_ai = nullptr;
        Unit *player = nullptr;
        if (shot_at_is_player) {
            computer_ai = findUnitInStarsystem(ownerDoNotDereference);
            player = unit;
        } else {
            computer_ai = unit;
            player = shooter_cockpit->GetParent();
        }

        if (computer_ai && player) {
            Order *computer_ai_state = computer_ai->getAIState();
            Order *player_ai_state = player->getAIState();
            bool ai_is_unit = computer_ai->isUnit() == Vega_UnitType::unit;
            bool player_is_unit = player->isUnit() == Vega_UnitType::unit;
            if (computer_ai_state != nullptr && player_ai_state &&
                    ai_is_unit && player_is_unit) {
                unsigned char gender;
                vector<Animation *> *anim = computer_ai->pilot->getCommFaces(gender);
                if (shooter_is_player && configuration()->ai.assist_friend_in_need) {
                    AllUnitsCloseAndEngage(player, computer_ai->faction);
                }
                if (unit->hull.Percent() > 0 || !shooter_cockpit) {
                    CommunicationMessage c(computer_ai, player, anim, gender);
                    c.SetCurrentState(shooter_cockpit ? c.fsm->GetDamagedNode() : c.fsm->GetDealtDamageNode(),
                            anim,
                            gender);
                    player->getAIState()->Communicate(c);
                }
            }
        }
    }

    // Damage internal systems
//    const bool system_damage_on_armor = vega_config::GetGameConfig().GetBool("physics.system_damage_on_armor", false);
//    bool hull_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;
//    bool armor_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;

    DamageRandomSystem(inflicted_damage, shot_at_is_player, pnt);

    // TODO: lib_damage rewrite non-lethal
    // Note: we really want a complete rewrite together with the modules sub-system
    // Non-lethal/Disabling Weapon code here
    /*static float disabling_constant =
            XMLSupport::parse_float( vs_config->getVariable( "physics", "disabling_weapon_constant", "1" ) );
    if (hull > 0)
        pImage->LifeSupportFunctionality += disabling_constant*damage/hull;
    if (pImage->LifeSupportFunctionality < 0) {
        pImage->LifeSupportFunctionalityMax += pImage->LifeSupportFunctionality;
        pImage->LifeSupportFunctionality     = 0;
        if (pImage->LifeSupportFunctionalityMax < 0)
            pImage->LifeSupportFunctionalityMax = 0;
    }*/

    DamageCargo(inflicted_damage);
}

// TODO: get rid of extern
extern bool DestroySystem(float hull_percent, float numhits);
extern bool DestroyPlayerSystem(float hull_percent, float numhits);
extern float rand01();
extern const Unit *loadUnitByCache(std::string name, int faction);

void Damageable::DamageRandomSystem(InflictedDamage inflicted_damage, bool player, Vector attack_vector) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    
    // Ship destroyed. No point in further work
    if(unit->Destroyed()) {
        return;
    }

    bool hull_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;
    bool armor_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;

    // It's actually easier to read this condition than the equivalent form
    if (!(hull_damage || (configuration()->physics_config.system_damage_on_armor && armor_damage))) {
        return;
    }

    bool damage_system;

    if (player) {
        damage_system = DestroyPlayerSystem(unit->hull.Percent(), 1);
    } else {
        damage_system = DestroySystem(unit->hull.Percent(), 1);
    }

    if (!damage_system) {
        return;
    }

    // A brief explanation
    // indiscriminate is a fraction (25% by default). 
    // Damage is calculated as 0.25 * rand + 0.75 * (hull_damage)/(current_hull)
    // Therefore, 
    double indiscriminate_system_destruction = configuration()->physics_config.indiscriminate_system_destruction;
    double random_damage_factor = indiscriminate_system_destruction * rand01();
    double hull_damage_modifier = 1 - indiscriminate_system_destruction;
    double hull_damage_factor = hull_damage_modifier * (1 - hull_damage / unit->hull.Get());
    unit->DamageRandSys(random_damage_factor + hull_damage_factor, attack_vector);
}

void Damageable::DamageCargo(InflictedDamage inflicted_damage) {
    // TODO: lib_damage
    // The following code needs to be renabled and placed somewhere
    // Non-lethal/Disabling Weapon code here
    // TODO: enable
    /*static float disabling_constant =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "disabling_weapon_constant", "1" ) );
    if (hull > 0)
      pImage->LifeSupportFunctionality += disabling_constant*damage/hull;
    if (pImage->LifeSupportFunctionality < 0) {
        pImage->LifeSupportFunctionalityMax += pImage->LifeSupportFunctionality;
        pImage->LifeSupportFunctionality     = 0;
        if (pImage->LifeSupportFunctionalityMax < 0)
          pImage->LifeSupportFunctionalityMax = 0;
      }*/

    bool hull_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;
    bool armor_damage = inflicted_damage.inflicted_damage_by_layer[0] > 0;

    // TODO: Same condition as DamageRandomSystem - move up and merge
    if (!(hull_damage || (configuration()->physics_config.system_damage_on_armor && armor_damage))) {
        return;
    }

    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // If nothing to damage, exit
    if (unit->numCargo() == 0) {
        return;
    }

    // Is the hit unit, lucky or not
    if (DestroySystem(unit->hull.Percent(), unit->numCargo())) {
        return;
    }

    static std::string restricted_items = vs_config->getVariable("physics", "indestructable_cargo_items", "");
    int cargo_to_damage_index = rand() % unit->numCargo();
    Cargo &cargo = unit->GetCargo(cargo_to_damage_index);
    const std::string &cargo_category = cargo.GetCategory();

    bool is_upgrade = cargo_category.find("upgrades/") == 0;
    bool already_damaged = cargo_category.find("upgrades/Damaged/") == 0;
    bool is_multiple = cargo_category.find("mult_") == 0;
    bool is_restricted = restricted_items.find(cargo.GetName()) == string::npos;

    // The following comment was kept in the hopes someone else knows what it means
    //why not downgrade _add GetCargo(which).content.find("add_")!=0&&
    if (!is_upgrade || already_damaged || is_multiple || is_restricted) {
        return;
    }

    const int prefix_length = strlen("upgrades/");
    cargo.SetCategory("upgrades/Damaged/" + cargo_category.substr(prefix_length));

    // TODO: find a better name for whatever this is. Right now it's not not downgrade
    if (configuration()->physics_config.separate_system_flakiness_component) {
        return;
    }

    const Unit *downgrade = loadUnitByCache(cargo.GetName(), FactionUtil::GetFactionIndex("upgrades"));
    if (!downgrade) {
        return;
    }

    if (0 == downgrade->getNumMounts() && downgrade->SubUnits.empty()) {
        double percentage = 0;
        unit->Downgrade(downgrade, 0, 0, percentage, nullptr);
    }
}

// TODO: get rid of extern
extern void DestroyMount(Mount *);

// TODO: a lot of this should be handled by RAII
void Damageable::Destroy() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    unit->hull.Destroy();

    if (!unit->killed) {
        for (int beamcount = 0; beamcount < unit->getNumMounts(); ++beamcount) {
            DestroyMount(&unit->mounts[beamcount]);
        }

        if (!unit->Explode(false, simulation_atom_var)) {
            unit->Kill();
        }
    }
}





// TODO: fix typo
void Damageable::leach(float damShield, float damShieldRecharge, float damEnRecharge) {
    // TODO: restore this lib_damage
}


bool Damageable::flickerDamage() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    float damagelevel = unit->hull.Percent();
    if (damagelevel == 0) {
        return false;
    }

    static double counter = getNewTime();

    float diff = getNewTime() - counter;
    if (diff > configuration()->graphics_config.glow_flicker.flicker_time) {
        counter = getNewTime();
        diff = 0;
    }
    float tmpflicker = configuration()->graphics_config.glow_flicker.flicker_time * damagelevel;
    if (tmpflicker < configuration()->graphics_config.glow_flicker.min_flicker_cycle) {
        tmpflicker = configuration()->graphics_config.glow_flicker.min_flicker_cycle;
    }
    diff = fmod(diff, tmpflicker);
    //we know counter is somewhere between 0 and damage level
    //cast this to an int for fun!
    unsigned int thus = ((unsigned int) (size_t) this) >> 2;
    thus = thus % ((unsigned int) tmpflicker);
    diff = fmod(diff + thus, tmpflicker);
    if (configuration()->graphics_config.glow_flicker.flicker_off_time > diff) {
        if (damagelevel > configuration()->graphics_config.glow_flicker.hull_for_total_dark) {
            return rand() > RAND_MAX * GetElapsedTime() * configuration()->graphics_config.glow_flicker.num_times_per_second_on;
        } else {
            return true;
        }
    }
    return false;
}
