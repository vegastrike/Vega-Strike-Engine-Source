/**
 * damageable.h
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


#ifndef DAMAGEABLE_H
#define DAMAGEABLE_H

#include "unit_armorshield.h"
#include "gfx/vec.h"


class Damageable
{
public:
    // TODO: Consider making into protected later
    // Fields
    // Inconsistency between this (0) and Init (1) for hull and maxhull.
    // Chose Init and value of 1. Also commented out was 10.
    // Doesn't really matter, but documenting anyway.
    float maxhull = 1;
    float hull = 1;
    Armor armor;
    Shield shield;


    // Methods
public:
  Damageable();

protected:
  virtual ~Damageable() = default;
  // forbidden
  Damageable( const Damageable& ) = delete;
  // forbidden
  Damageable& operator=( const Damageable& ) = delete;

public:
  float GetHull() const { return hull;}

  void ArmorData( float armor[8] ) const;

  // Is the shield up from this position
  bool ShieldUp( const Vector& ) const;

  //applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  virtual float DealDamageToShield( const Vector &pnt, float &Damage );

  float FShieldData() const;
  float RShieldData() const;
  float LShieldData() const;
  float BShieldData() const;
//short fix
//  void ArmorData( float armor[8] ) const;
//Gets the current status of the hull
//  float GetHull() const
//  {
//      return hull;
//  }
  float GetHullPercent() const
  {
      return maxhull != 0 ? hull/maxhull : hull;
  }

  //reduces shields to X percentage and reduces shield recharge to Y percentage
  void leach( float XshieldPercent, float YrechargePercent, float ZenergyPercent );

  virtual float DealDamageToHull( const Vector &pnt, float Damage );

  float MaxShieldVal() const;
//regenerates all 2,4, or 6 shields for 1 SIMULATION_ATOM
  virtual void RegenShields() = 0;
  bool withinShield( const ShieldFacing &facing, float theta, float rho );

  static float totalShieldVal( const Shield &shield );
  static float currentTotalShieldVal( const Shield &shield );
  static float totalShieldEnergyCapacitance( const Shield &shield );

protected:
  bool flickerDamage();
};



#endif // DAMAGEABLE_H
