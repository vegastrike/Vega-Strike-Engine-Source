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
#include "game_config.h"
#include "vs_globals.h"
#include "configxml.h"
#include "unit_armorshield.h"
#include "gfx/vec.h"
#include "lin_time.h"
#include "damageable_factory.h"

#include <algorithm>

// TODO: make GameConfig support sub sections
// See https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/358
static float  flickertime = 30.0f; // GameConfig::GetVariable( "graphics", "glowflicker", "time", 30.0f ) );
static float  flickerofftime = 2.0f; // ( "graphics", "glowflicker", "off-time", "2" ) );
static float  minflickercycle  = 2.0f; // ( "graphics", "glowflicker", "min-cycle", "2" ) );
static float  flickeronprob = 0.66f; // ( "graphics", "glowflicker", "num-times-per-second-on", ".66" ) );
static float  hullfornoflicker = 0.04f; // ( "graphics", "glowflicker", "hull-for-total-dark", ".04" ) );


Damageable::Damageable()
{
    armor = DamageableFactory::CreateLayer(FacetConfiguration::eight,
                                           Health(1,1,0), false);
    shield = DamageableFactory::CreateLayer(FacetConfiguration::eight,
                                            Health(1,1,0), false);
}

bool Damageable::ShieldUp( const Vector &pnt ) const
{
    const int    shield_min  = 5;

    // TODO: think about this. I have no idea why a nebula needs shields
//    static float nebshields = XMLSupport::parse_float( vs_config->getVariable( "physics", "nebula_shield_recharge", ".5" ) );
//    if (nebula != NULL || nebshields > 0)
//        return false;

    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);
    int facet_index = static_cast<DamageableLayer>(shield).GetFacetIndex(attack_vector);
    return shield.facets[facet_index].health.health > shield_min;
}

float Damageable::DealDamageToShield( const Vector &pnt, float &damage )
{
    Damage d;
    d.normal_damage = damage;
    float  percent = 0;
    CoreVector attack_vector(pnt.i, pnt.j, pnt.k);
    shield.DealDamage(attack_vector, d);
    int facet_index = shield.GetFacetIndex(attack_vector);

    float denominator = shield.facets[facet_index].health.health + health.health;
    percent = damage/denominator; //(denominator > absdamage && denom != 0) ? absdamage/denom : (denom == 0 ? 0.0 : 1.0);

    if ( !FINITE( percent ) )
        percent = 0;
    return percent;
}

float Damageable::FShieldData() const
{
    return static_cast<DamageableLayer>(shield).GetPercent(FacetName::front);
}

float Damageable::BShieldData() const
{
    return static_cast<DamageableLayer>(shield).GetPercent(FacetName::rear);
}

float Damageable::LShieldData() const
{
    return static_cast<DamageableLayer>(shield).GetPercent(FacetName::left);
}

float Damageable::RShieldData() const
{
    return static_cast<DamageableLayer>(shield).GetPercent(FacetName::right);
}

//short fix
void Damageable::ArmorData( float armor[8] ) const
{
    armor[0] = this->armor.facets[0].health.health;
    armor[1] = this->armor.facets[1].health.health;
    armor[2] = this->armor.facets[2].health.health;
    armor[3] = this->armor.facets[3].health.health;
    armor[4] = this->armor.facets[4].health.health;
    armor[5] = this->armor.facets[5].health.health;
    armor[6] = this->armor.facets[6].health.health;
    armor[7] = this->armor.facets[7].health.health;
}

// TODO: fix typo
void Damageable::leach( float damShield, float damShieldRecharge, float damEnRecharge )
{
  // TODO: restore this
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
    CoreVector core_vector(pnt.i, pnt.j, pnt.k);
    Damage d;
    d.normal_damage = damage;
    armor.DealDamage(core_vector, d);

    health.DealDamage(d);

    // We calculate and return the percent (of something)
    int facet_index = armor.GetFacetIndex(core_vector);
    DamageableFacet facet = armor.facets[facet_index];
    float denominator = facet.health.health + health.health;
    float percent = damage/denominator; //(denominator > absdamage && denom != 0) ? absdamage/denom : (denom == 0 ? 0.0 : 1.0);

    return percent;
}



float Damageable::MaxShieldVal() const
{
    return static_cast<DamageableLayer>(shield).AverageMaxLayerValue();
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
