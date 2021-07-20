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
}

bool Damageable::ShieldUp( const Vector &pnt ) const
{
    const int    shieldmin  = 5;

    // TODO: think about this. I have no idea why a nebula needs shields
//    static float nebshields = XMLSupport::parse_float( vs_config->getVariable( "physics", "nebula_shield_recharge", ".5" ) );
//    if (nebula != NULL || nebshields > 0)
//        return false;
    switch (shield.number)
    {
    case 2:
        return ( (pnt.k > 0) ? (shield.shield2fb.front) : (shield.shield2fb.back) ) > shieldmin;
    case 8:
        if (pnt.i > 0) {
            if (pnt.j > 0) {
                if (pnt.k > 0)
                    return shield.shield8.frontlefttop > shieldmin;
                else
                    return shield.shield8.backlefttop > shieldmin;
            } else {
                if (pnt.k > 0)
                    return shield.shield8.frontleftbottom > shieldmin;
                else
                    return shield.shield8.backleftbottom > shieldmin;
            }
        } else {
            if (pnt.j > 0) {
                if (pnt.k > 0)
                    return shield.shield8.frontrighttop > shieldmin;
                else
                    return shield.shield8.backrighttop > shieldmin;
            } else {
                if (pnt.k > 0)
                    return shield.shield8.frontrightbottom > shieldmin;
                else
                    return shield.shield8.backrightbottom > shieldmin;
            }
        }
        break;
    case 4:
        if ( fabs( pnt.k ) > fabs( pnt.i ) ) {
            if (pnt.k > 0)
                return shield.shield4fbrl.front > shieldmin;
            else
                return shield.shield4fbrl.back > shieldmin;
        } else {
            if (pnt.i > 0)
                return shield.shield4fbrl.left > shieldmin;
            else
                return shield.shield4fbrl.right > shieldmin;
        }
        return false;

    default:
        return false;
    }
}

float Damageable::DealDamageToShield( const Vector &pnt, float &damage )
{
    float  percent = 0;
    float *targ    = NULL;                       //short fix
    float  theta   = atan2( pnt.i, pnt.k );
    float  rho     = atan( pnt.j/sqrt( pnt.k*pnt.k+pnt.i*pnt.i ) );
    //ONLY APPLY DAMAGES IN NON-NETWORKING OR ON SERVER SIDE
    for (int i = 0; i < shield.number; ++i)
        if ( withinShield( shield.range[i], theta, rho ) ) {
            if (shield.shield.max[i]) {
                //comparing with max
                float tmp = damage/shield.shield.max[i];
                if (tmp > percent) percent = tmp;
            }
            targ = &shield.shield.cur[i];

            if (damage > *targ) {
                damage -= *targ;
                *targ   = 0;
              } else {
                //short fix
                *targ -= damage;
                damage = 0;
                break;
              }

        }
    if ( !FINITE( percent ) )
        percent = 0;
    return percent;
}

float Damageable::FShieldData() const
{
    switch (shield.number)
    {
    case 2:
        {
            if (shield.shield2fb.frontmax != 0)
                return shield.shield2fb.front/shield.shield2fb.frontmax;
            break;
        }
    case 4:
        {
            if (shield.shield4fbrl.frontmax != 0)
                return (shield.shield4fbrl.front)/shield.shield4fbrl.frontmax;
            break;
        }
    case 8:
        {
            if (shield.shield8.frontrighttopmax != 0 || shield.shield8.frontrightbottommax != 0
                || shield.shield8.frontlefttopmax != 0 || shield.shield8.frontleftbottommax
                != 0) {
                return (shield.shield8.frontrighttop+shield.shield8.frontrightbottom+shield.shield8.frontlefttop
                        +shield.shield8.frontleftbottom)
                       /(shield.shield8.frontrighttopmax+shield.shield8.frontrightbottommax+shield.shield8.frontlefttopmax
                         +shield.shield8.frontleftbottommax);
            }
            break;
        }
    }
    return 0;
}

float Damageable::BShieldData() const
{
    switch (shield.number)
    {
    case 2:
        {
            if (shield.shield2fb.backmax != 0)
                return shield.shield2fb.back/shield.shield2fb.backmax;
            break;
        }
    case 4:
        {
            if (shield.shield4fbrl.backmax != 0)
                return (shield.shield4fbrl.back)/shield.shield4fbrl.backmax;
            break;
        }
    case 8:
        {
            if (shield.shield8.backrighttopmax != 0 || shield.shield8.backrightbottommax != 0
                || shield.shield8.backlefttopmax != 0 || shield.shield8.backleftbottommax
                != 0) {
                return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.backlefttop
                        +shield.shield8.backleftbottom)
                       /(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.backlefttopmax
                         +shield.shield8.backleftbottommax);
            }
            break;
        }
    }
    return 0;
}

float Damageable::LShieldData() const
{
    switch (shield.number)
    {
    case 2:
        return 0;                                //no data, captain

    case 4:
        {
            if (shield.shield4fbrl.leftmax != 0)
                return (shield.shield4fbrl.left)/shield.shield4fbrl.leftmax;
            break;
        }
    case 8:
        {
            if (shield.shield8.backlefttopmax != 0 || shield.shield8.backleftbottommax != 0
                || shield.shield8.frontlefttopmax != 0 || shield.shield8.frontleftbottommax
                != 0) {
                return (shield.shield8.backlefttop+shield.shield8.backleftbottom+shield.shield8.frontlefttop
                        +shield.shield8.frontleftbottom)
                       /(shield.shield8.backlefttopmax+shield.shield8.backleftbottommax+shield.shield8.frontlefttopmax
                         +shield.shield8.frontleftbottommax);
            }
            break;
        }
    }
    return 0;
}

float Damageable::RShieldData() const
{
    switch (shield.number)
    {
    case 2:
        return 0;                                //don't react to stuff we have no data on

    case 4:
        {
            if (shield.shield4fbrl.rightmax != 0)
                return (shield.shield4fbrl.right)/shield.shield4fbrl.rightmax;
            break;
        }
    case 8:
        {
            if (shield.shield8.backrighttopmax != 0 || shield.shield8.backrightbottommax != 0
                || shield.shield8.frontrighttopmax != 0 || shield.shield8.frontrightbottommax
                != 0) {
                return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.frontrighttop
                        +shield.shield8.frontrightbottom)
                       /(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.frontrighttopmax
                         +shield.shield8.frontrightbottommax);
            }
            break;
        }
    }
    return 0;
}

//short fix
void Damageable::ArmorData( float armor[8] ) const
{
    armor[0] = this->armor.facets[0].health.health; //this->armor.frontrighttop;
    armor[1] = this->armor.facets[1].health.health; //this->armor.backrighttop;
    armor[2] = this->armor.facets[2].health.health; //this->armor.frontlefttop;
    armor[3] = this->armor.facets[3].health.health; //this->armor.backlefttop;
    armor[4] = this->armor.facets[4].health.health; //this->armor.frontrightbottom;
    armor[5] = this->armor.facets[5].health.health; //this->armor.backrightbottom;
    armor[6] = this->armor.facets[6].health.health; //this->armor.frontleftbottom;
    armor[7] = this->armor.facets[7].health.health; //this->armor.backleftbottom;
}

// TODO: fix typo
void Damageable::leach( float damShield, float damShieldRecharge, float damEnRecharge )
{
  // TODO: restore this
  // recharge *= damEnRecharge;
  shield.recharge *= damShieldRecharge;
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
    }
}



float Damageable::DealDamageToHull( const Vector &pnt, float damage )
{
    CoreVector core_vector(pnt.i, pnt.j, pnt.k);
    Damage d;
    d.normal_damage = damage;
    std::cout << "pre-armor normal_damage " << d.normal_damage << std::endl;
    armor.DealDamage(core_vector, d);

  /*float *affectedArmor = nullptr;

  // Identify which armor to damage
  if (pnt.i > 0) {
      if (pnt.j > 0) {
          if (pnt.k > 0)
            affectedArmor = &armor.frontlefttop;
          else
            affectedArmor = &armor.backlefttop;
        } else {
          if (pnt.k > 0)
            affectedArmor = &armor.frontleftbottom;
          else
            affectedArmor = &armor.backleftbottom;
        }
    } else {
      if (pnt.j > 0) {
          if (pnt.k > 0)
            affectedArmor = &armor.frontrighttop;
          else
            affectedArmor = &armor.backrighttop;
      } else {
          if (pnt.k > 0)
            affectedArmor = &armor.frontrightbottom;
          else
            affectedArmor = &armor.backrightbottom;
      }
    }

  float denominator = (*affectedArmor+health.health);
  float overflowDamage = std::max(damage - *affectedArmor, 0.0f);
  *affectedArmor = std::max(*affectedArmor - damage, 0.0f);

  health.health -= overflowDamage;*/

    std::cout << "post armor normal_damage " << d.normal_damage << std::endl;
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
    float maxshield = 0;
    switch (shield.number)
    {
    case 2:
        maxshield = .5*(shield.shield2fb.frontmax+shield.shield2fb.backmax);
        break;
    case 4:
        maxshield = .25
                    *(shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax
                      +shield.shield4fbrl.rightmax);
        break;
    case 8:
        maxshield = .125
                    *(shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax
                      +shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax
                      +shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax);
        break;
    }
    return maxshield;
}




float Damageable::totalShieldEnergyCapacitance( const Shield &shield )
{
    static float shieldenergycap =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "shield_energy_capacitance", ".2" ) );
    static bool  use_max_shield_value =
        XMLSupport::parse_bool( vs_config->getVariable( "physics", "use_max_shield_energy_usage", "false" ) );
    return shieldenergycap*(use_max_shield_value ? totalShieldVal( shield ) : currentTotalShieldVal( shield ));
}



bool Damageable::withinShield( const ShieldFacing &facing, float theta, float rho )
{
    float theta360 = theta+2*3.1415926536;
    return rho >= facing.rhomin && rho < facing.rhomax
           && ( (theta >= facing.thetamin
                 && theta < facing.thetamax) || (theta360 >= facing.thetamin && theta360 < facing.thetamax) );
}

float Damageable::totalShieldVal( const Shield &shield )
{
    float maxshield = 0;
    switch (shield.number)
    {
    case 2:
        maxshield = shield.shield2fb.frontmax+shield.shield2fb.backmax;
        break;
    case 4:
        maxshield = shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax
                    +shield.shield4fbrl.rightmax;
        break;
    case 8:
        maxshield = shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax
                    +shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax
                    +shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax;
        break;
    }
    return maxshield;
}

float Damageable::currentTotalShieldVal( const Shield &shield )
{
    float maxshield = 0;
    switch (shield.number)
    {
    case 2:
        maxshield = shield.shield2fb.front+shield.shield2fb.back;
        break;
    case 4:
        maxshield = shield.shield4fbrl.front+shield.shield4fbrl.back+shield.shield4fbrl.left+shield.shield4fbrl.right;
        break;
    case 8:
        maxshield = shield.shield8.frontrighttop+shield.shield8.backrighttop+shield.shield8.frontlefttop
                    +shield.shield8.backlefttop+shield.shield8.frontrightbottom+shield.shield8.backrightbottom
                    +shield.shield8.frontleftbottom+shield.shield8.backleftbottom;
        break;
    }
    return maxshield;
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
