/**
 * damageable.cpp
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "damageable.h"

#include "gfx/vec.h"
#include "vs_math.h"
#include "configuration/game_config.h"
#include "vs_globals.h"
#include "configxml.h"
#include "unit_armorshield.h"
#include "gfx/vec.h"
#include "lin_time.h"
#include "damageable_factory.h"
#include "damage.h"
#include "unit_generic.h"
#include "ai/communication.h"
#include "universe.h"
#include "ai/order.h"
#include "pilot.h"
#include "ai/comm_ai.h"
#include "gfx/mesh.h"

#include <algorithm>

// TODO: make GameConfig support sub sections
// See https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/358
static float  flickertime = 30.0f; // GameConfig::GetVariable( "graphics", "glowflicker", "time", 30.0f ) );
static float  flickerofftime = 2.0f; // ( "graphics", "glowflicker", "off-time", "2" ) );
static float  minflickercycle  = 2.0f; // ( "graphics", "glowflicker", "min-cycle", "2" ) );
static float  flickeronprob = 0.66f; // ( "graphics", "glowflicker", "num-times-per-second-on", ".66" ) );
static float  hullfornoflicker = 0.04f; // ( "graphics", "glowflicker", "hull-for-total-dark", ".04" ) );




bool Damageable::ShieldUp( const Vector &pnt ) const
{
    const int    shield_min  = 5;

    // TODO: think about this. I have no idea why a nebula needs shields
//    static float nebshields = XMLSupport::parse_float( vs_config->getVariable( "physics", "nebula_shield_recharge", ".5" ) );
//    if (nebula != NULL || nebshields > 0)
//        return false;

    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);

    DamageableLayer shield = const_cast<Damageable*>(this)->GetShieldLayer();
    int facet_index = shield.GetFacetIndex(attack_vector);
    return shield.facets[facet_index].health.health > shield_min;
}

float Damageable::DealDamageToShield( const Vector &pnt, float &damage )
{
    // TODO: lib_damage enable
    /*DamageableLayer shield = GetShield();
    DamageableLayer hull = GetHull();
    Damage d;
    d.normal_damage = damage;
    float  percent = 0;
    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);
    shield.DealDamage(attack_vector, d);
    int facet_index = shield.GetFacetIndex(attack_vector);

    float denominator = shield.facets[facet_index].health.health + hull.facets[0].health.health;
    percent = damage/denominator; //(denominator > absdamage && denom != 0) ? absdamage/denom : (denom == 0 ? 0.0 : 1.0);

    if ( !FINITE( percent ) )
        percent = 0;
    return percent;*/
    return 0;
}

// TODO: deal with this
extern void ScoreKill( Cockpit *cp, Unit *killer, Unit *killedUnit );

void Damageable::ApplyDamage( const Vector &pnt,
                              const Vector &normal,
                              Damage damage,
                              Unit *affected_unit,
                              const GFXColor &color,
                              void *ownerDoNotDereference)
{
    const Damageable *const_damagable = static_cast<const Damageable*>(this);
    Unit *unit = static_cast<Unit*>(this);


    static float hull_percent_for_comm = GameConfig::GetVariable( "AI", "HullPercentForComm", 0.75 );
    static int shield_damage_anger = GameConfig::GetVariable( "AI", "ShieldDamageAnger", 1 );
    static int hull_damage_anger   = GameConfig::GetVariable( "AI", "HullDamageAnger", 10 );
    static bool assist_ally_in_need =
        GameConfig::GetVariable( "AI", "assist_friend_in_need", true );
    static float nebula_shields = GameConfig::GetVariable( "physics", "nebula_shield_recharge", 0.5 );
    //We also do the following lock on client side in order not to display shield hits
    static bool no_dock_damage = GameConfig::GetVariable( "physics", "no_damage_to_docked_ships", true );
    static bool apply_difficulty_enemy_damage =
        GameConfig::GetVariable( "physics", "difficulty_based_enemy_damage", true );


    // Stop processing if the affected unit isn't this unit
    // How could this happen? Why even have two parameters (this and affected_unit)???
    if (affected_unit != unit) {
        return;
    }

    // Stop processing for destroyed units
    if(Destroyed()) {
        return;
    }

    // Stop processing for docked ships if no_dock_damage is set
    if (no_dock_damage && (unit->DockedOrDocking()&(unit->DOCKED_INSIDE|unit->DOCKED))) {
        return;
    }


    Cockpit     *cp = _Universe->isPlayerStarshipVoid( ownerDoNotDereference );
    bool shooter_is_player = (cp != nullptr);
    bool shot_at_is_player = _Universe->isPlayerStarship( unit );
    Vector       localpnt( InvTransform( unit->cumulative_transformation_matrix, pnt ) );
    Vector       localnorm( unit->ToLocalCoordinates( normal ) );
    CoreVector attack_vector(localpnt.i,localpnt.j,localpnt.k);
    float previous_hull_percent = GetHullPercent();

    /*float        hullpercent = GetHullPercent();
    bool         mykilled    = Destroyed();
    bool         armor_damage = false;*/

    InflictedDamage inflicted_damage = DealDamage(attack_vector, damage);

    if(shooter_is_player) {
        // Why is color relevant here?
        if (color.a != 2 && apply_difficulty_enemy_damage) {
            damage.phase_damage *= g_game.difficulty;
            damage.normal_damage *= g_game.difficulty;
        }

        // Anger Management
        float inflicted_armor_damage = inflicted_damage.inflicted_damage_by_layer[1];
        int anger = inflicted_armor_damage ? hull_damage_anger : shield_damage_anger;

        // If we damage the armor, we do this 10 times by default
        for (int i = 0; i < anger; ++i) {
            //now we can dereference it because we checked it against the parent
            CommunicationMessage c( reinterpret_cast< Unit* > (ownerDoNotDereference), unit, nullptr, 0 );
            c.SetCurrentState( c.fsm->GetHitNode(), nullptr, 0 );
            if ( unit->getAIState() ) unit->getAIState()->Communicate( c );
        }

        //the dark danger is real!
        unit->Threaten( reinterpret_cast< Unit* > (ownerDoNotDereference), 10 );
    } else {
        //if only the damage contained which faction it belonged to
        unit->pilot->DoHit( unit, ownerDoNotDereference, FactionUtil::GetNeutralFaction() );

        // Non-player ships choose a target when hit. Presumably the shooter.
        if (unit->aistate) {
            unit->aistate->ChooseTarget();
        }
    }

    if (Destroyed()) {
        std::cout << "ship is dead\n";

        unit->ClearMounts();

        if (shooter_is_player) {
            ScoreKill( cp, reinterpret_cast< Unit* > (ownerDoNotDereference), unit );
        } else {
            Unit *tmp;
            if ( ( tmp = findUnitInStarsystem( ownerDoNotDereference ) ) != nullptr ) {
                if ( ( nullptr != ( cp = _Universe->isPlayerStarshipVoid( tmp->owner ) ) )
                     && (cp->GetParent() != nullptr) )
                    ScoreKill( cp, cp->GetParent(), unit );
                else
                    ScoreKill( NULL, tmp, unit );
            }
        }


        return;
    }

    // Light shields if hit
    if(inflicted_damage.inflicted_damage_by_layer[2] > 0) {
        unit->LightShields( pnt, normal, GetShieldPercent(), color );
    }

    // Apply damage to meshes
    // TODO: move to drawable as a function
    if(inflicted_damage.inflicted_damage_by_layer[0] > 0 ||
            inflicted_damage.inflicted_damage_by_layer[1] > 0) {

        for (unsigned int i = 0; i < unit->nummesh(); ++i) {
            // TODO: figure out how to adjust looks for armor damage
            float hull_damage_percent = static_cast<float>(const_damagable->GetHullPercent());
            unit->meshdata[i]->AddDamageFX( pnt, unit->shieldtight ? unit->shieldtight*normal : Vector( 0, 0, 0 ),
                                            hull_damage_percent, color );
        }
    }

    // Shake cockpit
    if(shot_at_is_player) {
        if(inflicted_damage.inflicted_damage_by_layer[0] >0 ) {
            // Hull is hit - shake hardest
            cp->Shake( inflicted_damage.total_damage, 2 );
        } else if(inflicted_damage.inflicted_damage_by_layer[1] >0 ) {
            // Armor is hit - shake harder
            cp->Shake( inflicted_damage.total_damage, 1 );
        } else {
            // Shield is hit - shake
            cp->Shake( inflicted_damage.total_damage, 0 );
        }
    }

    // Only happens if we crossed the threshold in this attack
    if ( previous_hull_percent >= hull_percent_for_comm &&
         GetHullPercent() < hull_percent_for_comm &&
         ( shooter_is_player || shot_at_is_player ) ) {
        Unit *computer_ai = nullptr;
        Unit *player     = nullptr;
        if (shot_at_is_player) {
            computer_ai = findUnitInStarsystem( ownerDoNotDereference );
            player     = unit;
        } else { //cp != NULL
            computer_ai = unit;
            player     = cp->GetParent();
        }

        Order *computer_ai_state = computer_ai->getAIState();
        Order *player_ai_state = player->getAIState();
        bool ai_is_unit = computer_ai->isUnit() == _UnitType::unit;
        bool player_is_unit = player->isUnit() == _UnitType::unit;
        if (computer_ai && player && computer_ai_state && player_ai_state &&
            ai_is_unit && player_is_unit) {
            unsigned char gender;
            vector< Animation* > *anim = computer_ai->pilot->getCommFaces( gender );
            if (shooter_is_player && assist_ally_in_need) {
                AllUnitsCloseAndEngage( player, computer_ai->faction );
            }
            if (GetHullPercent() > 0 || !cp) {
                CommunicationMessage c( computer_ai, player, anim, gender );
                c.SetCurrentState( cp ? c.fsm->GetDamagedNode() : c.fsm->GetDealtDamageNode(), anim, gender );
                player->getAIState()->Communicate( c );
            }
        }
    }
}


void Damageable::Destroy() {

}


float Damageable::FShieldData() const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    return static_cast<DamageableLayer>(damageable->GetShieldLayer()).GetPercent(FacetName::front);
}

float Damageable::BShieldData() const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    return static_cast<DamageableLayer>(damageable->GetShieldLayer()).GetPercent(FacetName::rear);
}

float Damageable::LShieldData() const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    return static_cast<DamageableLayer>(damageable->GetShieldLayer()).GetPercent(FacetName::left);
}

float Damageable::RShieldData() const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    return static_cast<DamageableLayer>(damageable->GetShieldLayer()).GetPercent(FacetName::right);
}

//short fix
void Damageable::ArmorData( float armor[8] ) const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    DamageableLayer armor_layer = damageable->GetArmorLayer();
    armor[0] = armor_layer.facets[0].health.health;
    armor[1] = armor_layer.facets[1].health.health;
    armor[2] = armor_layer.facets[2].health.health;
    armor[3] = armor_layer.facets[3].health.health;
    armor[4] = armor_layer.facets[4].health.health;
    armor[5] = armor_layer.facets[5].health.health;
    armor[6] = armor_layer.facets[6].health.health;
    armor[7] = armor_layer.facets[7].health.health;
}

// TODO: fix typo
void Damageable::leach( float damShield, float damShieldRecharge, float damEnRecharge )
{
  // TODO: restore this lib_damage
  // recharge *= damEnRecharge;
  /*shield.recharge *= damShieldRecharge;
  switch (shield.number)
    {
    case 2:
      shield.shield2fb.frontmax   *= damShield;
      shield.shield2fb.backmax    *= damShield;
      break;
    case 4:
      shield.shield4fbrl.frontmax *= damShield;
      shield.shield4fbrl.backmax  *= damShield;
      shield.shield4fbrl.leftmax  *= damShield;
      shield.shield4fbrl.rightmax *= damShield;
      break;
    case 8:
      shield.shield8.frontrighttopmax    *= damShield;
      shield.shield8.backrighttopmax     *= damShield;
      shield.shield8.frontlefttopmax     *= damShield;
      shield.shield8.backlefttopmax      *= damShield;
      shield.shield8.frontrightbottommax *= damShield;
      shield.shield8.backrightbottommax  *= damShield;
      shield.shield8.frontleftbottommax  *= damShield;
      shield.shield8.backleftbottommax   *= damShield;
      break;
    }*/
}



float Damageable::DealDamageToHull( const Vector &pnt, float damage )
{
    // TODO: lib_damage enable
    /*
    DamageableLayer armor = GetArmor();
    DamageableLayer hull = GetHull();

    CoreVector core_vector(pnt.i, pnt.j, pnt.k);
    Damage d;
    d.normal_damage = damage;

    armor.DealDamage(core_vector, d);
    hull.DealDamage(core_vector, d);

    // We calculate and return the percent (of something)
    int facet_index = armor.GetFacetIndex(core_vector);
    DamageableFacet facet = armor.facets[facet_index];
    float denominator = facet.health.health + hull.facets[0].health.health;
    float percent = damage/denominator; //(denominator > absdamage && denom != 0) ? absdamage/denom : (denom == 0 ? 0.0 : 1.0);

    return percent;*/
    return 0.0;
}



float Damageable::MaxShieldVal() const
{
    Damageable *damageable = const_cast<Damageable*>(this);
    return static_cast<DamageableLayer>(damageable->GetShieldLayer()).AverageMaxLayerValue();
}




float Damageable::totalShieldEnergyCapacitance( const DamageableLayer &shield )
{
    float total_max_shield_value = static_cast<DamageableLayer>(shield).TotalMaxLayerValue();
    float total_current_shield_value = static_cast<DamageableLayer>(shield).TotalLayerValue();

    static float shieldenergycap =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "shield_energy_capacitance", ".2" ) );
    static bool  use_max_shield_value =
        XMLSupport::parse_bool( vs_config->getVariable( "physics", "use_max_shield_energy_usage", "false" ) );
    return shieldenergycap*(use_max_shield_value ? total_max_shield_value : total_current_shield_value);
}



bool Damageable::withinShield( const ShieldFacing &facing, float theta, float rho )
{
    float theta360 = theta+2*3.1415926536;
    return rho >= facing.rhomin && rho < facing.rhomax
           && ( (theta >= facing.thetamin
                 && theta < facing.thetamax) || (theta360 >= facing.thetamin && theta360 < facing.thetamax) );
}





bool Damageable::flickerDamage()
{
    float damagelevel = GetHullPercent();
    static double counter = getNewTime();


    float diff = getNewTime()-counter;
    if (diff > flickertime) {
        counter = getNewTime();
        diff    = 0;
    }
    float tmpflicker = flickertime*damagelevel;
    if (tmpflicker < minflickercycle)
        tmpflicker = minflickercycle;
    diff = fmod( diff, tmpflicker );
    //we know counter is somewhere between 0 and damage level
    //cast this to an int for fun!
    unsigned int thus = ( (unsigned int) (size_t) this )>>2;
    thus = thus%( (unsigned int) tmpflicker );
    diff = fmod( diff+thus, tmpflicker );
    if (flickerofftime > diff) {
        if (damagelevel > hullfornoflicker)
            return rand() > RAND_MAX * GetElapsedTime()*flickeronprob;
        else
            return true;
    }
    return false;
}
