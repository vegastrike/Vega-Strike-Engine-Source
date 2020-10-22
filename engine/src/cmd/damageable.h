#ifndef DAMAGEABLE_H
#define DAMAGEABLE_H

#include "unit_armorshield.h"
#include "gfx/vec.h"


class Damageable
{
public:
    // TODO: Consider making into protected later
    // Fields
    float maxhull;
    float hull;
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


  virtual void ArmorDamageSound( const Vector &pnt ) = 0;
  virtual void HullDamageSound( const Vector &pnt ) = 0;
  static float totalShieldVal( const Shield &shield );
  static float currentTotalShieldVal( const Shield &shield );
  static float totalShieldEnergyCapacitance( const Shield &shield );
};



#endif // DAMAGEABLE_H
